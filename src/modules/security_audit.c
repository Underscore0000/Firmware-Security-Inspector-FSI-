#include "security_audit.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ANSI_GREEN  "\033[32m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_RED    "\033[31m"
#define ANSI_CYAN   "\033[36m"
#define ANSI_RESET  "\033[0m"

typedef struct {
    const char *name;
    const char *description;
    int         max_score;
} check_def_t;

static const check_def_t check_defs[] = {
    {"Secure Boot",         "Secure Boot verifies boot chain signatures",       20},
    {"TPM 2.0",             "TPM 2.0 present and enabled",                      20},
    {"NX/XD",               "No-Execute bit enabled (EFER.NXE)",                10},
    {"SMEP",                "Supervisor Mode Execution Prevention",             10},
    {"SMAP",                "Supervisor Mode Access Prevention",                10},
    {"UMIP",                "User-Mode Instruction Prevention",                  5},
    {"CET",                 "Control-flow Enforcement Technology",               5},
    {"IOMMU/VT-d",          "IOMMU for DMA protection",                          5},
    {"Microcode",           "Microcode with recent security patches",            5},
    {"SPEC_CTRL.IBRS",      "Indirect Branch Restricted Speculation",            5},
    {"Virtualization",      "VT-x or AMD-V enabled",                             3},
    {"AES-NI",              "Hardware AES acceleration",                         2},
};

#define NUM_CHECKS ((int)(sizeof(check_defs) / sizeof(check_defs[0])))

const char *fsi_audit_result_str(fsi_audit_result_t r)
{
    switch (r) {
        case AUDIT_PASS:    return "PASS";
        case AUDIT_WARNING: return "WARNING";
        case AUDIT_FAIL:    return "FAIL";
        case AUDIT_INFO:    return "INFO";
        default:            return "UNKNOWN";
    }
}

const char *fsi_audit_result_color_ansi(fsi_audit_result_t r)
{
    switch (r) {
        case AUDIT_PASS:    return ANSI_GREEN;
        case AUDIT_WARNING: return ANSI_YELLOW;
        case AUDIT_FAIL:    return ANSI_RED;
        case AUDIT_INFO:    return ANSI_CYAN;
        default:            return ANSI_RESET;
    }
}

int fsi_audit_run(fsi_audit_report_t       *report,
                  const fsi_cpu_info_t      *cpu,
                  const fsi_secureboot_info_t *sb,
                  const fsi_tpm_info_t        *tpm,
                  const fsi_registers_t       *regs,
                  const fsi_msr_data_t        *msr)
{
    if (!report || !cpu || !sb || !tpm) return -1;

    memset(report, 0, sizeof(*report));
    report->checks = calloc((size_t)NUM_CHECKS, sizeof(fsi_audit_check_t));
    if (!report->checks) return -1;
    report->check_count = NUM_CHECKS;

    int idx = 0;
    int total_score = 0;
    int total_max = 0;

#define CHECK_BEGIN(i)   fsi_audit_check_t *c = &report->checks[idx]; \
                         c->name = check_defs[i].name; \
                         c->description = check_defs[i].description; \
                         c->max_contribution = check_defs[i].max_score;

#define CHECK_PASS(rec)  c->result = AUDIT_PASS; \
                         c->score_contribution = c->max_contribution; \
                         c->recommendation = (rec);

#define CHECK_WARN(rec)  c->result = AUDIT_WARNING; \
                         c->score_contribution = c->max_contribution / 2; \
                         c->recommendation = (rec);

#define CHECK_FAIL(rec)  c->result = AUDIT_FAIL; \
                         c->score_contribution = 0; \
                         c->recommendation = (rec);

#define CHECK_END        total_score += c->score_contribution; \
                         total_max += c->max_contribution; \
                         idx++;

    /* 0: Secure Boot */
    { CHECK_BEGIN(0)
      if (sb->state == SB_STATE_ENABLED && sb->dbx_present) {
          CHECK_PASS("Secure Boot enabled with revocations. Excellent.")
      } else if (sb->state == SB_STATE_ENABLED) {
          CHECK_WARN("Update DBX (forbidden signatures list).")
      } else if (sb->state == SB_STATE_UNKNOWN) {
          c->result = AUDIT_INFO;
          c->score_contribution = 0;
          c->recommendation = "Cannot determine Secure Boot status.";
      } else {
          CHECK_FAIL("Enable Secure Boot in UEFI setup.")
      }
      CHECK_END }

    /* 1: TPM 2.0 */
    { CHECK_BEGIN(1)
      if (tpm->present && tpm->version == TPM_VERSION_20 && tpm->enabled) {
          CHECK_PASS("TPM 2.0 present and enabled.")
      } else if (tpm->present && tpm->version == TPM_VERSION_12) {
          CHECK_WARN("TPM 1.2 is obsolete. Upgrade to TPM 2.0 hardware.")
      } else if (tpm->present && !tpm->enabled) {
          CHECK_FAIL("TPM present but disabled. Enable it in UEFI.")
      } else {
          CHECK_FAIL("No TPM detected. Consider hardware with TPM 2.0.")
      }
      CHECK_END }

    /* 2: NX/XD */
    { CHECK_BEGIN(2)
      if (cpu->features.nx) {
          CHECK_PASS("NX/XD enabled. Protection against code injection.")
          if (msr && msr->available) {
              for (int i = 0; i < msr->count; i++) {
                  if (msr->entries[i].index == MSR_IA32_EFER &&
                      msr->entries[i].readable) {
                      if (!((msr->entries[i].value >> 11) & 1)) {
                          CHECK_WARN("NX supported but EFER.NXE=0. Kernel should set it.")
                      }
                      break;
                  }
              }
          }
      } else {
          CHECK_FAIL("CPU does not support NX/XD. Outdated hardware.")
      }
      CHECK_END }

    /* 3: SMEP */
    { CHECK_BEGIN(3)
      bool smep_cr4 = regs && regs->available && (regs->cr4 & CR4_SMEP);
      if (cpu->features.smep && smep_cr4) {
          CHECK_PASS("SMEP supported and enabled in CR4.")
      } else if (cpu->features.smep && !smep_cr4) {
          CHECK_WARN("SMEP supported but not enabled in CR4. Kernel should enable it.")
      } else if (cpu->features.smep) {
          c->result = AUDIT_PASS;
          c->score_contribution = c->max_contribution;
          c->recommendation = "SMEP supported (CR4 status not verifiable).";
      } else {
          CHECK_FAIL("SMEP not supported. Upgrade CPU.")
      }
      CHECK_END }

    /* 4: SMAP */
    { CHECK_BEGIN(4)
      bool smap_cr4 = regs && regs->available && (regs->cr4 & CR4_SMAP);
      if (cpu->features.smap && smap_cr4) {
          CHECK_PASS("SMAP supported and enabled in CR4.")
      } else if (cpu->features.smap && !smap_cr4) {
          CHECK_WARN("SMAP supported but not enabled in CR4.")
      } else if (cpu->features.smap) {
          c->result = AUDIT_PASS;
          c->score_contribution = c->max_contribution;
          c->recommendation = "SMAP supported.";
      } else {
          CHECK_FAIL("SMAP not supported. Upgrade CPU.")
      }
      CHECK_END }

    /* 5: UMIP */
    { CHECK_BEGIN(5)
      if (cpu->features.umip) {
          CHECK_PASS("UMIP supported and available.")
      } else {
          CHECK_WARN("UMIP not supported. Available on Goldmont+/Skylake+ CPUs.")
      }
      CHECK_END }

    /* 6: CET */
    { CHECK_BEGIN(6)
      if (cpu->features.cet_ss && cpu->features.cet_ibt) {
          CHECK_PASS("CET Shadow Stack and IBT supported.")
      } else if (cpu->features.cet_ss || cpu->features.cet_ibt) {
          CHECK_WARN("CET partially supported.")
      } else {
          CHECK_WARN("CET not supported. Requires Tiger Lake+ / Zen3+.")
      }
      CHECK_END }

    /* 7: IOMMU */
    { CHECK_BEGIN(7)
      if (cpu->features.vtx || cpu->features.amd_v) {
          c->result = AUDIT_INFO;
          c->score_contribution = c->max_contribution / 2;
          c->recommendation = "VT-x/AMD-V present. Verify IOMMU enabled in UEFI.";
      } else {
          CHECK_FAIL("Virtualization not available. IOMMU unlikely.")
      }
      CHECK_END }

    /* 8: Microcode */
    { CHECK_BEGIN(8)
      bool found_ucode = false;
      uint64_t ucode_ver = 0;
      if (msr && msr->available) {
          for (int i = 0; i < msr->count; i++) {
              if (msr->entries[i].index == MSR_IA32_UCODE_REV &&
                  msr->entries[i].readable) {
                  ucode_ver = msr->entries[i].value;
                  found_ucode = true;
                  break;
              }
          }
      }
      if (found_ucode && ucode_ver != 0) {
          c->result = AUDIT_PASS;
          c->score_contribution = c->max_contribution;
          c->recommendation = "Microcode readable. Verify latest version available.";
      } else {
          c->result = AUDIT_INFO;
          c->score_contribution = 0;
          c->recommendation = "Microcode version not verifiable (requires root for MSR).";
      }
      CHECK_END }

    /* 9: SPEC_CTRL */
    { CHECK_BEGIN(9)
      bool found_spec = false;
      bool ibrs_set = false;
      if (msr && msr->available) {
          for (int i = 0; i < msr->count; i++) {
              if (msr->entries[i].index == MSR_IA32_SPEC_CTRL &&
                  msr->entries[i].readable) {
                  found_spec = true;
                  ibrs_set = (msr->entries[i].value & 1) != 0;
                  break;
              }
          }
      }
      if (found_spec) {
          if (ibrs_set) {
              CHECK_PASS("IBRS enabled (Spectre v2 mitigation).")
          } else {
              CHECK_WARN("SPEC_CTRL present but IBRS=0. Kernel uses IBPB/retpoline as alternative.")
          }
      } else {
          c->result = AUDIT_INFO;
          c->score_contribution = c->max_contribution / 2;
          c->recommendation = "SPEC_CTRL not verifiable (requires root).";
      }
      CHECK_END }

    /* 10: Virtualization */
    { CHECK_BEGIN(10)
      if (cpu->features.vtx || cpu->features.amd_v) {
          CHECK_PASS("Hardware virtualization available.")
      } else {
          CHECK_WARN("Hardware virtualization not available.")
      }
      CHECK_END }

    /* 11: AES-NI */
    { CHECK_BEGIN(11)
      if (cpu->features.aes_ni) {
          CHECK_PASS("AES-NI hardware present. Accelerated encryption.")
      } else {
          CHECK_WARN("AES-NI absent. Software encryption will be slower.")
      }
      CHECK_END }

    report->max_score = total_max;
    if (total_max > 0)
        report->score = (total_score * 100) / total_max;
    else
        report->score = 0;

    if (report->score >= 90) strncpy(report->grade, "A", 4);
    else if (report->score >= 75) strncpy(report->grade, "B", 4);
    else if (report->score >= 60) strncpy(report->grade, "C", 4);
    else if (report->score >= 40) strncpy(report->grade, "D", 4);
    else strncpy(report->grade, "F", 4);

    return 0;
}

void fsi_audit_free(fsi_audit_report_t *report)
{
    if (report && report->checks) {
        free(report->checks);
        report->checks = NULL;
        report->check_count = 0;
    }
}

void fsi_audit_print(const fsi_audit_report_t *report)
{
    if (!report) return;
    printf("=== Security Audit Report ===\n\n");
    printf("Security Score: %d/100  [Grade: %s]\n\n",
           report->score, report->grade);

    for (int i = 0; i < report->check_count; i++) {
        const fsi_audit_check_t *c = &report->checks[i];
        const char *color = fsi_audit_result_color_ansi(c->result);
        printf("%s%-8s%s  %-24s  %+d/%d  %s\n",
               color,
               fsi_audit_result_str(c->result),
               ANSI_RESET,
               c->name,
               c->score_contribution,
               c->max_contribution,
               c->recommendation ? c->recommendation : "");
    }
    printf("\n");
}
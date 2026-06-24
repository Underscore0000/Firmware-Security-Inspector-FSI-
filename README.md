<div align="center">

<img src="https://img.shields.io/badge/Platform-x86--64-blue?style=for-the-badge&logo=intel&logoColor=white"/>
<img src="https://img.shields.io/badge/OS-Windows%20%7C%20Linux%20%7C%20UEFI-informational?style=for-the-badge&logo=linux&logoColor=white"/>
<img src="https://img.shields.io/badge/Mode-Read--Only-success?style=for-the-badge&logo=shieldsdotio&logoColor=white"/>
<img src="https://img.shields.io/badge/UI-Qt6-41CD52?style=for-the-badge&logo=qt&logoColor=white"/>
<img src="https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge"/>

<br/>
<br/>

# 🔒 Firmware Security Inspector

### Advanced firmware & system security analysis tool for x86-64 platforms

*A professional-grade, read-only auditing framework for deep inspection of firmware, hardware, and low-level system integrity.*

<br/>

[🚀 Getting Started](#-installation) • [📖 Usage](#-usage) • [📊 Scoring Model](#-security-scoring-model) • [📁 Structure](#-project-structure)

</div>

---

## 📌 Overview

**Firmware Security Inspector (FSI)** provides detailed visibility into CPU architecture, firmware configuration, memory layout, virtualization state, and security mitigations — using native system interfaces such as **CPUID**, **MSR**, **ACPI**, and **UEFI**.

The tool is entirely **read-only** and makes no modifications to firmware or hardware.

---

## 🎯 Designed For

| Audience | Use Case |
|---|---|
| 🔬 Security Researchers | Deep firmware & hardware analysis |
| 🧩 Reverse Engineers | Low-level system inspection |
| 🖥️ System Administrators | Platform security auditing |
| 🕵️ Penetration Testers | Pre-engagement recon |
| ⚙️ Low-level Developers | Hardware capability verification |

---

## 🧠 Core Features

### 🔧 CPU & Hardware Analysis

- Processor vendor, model, and architecture detection
- Core / thread enumeration
- CPUID feature flags
- Security extensions: **NX/XD**, **SMEP**, **SMAP**, **CET**, **VT-x / AMD-V**, **SGX**
- Cache hierarchy analysis (L1 / L2 / L3)

### 🛡️ Firmware & Platform Analysis

- UEFI / BIOS information extraction
- SMBIOS / DMI parsing
- ACPI table enumeration
- PCI / PCIe device discovery
- System manufacturer identification
- Virtualization detection layer

### 🔐 Security Audit Engine

- Secure Boot verification (PK / KEK / DB / DBX)
- TPM 1.2 / TPM 2.0 detection and status
- MSR register inspection
- Control Register decoding (CR0–CR4)
- CPU security mitigation checks
- Overall security posture scoring (0–100)

### 📊 Reporting System

| Format | Description |
|---|---|
| `JSON` | Machine-readable export for automation pipelines |
| `HTML` | Visual reports for browser viewing |
| `TXT` | Plain-text logs |
| `.fsi` | System snapshot capture & diff comparison |

---

## 📈 Security Scoring Model

| Component | Max Score | Description |
|---|:---:|---|
| 🔑 Secure Boot | 20 | Boot chain integrity |
| 🔒 TPM 2.0 | 20 | Trusted Platform Module |
| 🚫 NX / XD | 10 | Memory execution protection |
| 🧱 SMEP | 10 | Kernel execution protection |
| 🛑 SMAP | 10 | Kernel access protection |
| 🔏 UMIP | 5 | User-mode instruction protection |
| 🔀 CET | 5 | Control-flow integrity |
| 🌐 IOMMU / VT-d | 5 | DMA protection |
| 💾 Microcode | 5 | CPU security updates |
| ⚡ SPEC_CTRL | 5 | Spectre mitigation status |
| 🖥️ Virtualization | 3 | Hardware virtualization support |
| 🔐 AES-NI | 2 | Cryptographic acceleration |

### 🏅 Grades

| Grade | Score | Posture |
|:---:|---|---|
| 🟢 **A** | 90–100 | Excellent security posture |
| 🔵 **B** | 75–89 | Good |
| 🟡 **C** | 60–74 | Moderate |
| 🟠 **D** | 40–59 | Weak |
| 🔴 **F** | 0–39 | Critical Risk |

---

## ⚙️ System Requirements

| | Minimum | Recommended |
|---|---|---|
| **OS** | Windows 10/11 or Ubuntu 20.04+ | Windows 11 / Ubuntu 22.04+ |
| **CPU** | x86-64 with CPUID support | Intel i5 / AMD Ryzen 5 or better |
| **RAM** | 512 MB | 1–2 GB |
| **Storage** | 100 MB | — |
| **UI** | — | Qt 6.6+ |

---

## 🚀 Installation

### 🪟 Windows

```bash
git clone https://github.com/yourusername/firmware-security-inspector.git
cd firmware-security-inspector
mkdir build && cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
mingw32-make -j4
```

```bash
# Run
fsi.exe
fsi-cli.exe audit
```

### 🐧 Linux

```bash
sudo apt update
sudo apt install build-essential cmake nasm pkg-config qt6-base-dev qt6-tools-dev

git clone https://github.com/yourusername/firmware-security-inspector.git
cd firmware-security-inspector
mkdir build && cd build
cmake ..
make -j$(nproc)
```

```bash
# Run
./fsi
./fsi-cli audit
```

### 💾 UEFI (EDK2)

```bash
git clone https://github.com/tianocore/edk2.git
git clone https://github.com/yourusername/firmware-security-inspector.git

source edk2/edksetup.sh
build -p FsiUefi.dsc -t GCC5 -b RELEASE -a X64
```

---

## 🖥️ Usage

### GUI Mode

A modern Qt6 graphical interface featuring:

- 📊 Security dashboard overview
- 🔧 CPU and firmware inspection panels
- 🧠 Memory and PCIe analysis
- 🔒 Secure Boot and TPM status
- 📈 Live security scoring

**Keyboard Shortcuts:**

| Shortcut | Action |
|---|---|
| `Ctrl + R` | Refresh system data |
| `Ctrl + S` | Save snapshot |
| `Ctrl + E` | Export report |

### CLI Mode

**Inspection commands:**

```
fsi-cli audit          →  Full security audit
fsi-cli cpu            →  CPU & hardware info
fsi-cli firmware       →  UEFI / BIOS details
fsi-cli secureboot     →  Secure Boot status
fsi-cli tpm            →  TPM detection
fsi-cli acpi           →  ACPI tables
fsi-cli smbios         →  SMBIOS / DMI data
fsi-cli pcie           →  PCIe device list
fsi-cli memory         →  Memory layout
fsi-cli msr            →  MSR register dump
fsi-cli all            →  Run all modules
```

**Reporting & snapshots:**

```bash
fsi-cli report html report.html
fsi-cli snapshot save system.fsi
fsi-cli snapshot diff a.fsi b.fsi
```

---

## 📁 Project Structure

```
src/
├── asm/         Low-level x86-64 assembly
├── modules/     Core security engine
├── gui/         Qt6 interface
├── cli/         Command-line interface
├── report/      Reporting system
├── snapshot/    System state management
├── platform/    OS-specific implementations
└── uefi/        UEFI application layer
```

---

## ⚠️ Disclaimer

> This tool performs **read-only** system analysis only.  
> It does not modify firmware, BIOS, or hardware in any way.  
> Some features may require administrator / root privileges.  
> Use responsibly and only on systems you own or are authorized to test.

---

<div align="center">
  <sub>Built for security professionals · Read-only · No modifications · x86-64</sub>
</div>

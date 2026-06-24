#include "registers.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include "../platform/windows_platform.h"
#endif

int fsi_registers_collect(fsi_registers_t *regs)
{
    if (!regs) return -1;
    memset(regs, 0, sizeof(*regs));

#ifdef _WIN32
    windows_get_registers_info(regs);
#else

#endif

    return 0;
}
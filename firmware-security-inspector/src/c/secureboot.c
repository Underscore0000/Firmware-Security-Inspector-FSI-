#include "secureboot.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include "../platform/windows_platform.h"
#endif

int fsi_secureboot_collect(fsi_secureboot_info_t *info)
{
    if (!info) return -1;
    memset(info, 0, sizeof(*info));

#ifdef _WIN32
    windows_get_secureboot_info(info);
#else

#endif

    return 0;
}
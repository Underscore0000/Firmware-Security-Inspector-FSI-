#include "firmware.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include "../platform/windows_platform.h"
#endif

#if defined(__linux__)
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#endif

int fsi_firmware_collect(fsi_firmware_info_t *info)
{
    if (!info) return -1;
    memset(info, 0, sizeof(*info));

#ifdef _WIN32
    windows_get_firmware_info(info);
#else

#endif

    return 0;
}
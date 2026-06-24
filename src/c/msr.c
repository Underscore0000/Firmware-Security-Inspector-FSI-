#include "msr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include "../platform/windows_platform.h"
#endif

int fsi_msr_collect(fsi_msr_data_t *data)
{
    if (!data) return -1;
    memset(data, 0, sizeof(*data));

#ifdef _WIN32
    windows_get_msr_info(data);
#else

#endif

    return 0;
}
#include "pcie.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include "../platform/windows_platform.h"
#endif

int fsi_pcie_collect(fsi_pcie_data_t *data)
{
    if (!data) return -1;
    memset(data, 0, sizeof(*data));

#ifdef _WIN32
    windows_get_pcie_info(data);
#else

#endif

    return 0;
}
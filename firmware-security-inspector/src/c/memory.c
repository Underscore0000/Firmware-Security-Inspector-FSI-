#include "memory.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include "../platform/windows_platform.h"
#endif

int fsi_memory_collect(fsi_memory_map_t *map)
{
    if (!map) return -1;
    memset(map, 0, sizeof(*map));

#ifdef _WIN32
    windows_get_memory_info(map);
#else

#endif

    return 0;
}
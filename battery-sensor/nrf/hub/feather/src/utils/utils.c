/**
 * @file utils.c
 * @brief Utility functions implementation
 */

#include <zephyr/kernel.h>
#include <zephyr/random/rand32.h>
#include <stdio.h>
#include "utils/utils.h"

int64_t utils_get_timestamp(void)
{
    /* In a real application with a proper RTC, this would get the actual time
     * but for this simulation we'll use k_uptime_get()
     */
    return (int64_t)(k_uptime_get() / 1000) + 1680000000; /* Add Jan 1, 2023 offset for realism */
}

int utils_generate_random_id(char *buffer, size_t size)
{
    if (!buffer || size < 9) {
        return -EINVAL;
    }
    
    uint32_t random = sys_rand32_get();
    snprintf(buffer, size, "%08x", random);
    
    return 0;
}
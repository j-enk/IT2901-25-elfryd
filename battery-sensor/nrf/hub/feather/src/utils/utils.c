/**
 * @file utils.c
 * @brief Utility functions implementation
 */

#include <zephyr/kernel.h>
#include <zephyr/random/rand32.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
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
    if (!buffer || size < 9)
    {
        return -EINVAL;
    }

    uint32_t random = sys_rand32_get();
    snprintf(buffer, size, "%08x", random);

    return 0;
}

/**
 * Format a 64-bit timestamp into a string without using %lld
 *
 * @param timestamp Timestamp value to format
 * @param buffer    Output buffer to store the formatted string
 * @param size      Size of the output buffer
 * @return          Length of the formatted string or negative error code
 */
int format_timestamp(int64_t timestamp, char *buffer, size_t size)
{
    if (buffer == NULL || size < 2)
    {
        return -1;
    }

    /* Start at the end of the buffer (minus space for null terminator) */
    char *ptr = buffer + size - 2;
    *++ptr = '\0'; /* Null terminate */

    /* Handle zero as a special case */
    if (timestamp == 0)
    {
        if (size < 2)
        {
            return -1;
        }
        *--ptr = '0';
        memmove(buffer, ptr, 2); /* Move to beginning including null terminator */
        return 1;
    }

    /* Handle negative numbers */
    bool negative = (timestamp < 0);
    if (negative)
    {
        timestamp = -timestamp;
    }

    /* Convert timestamp digit by digit */
    while (timestamp > 0 && ptr > buffer)
    {
        *--ptr = '0' + (timestamp % 10);
        timestamp /= 10;
    }

    /* Add negative sign if needed */
    if (negative && ptr > buffer)
    {
        *--ptr = '-';
    }

    /* If we ran out of space, return an error */
    if (timestamp > 0)
    {
        return -1;
    }

    /* Move the string to the beginning of the buffer */
    int length = (buffer + size - 1) - ptr;
    memmove(buffer, ptr, length + 1); /* Include null terminator */

    return length;
}
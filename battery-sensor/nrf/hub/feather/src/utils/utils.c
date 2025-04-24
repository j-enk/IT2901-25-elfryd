/**
 * @file utils.c
 * @brief Utility functions implementation
 */

#include <zephyr/kernel.h>
#include <zephyr/random/rand32.h>
#include <stdio.h>
#include <string.h>
#include <date_time.h>
#include "utils/utils.h"

/* Flag to track if RTC is synchronized */
static bool rtc_synchronized = false;

/* Mutex to protect the RTC sync flag */
static K_MUTEX_DEFINE(rtc_sync_mutex);

void utils_notify_time_synchronized(void)
{
    k_mutex_lock(&rtc_sync_mutex, K_FOREVER);
    rtc_synchronized = true;
    k_mutex_unlock(&rtc_sync_mutex);
}

bool utils_is_time_synchronized(void)
{
    bool is_sync;

    k_mutex_lock(&rtc_sync_mutex, K_FOREVER);
    is_sync = rtc_synchronized;
    k_mutex_unlock(&rtc_sync_mutex);

    return is_sync;
}

int64_t utils_get_timestamp(void)
{
    int err;
    int64_t timestamp = 0;

    /* Check if RTC is synchronized before using date_time API */
    if (!utils_is_time_synchronized())
    {
        return 0; /* Return 0 to indicate that time is not valid yet */
    }

    /* Get the current real timestamp from date_time API */
    err = date_time_now(&timestamp);
    if (err)
    {
        return 0; /* Return 0 if unable to get the timestamp */
    }

    /* date_time_now returns timestamp in milliseconds, convert to seconds */
    return timestamp / 1000;
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
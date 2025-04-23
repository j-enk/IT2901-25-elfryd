/**
 * @file utils.h
 * @brief Utility functions for the Elfryd hub
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Notify that time has been synchronized
 * 
 * This must be called when the system has obtained valid time
 * from the network or other source before timestamp usage
 */
void utils_notify_time_synchronized(void);

/**
 * @brief Check if time is synchronized
 * 
 * @return true if time is synchronized, false otherwise
 */
bool utils_is_time_synchronized(void);

/**
 * @brief Get current timestamp
 *
 * @return Current timestamp in seconds since epoch, or 0 if time not synchronized
 */
int64_t utils_get_timestamp(void);

/**
 * @brief Generate a random ID string
 *
 * @param buffer Buffer to store the generated ID
 * @param size Size of the buffer
 * @return 0 on success, negative errno on failure
 */
int utils_generate_random_id(char *buffer, size_t size);

/**
 * @brief Format a 64-bit timestamp into a string
 *
 * @param timestamp Timestamp value to format
 * @param buffer Buffer to store the formatted string
 * @param size Size of the buffer
 * @return Length of the formatted string or negative error code
 */
int format_timestamp(int64_t timestamp, char *buffer, size_t size);

#endif /* UTILS_H */
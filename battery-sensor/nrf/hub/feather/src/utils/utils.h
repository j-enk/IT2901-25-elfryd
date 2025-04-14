/**
 * @file utils.h
 * @brief Utility functions for the Elfryd hub
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Get current time as unix timestamp
 *
 * @return Current unix timestamp in seconds
 */
int64_t utils_get_timestamp(void);

/**
 * @brief Generate a random string for use as a message ID
 *
 * @param buffer Buffer to store the message ID
 * @param size Size of the buffer
 * @return 0 on success, negative errno code on failure
 */
int utils_generate_random_id(char *buffer, size_t size);

/**
 * Format a 64-bit timestamp into a string without using %lld
 *
 * @param timestamp Timestamp value to format
 * @param buffer    Output buffer to store the formatted string
 * @param size      Size of the output buffer
 * @return          Length of the formatted string or negative error code
 */
int format_timestamp(int64_t timestamp, char *buffer, size_t size);

#endif /* UTILS_H */
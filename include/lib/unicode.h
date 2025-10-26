#ifndef UNICODE_H
#define UNICODE_H

#include "definitions.h"

/* Unicode and UTF-8 Utilities
 * 
 * Provides functions for working with Unicode codepoints and UTF-8 encoding.
 * Used by keyboard drivers to convert Unicode to UTF-8 for character devices.
 */

/* Convert Unicode codepoint to UTF-8 sequence
 * 
 * @param codepoint: Unicode codepoint to convert
 * @param utf8_buf: Buffer to store UTF-8 sequence (must be at least 4 bytes)
 * @return: Number of bytes in UTF-8 sequence (1-4), or 0 if invalid codepoint
 */
int unicode_to_utf8(uint32_t codepoint, char *utf8_buf);

/* Get the length of a UTF-8 sequence for a given codepoint
 * 
 * @param codepoint: Unicode codepoint
 * @return: Number of bytes needed for UTF-8 encoding (1-4), or 0 if invalid
 */
int unicode_utf8_length(uint32_t codepoint);

/* Check if a Unicode codepoint is printable
 * 
 * @param codepoint: Unicode codepoint to check
 * @return: true if the codepoint represents a printable character
 */
bool unicode_is_printable(uint32_t codepoint);

/* Check if a Unicode codepoint is in the ASCII range
 * 
 * @param codepoint: Unicode codepoint to check  
 * @return: true if the codepoint is 0-127 (ASCII)
 */
static inline bool unicode_is_ascii(uint32_t codepoint) {
    return codepoint <= 0x7F;
}

/* Get ASCII character from Unicode codepoint (if possible)
 * 
 * @param codepoint: Unicode codepoint
 * @return: ASCII character (0-127), or 0 if not ASCII
 */
static inline char unicode_to_ascii(uint32_t codepoint) {
    return unicode_is_ascii(codepoint) ? (char)codepoint : 0;
}

#endif /* UNICODE_H */
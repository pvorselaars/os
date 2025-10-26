#include "lib/unicode.h"

/* Convert Unicode codepoint to UTF-8 sequence */
int unicode_to_utf8(uint32_t codepoint, char *utf8_buf) {
    if (!utf8_buf) {
        return 0;
    }
    
    if (codepoint <= 0x7F) {
        // 1-byte sequence (ASCII)
        utf8_buf[0] = (char)codepoint;
        return 1;
    } else if (codepoint <= 0x7FF) {
        // 2-byte sequence
        utf8_buf[0] = (char)(0xC0 | (codepoint >> 6));
        utf8_buf[1] = (char)(0x80 | (codepoint & 0x3F));
        return 2;
    } else if (codepoint <= 0xFFFF) {
        // 3-byte sequence
        utf8_buf[0] = (char)(0xE0 | (codepoint >> 12));
        utf8_buf[1] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        utf8_buf[2] = (char)(0x80 | (codepoint & 0x3F));
        return 3;
    } else if (codepoint <= 0x10FFFF) {
        // 4-byte sequence
        utf8_buf[0] = (char)(0xF0 | (codepoint >> 18));
        utf8_buf[1] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
        utf8_buf[2] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        utf8_buf[3] = (char)(0x80 | (codepoint & 0x3F));
        return 4;
    } else {
        // Invalid codepoint
        return 0;
    }
}

/* Get the length of a UTF-8 sequence for a given codepoint */
int unicode_utf8_length(uint32_t codepoint) {
    if (codepoint <= 0x7F) {
        return 1;
    } else if (codepoint <= 0x7FF) {
        return 2;
    } else if (codepoint <= 0xFFFF) {
        return 3;
    } else if (codepoint <= 0x10FFFF) {
        return 4;
    } else {
        return 0;  // Invalid codepoint
    }
}

/* Check if a Unicode codepoint is printable */
bool unicode_is_printable(uint32_t codepoint) {
    // Basic printable ranges (simplified check)
    
    // ASCII printable range
    if (codepoint >= 0x20 && codepoint <= 0x7E) {
        return true;
    }
    
    // Non-breaking space
    if (codepoint == 0xA0) {
        return true;
    }
    
    // Latin-1 Supplement printable range
    if (codepoint >= 0xA1 && codepoint <= 0xFF) {
        return true;
    }
    
    // Latin Extended-A
    if (codepoint >= 0x0100 && codepoint <= 0x017F) {
        return true;
    }
    
    // Latin Extended-B  
    if (codepoint >= 0x0180 && codepoint <= 0x024F) {
        return true;
    }
    
    // For simplicity, assume most other ranges are printable
    // A full implementation would check Unicode categories
    if (codepoint >= 0x0250 && codepoint <= 0xFFFD) {
        // Exclude control characters and private use areas
        if ((codepoint >= 0x0000 && codepoint <= 0x001F) ||   // C0 controls
            (codepoint >= 0x007F && codepoint <= 0x009F) ||   // C1 controls  
            (codepoint >= 0xE000 && codepoint <= 0xF8FF) ||   // Private use
            (codepoint >= 0xFFF0 && codepoint <= 0xFFFF)) {   // Specials
            return false;
        }
        return true;
    }
    
    return false;
}
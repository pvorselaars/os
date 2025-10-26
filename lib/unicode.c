#include "lib/unicode.h"

int unicode_to_utf8(uint32_t codepoint, char *utf8_buf) {
    if (!utf8_buf) {
        return 0;
    }
    
    if (codepoint <= 0x7F) {
        utf8_buf[0] = (char)codepoint;
        return 1;
    } else if (codepoint <= 0x7FF) {
        utf8_buf[0] = (char)(0xC0 | (codepoint >> 6));
        utf8_buf[1] = (char)(0x80 | (codepoint & 0x3F));
        return 2;
    } else if (codepoint <= 0xFFFF) {
        utf8_buf[0] = (char)(0xE0 | (codepoint >> 12));
        utf8_buf[1] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        utf8_buf[2] = (char)(0x80 | (codepoint & 0x3F));
        return 3;
    } else if (codepoint <= 0x10FFFF) {
        utf8_buf[0] = (char)(0xF0 | (codepoint >> 18));
        utf8_buf[1] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
        utf8_buf[2] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        utf8_buf[3] = (char)(0x80 | (codepoint & 0x3F));
        return 4;
    } else {
        return 0;
    }
}

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
        return 0;
    }
}

bool unicode_is_printable(uint32_t codepoint) {
    if (codepoint >= 0x20 && codepoint <= 0x7E) {
        return true;
    }
    
    if (codepoint >= 0xA1 && codepoint <= 0xFF) {
        return true;
    }
    
    if (codepoint >= 0x0100 && codepoint <= 0x017F) {
        return true;
    }
    
    if (codepoint >= 0x0180 && codepoint <= 0x024F) {
        return true;
    }
    
    return false;
}
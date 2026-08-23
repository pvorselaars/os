#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "kernel/base.h"

// Logical key identifiers for special keys (hardware-independent)
typedef enum {
    KEY_UNKNOWN = 0,

    // Modifier keys
    KEY_LEFT_SHIFT, KEY_RIGHT_SHIFT,
    KEY_LEFT_CTRL, KEY_RIGHT_CTRL,
    KEY_LEFT_ALT, KEY_RIGHT_ALT,
    KEY_LEFT_SUPER, KEY_RIGHT_SUPER,  // Windows/Cmd keys

    // Lock keys
    KEY_CAPS_LOCK, KEY_NUM_LOCK, KEY_SCROLL_LOCK,

    // Navigation keys
    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
    KEY_HOME, KEY_END, KEY_PAGE_UP, KEY_PAGE_DOWN,

    // Editing keys
    KEY_INSERT, KEY_DELETE, KEY_BACKSPACE,
    KEY_TAB, KEY_ENTER, KEY_ESCAPE,

    // Function keys
    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6,
    KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,

    // System keys
    KEY_PRINT_SCREEN, KEY_PAUSE, KEY_MENU,

    // Keypad keys (when num lock affects them)
    KEY_KP_0, KEY_KP_1, KEY_KP_2, KEY_KP_3, KEY_KP_4,
    KEY_KP_5, KEY_KP_6, KEY_KP_7, KEY_KP_8, KEY_KP_9,
    KEY_KP_DECIMAL, KEY_KP_ENTER, KEY_KP_PLUS, KEY_KP_MINUS,
    KEY_KP_MULTIPLY, KEY_KP_DIVIDE,

    KEY_MAX
} keyboard_key_t;

typedef struct {
    // Unicode codepoint (0 if not a printable character)
    uint32_t unicode;

    // Logical key identifier (for special keys and when unicode == 0)
    keyboard_key_t key;

    // Event type
    bool pressed;               // true = press, false = release

    // Current modifier states (independent of this event)
    bool shift;                 // Any shift key currently pressed
    bool ctrl;                  // Any ctrl key currently pressed
    bool alt;                   // Any alt key currently pressed
    bool super;                 // Any super/windows key currently pressed
    bool caps_lock;             // Caps lock is active
    bool num_lock;              // Num lock is active
    bool scroll_lock;           // Scroll lock is active
} keyboard_event_t;

#endif
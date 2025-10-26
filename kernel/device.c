#include "kernel/device.h"
#include "lib/string.h"
#include "arch/arch.h"
#include "drivers/serial.h"
#include "drivers/keyboard.h"
#include "drivers/parallel.h"
#include "drivers/audio.h"
#include "drivers/disk.h"
#include "drivers/display.h"
#include "drivers/console.h"

static device_t *device_list_head = NULL;
static uint32_t device_count = 0;
static bool device_subsystem_initialized = false;

static const char *device_class_names[] = {
    [DEVICE_CLASS_CHAR] = "char",
    [DEVICE_CLASS_BLOCK] = "block",
    [DEVICE_CLASS_DISPLAY] = "display",
};

static const char *device_state_names[] = {
    [DEVICE_STATE_UNINITIALIZED] = "uninitialized",
    [DEVICE_STATE_INITIALIZING] = "initializing",
    [DEVICE_STATE_READY] = "ready",
    [DEVICE_STATE_ERROR] = "error", 
    [DEVICE_STATE_REMOVED] = "removed"
};

arch_result device_init(void)
{
    if (device_subsystem_initialized) {
        return ARCH_OK;
    }
    
    device_list_head = NULL;
    device_count = 0;
    device_subsystem_initialized = true;
    
    arch_debug_printf("Device subsystem initialized\n");
    return ARCH_OK;
}

arch_result device_init_drivers(void)
{
    if (!device_subsystem_initialized) {
        return ARCH_ERROR;
    }
    
    arch_debug_printf("Initializing device drivers...\n");
    
    arch_result results[] = {
        serial_driver_init(),
        parallel_driver_init(), 
        keyboard_driver_init(),
        audio_driver_init(),
        disk_driver_init(),
        display_driver_init(),
        console_driver_init()
    };
    
    const char *driver_names[] = {
        "serial", "parallel", "keyboard", "audio", "disk", "display", "console"
    };
    
    int failed_count = 0;
    for (int i = 0; i < 7; i++) {
        if (results[i] != ARCH_OK) {
            arch_debug_printf("❌ %s driver failed\n", driver_names[i]);
            failed_count++;
        }
    }
    
    arch_debug_printf("Device drivers initialized (%d/%d successful)\n", 7 - failed_count, 7);
    return (failed_count == 0) ? ARCH_OK : ARCH_ERROR;
}

arch_result device_register(device_t *device)
{
    if (!device_subsystem_initialized) {
        return ARCH_ERROR;
    }
    
    if (!device || !device->name[0]) {
        return ARCH_INVALID;
    }
    
    if (device_find_by_name(device->name)) {
        arch_debug_printf("Device '%s' already registered\n", device->name);
        return ARCH_ERROR;
    }
    
    if (device->class >= DEVICE_CLASS_MAX) {
        return ARCH_INVALID;
    }
    
    device->state = DEVICE_STATE_INITIALIZING;
    device->next = NULL;
    
    if (device_list_head == NULL) {
        device_list_head = device;
    } else {
        device_t *current = device_list_head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = device;
    }
    
    device_count++;
    
    if (device->open) {
        arch_result result = device->open(device);
        if (result == ARCH_OK) {
            device->state = DEVICE_STATE_READY;
        } else {
            device->state = DEVICE_STATE_ERROR;
            arch_debug_printf("Device '%s' initialization failed\n", device->name);
            return result;
        }
    } else {
        device->state = DEVICE_STATE_READY;
    }
    
    arch_debug_printf("Registered %s device '%s'\n",
                     device_class_name(device->class), device->name);
    
    return ARCH_OK;
}

arch_result device_unregister(device_t *device)
{
    if (!device_subsystem_initialized || !device) {
        return ARCH_INVALID;
    }
    
    if (device_list_head == device) {
        device_list_head = device->next;
    } else {
        device_t *current = device_list_head;
        while (current && current->next != device) {
            current = current->next;
        }
        if (current) {
            current->next = device->next;
        } else {
            return ARCH_ERROR;
        }
    }
    
    if (device->close) {
        device->close(device);
    }
    
    device->state = DEVICE_STATE_REMOVED;
    device_count--;
    
    arch_debug_printf("Unregistered device '%s'\n", device->name);
    return ARCH_OK;
}

device_t* device_find_by_name(const char *name)
{
    if (!device_subsystem_initialized || !name) {
        return NULL;
    }
    
    device_t *current = device_list_head;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

device_t* device_find_by_class(device_class_t class, uint32_t index)
{
    if (!device_subsystem_initialized || class >= DEVICE_CLASS_MAX) {
        return NULL;
    }
    
    uint32_t found_count = 0;
    device_t *current = device_list_head;
    
    while (current) {
        if (current->class == class && current->state == DEVICE_STATE_READY) {
            if (found_count == index) {
                return current;
            }
            found_count++;
        }
        current = current->next;
    }
    
    return NULL;
}

void device_list_all(void)
{
    if (!device_subsystem_initialized) {
        arch_debug_printf("Device subsystem not initialized\n");
        return;
    }
    
    arch_debug_printf("Registered devices (%u total):\n", device_count);
    
    device_t *current = device_list_head;
    while (current) {
        arch_debug_printf("  %s: %s device, state=%s\n",
                         current->name,
                         device_class_name(current->class),
                         device_state_name(current->state));
        current = current->next;
    }
}

const char* device_class_name(device_class_t class)
{
    if (class >= DEVICE_CLASS_MAX) {
        return "unknown";
    }
    return device_class_names[class];
}

const char* device_state_name(device_state_t state)
{
    if (state > DEVICE_STATE_REMOVED) {
        return "unknown";
    }
    return device_state_names[state];
}
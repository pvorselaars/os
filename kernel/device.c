// kernel/device.c
#include "kernel/device.h"
#include "lib/string.h"

static device_t *devices;

result_t device_register(device_t *device)
{
    if (!device || !device->name || device->class >= DEVICE_CLASS_MAX ||
        device_find_by_name(device->name)) {
        return RESULT_ERROR;
        }

    if (device->init && device->init(device) != RESULT_OK) {
        return RESULT_ERROR;
    }

    device->next = devices;
    devices = device;
    return RESULT_OK;
}

device_t *device_find_by_name(const char *name)
{
    for (device_t *device = devices; device; device = device->next) {
        if (strcmp(device->name, name) == 0) {
            return device;
        }
    }

    return NULL;
}

device_t *device_find_by_class(device_class_t class, uint32_t index)
{
    for (device_t *device = devices; device; device = device->next) {
        if (device->class == class && index-- == 0) {
            return device;
        }
    }

    return NULL;
}
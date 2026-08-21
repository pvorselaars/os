#include "board/pc/audio.h"

#include "arch/arch.h"
#include "kernel/device.h"

#define PIT_FREQUENCY    1193180    // PIT base frequency
#define PIT_COMMAND      0x43       // PIT command register
#define PIT_CHANNEL_2    0x42       // PIT channel 2 (PC speaker)
#define SPEAKER_PORT     0x61       // PC speaker control port

typedef struct {
    bool initialized;
    bool is_playing;
    uint32_t current_frequency;
} pc_speaker_device_t;

static pc_speaker_device_t pc_speaker = {0};

static result_t pc_speaker_init(device_t *device)
{
    pc_speaker_device_t *speaker = device->data;

    uint8_t speaker_control = arch_io_read8(SPEAKER_PORT) & 0xFC;
    arch_io_write8(SPEAKER_PORT, speaker_control);

    speaker->initialized = true;
    speaker->is_playing = false;
    speaker->current_frequency = 0;
    return RESULT_OK;
}

static result_t pc_speaker_stop(pc_speaker_device_t *spk)
{
    if (!spk->initialized) {
        return RESULT_ERROR;
    }

    uint8_t speaker_control = arch_io_read8(SPEAKER_PORT) & 0xFC;
    arch_io_write8(SPEAKER_PORT, speaker_control);
    
    spk->is_playing = false;
    spk->current_frequency = 0;
    
    return RESULT_OK;
}

static result_t pc_speaker_play(pc_speaker_device_t *spk, uint32_t frequency)
{
    if (!spk->initialized) {
        return RESULT_ERROR;
    }

    if (frequency == 0) {
        return pc_speaker_stop(spk);
    }

    uint32_t divisor = PIT_FREQUENCY / frequency;
    if (divisor == 0) {
        divisor = 1;
    } else if (divisor > 0xFFFF) {
        divisor = 0xFFFF;
    }

    arch_io_write8(PIT_COMMAND, 0xB6);
    arch_io_write8(PIT_CHANNEL_2, (uint8_t)(divisor & 0xFF));
    arch_io_write8(PIT_CHANNEL_2, (uint8_t)((divisor >> 8) & 0xFF));

    uint8_t speaker_control = arch_io_read8(SPEAKER_PORT);
    arch_io_write8(SPEAKER_PORT, speaker_control | 0x03);

    spk->is_playing = true;
    spk->current_frequency = frequency;
    return RESULT_OK;
}

static int pc_speaker_read(device_t *device, void *buffer, size_t length)
{
    (void)device;
    (void)buffer;
    (void)length;
    return -1;
}

static int pc_speaker_write(device_t *device, const void *buffer, size_t length)
{
    if (!buffer || length != sizeof(uint32_t)) {
        return -1;
    }

    const uint8_t *bytes = buffer;
    uint32_t frequency =
        (uint32_t)bytes[0] |
        ((uint32_t)bytes[1] << 8) |
        ((uint32_t)bytes[2] << 16) |
        ((uint32_t)bytes[3] << 24);

    return pc_speaker_play(device->data, frequency) == RESULT_OK
        ? (int)length
        : -1;
}

static device_t pc_speaker_device = {
    .name = "pcspk0",
    .class = DEVICE_CLASS_CHAR,
    .init = pc_speaker_init,
    .char_ops = {
        .read = pc_speaker_read,
        .write = pc_speaker_write,
    },
    .data = &pc_speaker,
};

result_t pc_audio_register(void)
{
    return device_register(&pc_speaker_device);
}
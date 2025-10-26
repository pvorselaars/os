#include "arch/arch.h"
#include "arch/x86_64/io.h"

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

int arch_audio_get_count(void)
{
    return 1;
}

arch_result arch_audio_get_info(int index, arch_audio_info_t *info)
{
    if (index != 0 || !info) {
        return ARCH_ERROR;
    }
    
    info->device = (arch_audio_device_t *)&pc_speaker;
    info->name = "pcspk0";
    
    return ARCH_OK;
}

arch_result arch_audio_init(arch_audio_device_t *device)
{
    pc_speaker_device_t *spk = (pc_speaker_device_t *)device;
    
    if (spk != &pc_speaker) {
        return ARCH_ERROR;
    }
    
    uint8_t speaker_control = inb(SPEAKER_PORT) & 0xFC;
    outb(SPEAKER_PORT, speaker_control);
    
    spk->initialized = true;
    spk->is_playing = false;
    spk->current_frequency = 0;
    
    return ARCH_OK;
}

arch_result arch_audio_play_tone(arch_audio_device_t *device, uint32_t frequency)
{
    pc_speaker_device_t *spk = (pc_speaker_device_t *)device;
    
    if (spk != &pc_speaker || !spk->initialized) {
        return ARCH_ERROR;
    }
    
    if (frequency == 0) {
        return arch_audio_stop(device);
    }
    
    uint32_t divisor = PIT_FREQUENCY / frequency;
    
    outb(PIT_COMMAND, 0xB6);  // Channel 2, lobyte/hibyte, square wave mode
    outb(PIT_CHANNEL_2, (uint8_t)(divisor & 0xFF));        // Low byte
    outb(PIT_CHANNEL_2, (uint8_t)((divisor >> 8) & 0xFF)); // High byte
    
    uint8_t speaker_control = inb(SPEAKER_PORT);
    outb(SPEAKER_PORT, speaker_control | 0x03);
    
    spk->is_playing = true;
    spk->current_frequency = frequency;
    
    return ARCH_OK;
}

arch_result arch_audio_stop(arch_audio_device_t *device)
{
    pc_speaker_device_t *spk = (pc_speaker_device_t *)device;
    
    if (spk != &pc_speaker || !spk->initialized) {
        return ARCH_ERROR;
    }
    
    uint8_t speaker_control = inb(SPEAKER_PORT) & 0xFC;
    outb(SPEAKER_PORT, speaker_control);
    
    spk->is_playing = false;
    spk->current_frequency = 0;
    
    return ARCH_OK;
}
#include "board/pc/disk.h"

#include "board/pc/io.h"
#include "kernel/device.h"

#define ATA_DATA         0x1F0
#define ATA_SECTOR_COUNT 0x1F2
#define ATA_LBA_LOW      0x1F3
#define ATA_LBA_MID      0x1F4
#define ATA_LBA_HIGH     0x1F5
#define ATA_DRIVE_SELECT 0x1F6
#define ATA_STATUS       0x1F7
#define ATA_COMMAND      0x1F7

#define ATA_CMD_READ_SECTORS_EXT  0x24
#define ATA_CMD_WRITE_SECTORS_EXT 0x34

#define ATA_STATUS_BUSY  0x80
#define ATA_STATUS_READY 0x40
#define ATA_STATUS_DRQ   0x08
#define ATA_STATUS_ERROR 0x01

typedef struct {
    bool initialized;
    uint64_t block_count;
    uint32_t block_size;
} ata_disk_t;

static ata_disk_t ata_disk = {
    .block_count = 10,
    .block_size = 512,
};

static result_t ata_wait_ready(void)
{
    for (int timeout = 10000; timeout > 0; timeout--) {
        uint8_t status = pc_io_read8(ATA_STATUS);
        if (!(status & ATA_STATUS_BUSY) && (status & ATA_STATUS_READY)) {
            return RESULT_OK;
        }
    }

    return RESULT_ERROR;
}

static result_t ata_wait_data(void)
{
    for (int timeout = 10000; timeout > 0; timeout--) {
        uint8_t status = pc_io_read8(ATA_STATUS);
        if (status & ATA_STATUS_ERROR) {
            return RESULT_ERROR;
        }
        if (!(status & ATA_STATUS_BUSY) && (status & ATA_STATUS_DRQ)) {
            return RESULT_OK;
        }
    }

    return RESULT_ERROR;
}

static result_t ata_select_sector(uint64_t lba, uint8_t command)
{
    if (ata_wait_ready() != RESULT_OK) {
        return RESULT_ERROR;
    }

    pc_io_write8(ATA_DRIVE_SELECT, 0x40);
    pc_io_write8(ATA_SECTOR_COUNT, 0);
    pc_io_write8(ATA_LBA_LOW, (uint8_t)(lba >> 24));
    pc_io_write8(ATA_LBA_MID, (uint8_t)(lba >> 32));
    pc_io_write8(ATA_LBA_HIGH, (uint8_t)(lba >> 40));
    pc_io_write8(ATA_SECTOR_COUNT, 1);
    pc_io_write8(ATA_LBA_LOW, (uint8_t)lba);
    pc_io_write8(ATA_LBA_MID, (uint8_t)(lba >> 8));
    pc_io_write8(ATA_LBA_HIGH, (uint8_t)(lba >> 16));
    pc_io_write8(ATA_COMMAND, command);

    return ata_wait_data();
}

static result_t ata_disk_init(device_t *device)
{
    ata_disk_t *disk = device_data(device);

    disk->initialized = true;
    return RESULT_OK;
}

static result_t ata_disk_sync(device_t *device)
{
    ata_disk_t *disk = device_data(device);

    if (!disk->initialized) {
        return RESULT_ERROR;
    }

    return ata_wait_ready();
}

static int ata_disk_read_blocks(
    device_t *device,
    void *buffer,
    uint64_t start_block,
    uint32_t block_count
) {
    ata_disk_t *disk = device_data(device);

    if (!disk->initialized || !buffer || block_count == 0 ||
        start_block >= disk->block_count ||
        block_count > disk->block_count - start_block) {
        return -1;
    }

    uint16_t *words = buffer;
    for (uint32_t block = 0; block < block_count; block++) {
        if (ata_select_sector(start_block + block, ATA_CMD_READ_SECTORS_EXT) != RESULT_OK) {
            return -1;
        }

        for (int word = 0; word < 256; word++) {
            *words++ = pc_io_read16(ATA_DATA);
        }
    }

    return (int)block_count;
}

static int ata_disk_write_blocks(
    device_t *device,
    const void *buffer,
    uint64_t start_block,
    uint32_t block_count
) {
    ata_disk_t *disk = device_data(device);

    if (!disk->initialized || !buffer || block_count == 0 ||
        start_block >= disk->block_count ||
        block_count > disk->block_count - start_block) {
        return -1;
    }

    const uint16_t *words = buffer;
    for (uint32_t block = 0; block < block_count; block++) {
        if (ata_select_sector(start_block + block, ATA_CMD_WRITE_SECTORS_EXT) != RESULT_OK) {
            return -1;
        }

        for (int word = 0; word < 256; word++) {
            pc_io_write16(ATA_DATA, *words++);
        }
    }

    return (int)block_count;
}

static uint32_t ata_disk_get_block_size(device_t *device)
{
    return ((ata_disk_t *)device_data(device))->block_size;
}

static uint64_t ata_disk_get_block_count(device_t *device)
{
    return ((ata_disk_t *)device_data(device))->block_count;
}

static const device_descriptor_t ata_disk_device = {
    .name = "ata0",
    .class = DEVICE_CLASS_BLOCK,
    .init = ata_disk_init,
    .block_ops = {
        .read_blocks = ata_disk_read_blocks,
        .write_blocks = ata_disk_write_blocks,
        .sync = ata_disk_sync,
        .get_block_size = ata_disk_get_block_size,
        .get_block_count = ata_disk_get_block_count,
    },
    .data = &ata_disk,
};

result_t pc_disk_register(void)
{
    return device_register(&ata_disk_device);
}

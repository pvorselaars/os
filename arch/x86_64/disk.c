#include "arch/arch.h"
#include "arch/x86_64/io.h"

/* x86_64 ATA/IDE Disk

  ATA Registers:

  - 0x1F0: Data port
  - 0x1F1: Features/Error information  
  - 0x1F2: Sector count
  - 0x1F3: LBA low (bits 0-7)
  - 0x1F4: LBA mid (bits 8-15)
  - 0x1F5: LBA high (bits 16-23)
  - 0x1F6: Drive select + LBA bits 24-27
  - 0x1F7: Command/Status register
 */

// ATA Controller ports
#define ATA_DATA        0x1F0
#define ATA_ERROR       0x1F1
#define ATA_FEATURES    0x1F1
#define ATA_SECTOR_COUNT 0x1F2
#define ATA_LBA_LOW     0x1F3
#define ATA_LBA_MID     0x1F4
#define ATA_LBA_HIGH    0x1F5
#define ATA_DRIVE_SELECT 0x1F6
#define ATA_STATUS      0x1F7
#define ATA_COMMAND     0x1F7

// ATA Commands
#define ATA_CMD_READ_SECTORS_EXT  0x24  // READ SECTORS EXT (48-bit LBA)
#define ATA_CMD_WRITE_SECTORS_EXT 0x34  // WRITE SECTORS EXT (48-bit LBA)

// ATA Status bits
#define ATA_STATUS_BUSY    0x80
#define ATA_STATUS_READY   0x40
#define ATA_STATUS_DRQ     0x08  // Data Request
#define ATA_STATUS_ERROR   0x01

// Simple disk device - assume primary master for now
typedef struct {
    bool initialized;
    uint64_t block_count;    // Total sectors
    uint32_t block_size;     // Bytes per sector (512)
} ata_disk_device_t;

static ata_disk_device_t ata_disk = {0};

/* Wait for ATA controller to be ready */
static arch_result ata_wait_ready(void)
{
    uint8_t status;
    int timeout = 10000;
    
    do {
        status = inb(ATA_STATUS);
        if (!(status & ATA_STATUS_BUSY) && (status & ATA_STATUS_READY)) {
            return ARCH_OK;
        }
        timeout--;
    } while (timeout > 0);
    
    return ARCH_ERROR; // Timeout
}

/* Wait for data to be ready */
static arch_result ata_wait_data(void)
{
    uint8_t status;
    int timeout = 10000;
    
    do {
        status = inb(ATA_STATUS);
        if (!(status & ATA_STATUS_BUSY) && (status & ATA_STATUS_DRQ)) {
            return ARCH_OK;
        }
        if (status & ATA_STATUS_ERROR) {
            return ARCH_ERROR;
        }
        timeout--;
    } while (timeout > 0);
    
    return ARCH_ERROR; // Timeout or error
}


int arch_disk_get_count(void)
{
    return 1;
}

arch_result arch_disk_get_info(int index, arch_disk_info_t *info)
{
    if (index != 0 || !info) {
        return ARCH_ERROR;
    }
    
    // Fill in ATA disk info
    info->device = (arch_disk_device_t *)&ata_disk;
    info->name = "ata0";
    info->block_size = 512;    // Standard sector size
    info->block_count = 10;    // 5MB for testing (10 * 512 bytes)
    info->read_only = false;
    
    return ARCH_OK;
}

arch_result arch_disk_init(arch_disk_device_t *device)
{
    ata_disk_device_t *disk = (ata_disk_device_t *)device;
    
    if (disk != &ata_disk) {
        return ARCH_ERROR;
    }
    
    disk->initialized = true;
    disk->block_size = 512;

    // TODO: detect actual size
    disk->block_count = 10; // 5MB
    
    return ARCH_OK;
}

arch_result arch_disk_read_blocks(arch_disk_device_t *device, void *buf, uint64_t start_block, uint32_t block_count)
{
    ata_disk_device_t *disk = (ata_disk_device_t *)device;
    
    if (disk != &ata_disk || !disk->initialized || !buf || block_count == 0) {
        return ARCH_ERROR;
    }
    
    uint16_t *buffer = (uint16_t *)buf;
    
    for (uint32_t block = 0; block < block_count; block++) {
        uint64_t lba = start_block + block;
        
        if (ata_wait_ready() != ARCH_OK) {
            return ARCH_ERROR;
        }
        
        outb(ATA_DRIVE_SELECT, 0x40);               // Select master drive, LBA mode
        outb(ATA_SECTOR_COUNT, (block_count >> 8) & 0xFF);  // Sector count high
        outb(ATA_LBA_LOW, (lba >> 24) & 0xFF);      // LBA bits 24-31
        outb(ATA_LBA_MID, (lba >> 32) & 0xFF);      // LBA bits 32-39  
        outb(ATA_LBA_HIGH, (lba >> 40) & 0xFF);     // LBA bits 40-47
        outb(ATA_SECTOR_COUNT, block_count & 0xFF); // Sector count low
        outb(ATA_LBA_LOW, lba & 0xFF);              // LBA bits 0-7
        outb(ATA_LBA_MID, (lba >> 8) & 0xFF);       // LBA bits 8-15
        outb(ATA_LBA_HIGH, (lba >> 16) & 0xFF);     // LBA bits 16-23
        outb(ATA_COMMAND, ATA_CMD_READ_SECTORS_EXT); // Issue read command
        
        if (ata_wait_data() != ARCH_OK) {
            return ARCH_ERROR;
        }
        
        for (int word = 0; word < 256; word++) {
            *buffer = inw(ATA_DATA);
            buffer++;
        }
    }
    
    return ARCH_OK;
}

arch_result arch_disk_write_blocks(arch_disk_device_t *device, const void *buf, uint64_t start_block, uint32_t block_count)
{
    ata_disk_device_t *disk = (ata_disk_device_t *)device;
    
    if (disk != &ata_disk || !disk->initialized || !buf || block_count == 0) {
        return ARCH_ERROR;
    }
    
    const uint16_t *buffer = (const uint16_t *)buf;
    
    for (uint32_t block = 0; block < block_count; block++) {
        uint64_t lba = start_block + block;
        
        if (ata_wait_ready() != ARCH_OK) {
            return ARCH_ERROR;
        }
        
        outb(ATA_DRIVE_SELECT, 0x40);               // Select master drive, LBA mode
        outb(ATA_SECTOR_COUNT, (block_count >> 8) & 0xFF);  // Sector count high
        outb(ATA_LBA_LOW, (lba >> 24) & 0xFF);      // LBA bits 24-31
        outb(ATA_LBA_MID, (lba >> 32) & 0xFF);      // LBA bits 32-39
        outb(ATA_LBA_HIGH, (lba >> 40) & 0xFF);     // LBA bits 40-47
        outb(ATA_SECTOR_COUNT, block_count & 0xFF); // Sector count low
        outb(ATA_LBA_LOW, lba & 0xFF);              // LBA bits 0-7
        outb(ATA_LBA_MID, (lba >> 8) & 0xFF);       // LBA bits 8-15
        outb(ATA_LBA_HIGH, (lba >> 16) & 0xFF);     // LBA bits 16-23
        outb(ATA_COMMAND, ATA_CMD_WRITE_SECTORS_EXT); // Issue write command
        
        if (ata_wait_data() != ARCH_OK) {
            return ARCH_ERROR;
        }
        
        for (int word = 0; word < 256; word++) {
            outw(ATA_DATA, *buffer);
            buffer++;
        }
    }
    
    return ARCH_OK;
}

arch_result arch_disk_sync(arch_disk_device_t *device)
{
    ata_disk_device_t *disk = (ata_disk_device_t *)device;
    
    if (disk != &ata_disk || !disk->initialized) {
        return ARCH_ERROR;
    }
    
    // TODO: flush command
    return ata_wait_ready();
}
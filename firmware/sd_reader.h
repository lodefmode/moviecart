

bool		disk_initialize();
uint8_t*    disk_read_block1 (uint32_t sector);
void        disk_read_block2 (uint32_t sector, uint8_t *dst);


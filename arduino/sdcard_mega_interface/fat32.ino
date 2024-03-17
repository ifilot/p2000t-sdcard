/**
 * Extract metadata from the first sector of the FAT
 */
void analyse_first_sector_fat(uint8_t* block, FATData* fatdata) {
  memcpy(&fatdata->bytes_per_sector, &block[0x0B], sizeof(uint16_t));
  memcpy(&fatdata->sectors_per_cluster, &block[0x0D], sizeof(uint8_t));
  memcpy(&fatdata->reserved_sectors, &block[0x0E], sizeof(uint16_t));
  memcpy(&fatdata->number_of_fats, &block[0x10], sizeof(uint8_t));
  memcpy(&fatdata->sectors_per_fat, &block[0x24], sizeof(uint32_t));
  memcpy(&fatdata->rootdir_first_cluster, &block[0x2C], sizeof(uint32_t));
  memcpy(&fatdata->signature, &block[0x1FE], sizeof(uint16_t));
}

/**
 * Print the metadata of the FAT
 */
void print_fat32_data(FATData* fatdata) {
  Serial.print("Bytes per sector: ");
  Serial.print(fatdata->bytes_per_sector, HEX);
  Serial.println();

  Serial.print("Sectors per cluster: ");
  Serial.print(fatdata->sectors_per_cluster, HEX);
  Serial.println();

  Serial.print("Number of reserved sectors: ");
  Serial.print(fatdata->reserved_sectors, HEX);
  Serial.println();

  Serial.print("Number of FATS: ");
  Serial.print(fatdata->number_of_fats, HEX);
  Serial.println();

  Serial.print("Sectors per FATS: ");
  Serial.print(fatdata->sectors_per_fat, HEX);
  Serial.println();

  Serial.print("Root directory first cluster: ");
  Serial.print(fatdata->rootdir_first_cluster, HEX);
  Serial.println();

  Serial.print("Signature: ");
  Serial.print(fatdata->signature, HEX);
  Serial.println();
}

/**
 * Analyse the partitions
 */
void analyse_partitions(uint8_t* block) {
  for(int i=0; i<4; i++) {
    Serial.print("Partition ");
    Serial.print(i, HEX);
    Serial.print(": ");
    uint16_t pstart = 514 - 4 - (4-i)*16;
    for(unsigned int j=0; j<16; j++) {
      if(block[pstart + j] < 0x10) {
        Serial.print("0");
      }
      Serial.print(block[pstart + j], HEX); 
      Serial.print(" ");
    }
    Serial.println();
    // note that FAT32 uses little endian encoding
    uint32_t lba_begin = 0;
    memcpy(&lba_begin, &block[pstart + 8], sizeof(uint32_t));
    uint32_t num_sec = 0;
    memcpy(&num_sec, &block[pstart + 12], sizeof(uint32_t));
    uint8_t type_code = block[pstart + 4];
    Serial.print("Type code: ");
    Serial.print(type_code, HEX);
    Serial.println();
    Serial.print("LBA begin: ");
    Serial.print(lba_begin, HEX);
    Serial.println();
    Serial.print("Number of sectors: ");
    Serial.print(num_sec, HEX);
    Serial.println();
  }
}

/**
 * Get the location of the first partition
 * (LBA = Logical Block Addressing)
 */
uint32_t get_lba_begin(uint8_t* block) {
  uint16_t pstart = 514 - 4 - 4 * 16;
  return block[pstart + 8];
}

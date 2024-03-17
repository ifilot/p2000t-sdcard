struct FATData {
  uint16_t bytes_per_sector;
  uint8_t sectors_per_cluster;
  uint16_t reserved_sectors;
  uint8_t number_of_fats;
  uint32_t sectors_per_fat;
  uint32_t rootdir_first_cluster;
  uint16_t signature;
};

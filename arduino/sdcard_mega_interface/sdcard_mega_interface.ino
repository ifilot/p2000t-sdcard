#include "fat32.h"

void setup() {
  init_sdcard();
}

uint8_t resp[6];    // use a 6 bit response buffer
uint8_t block[514]; // use 512 bytes for reading blocks + 2 bytes for the checksum

void loop() {
  // open the sd card interface
  access_sd();

  // read first sector
  open_command();
  cmd17(resp, 0x00000000);
  print_response("CMD17", resp, 1);
  read_block(block);
  close_command();
  //print_datablock(block);
  analyse_partitions(block);

  // obtain first sector of partition
  uint32_t lba_begin = get_lba_begin(block);
  open_command();
  cmd17(resp, lba_begin);
  print_response("CMD17", resp, 1);
  read_block(block);
  close_command();
  //print_datablock(block);
  FATData fatdata;
  analyse_first_sector_fat(block, &fatdata);
  print_fat32_data(&fatdata);

  while(1){} // set infinite loop
}

/**
 * Access the SD card
 * 
 * CMD0 > CMD8 > ACMD41 -> ready
 */
void access_sd() {
  cs_reset();

  /* 
   * first send 72 pulses to the SD card 
   */
  for(uint8_t i=0; i<12; i++) {
    write_byte(0xFF);  
  }

  open_command();
  cmd0(resp);
  print_response("CMD0", resp, 1);

  cmd8(resp);
  print_response("CMD8", resp, 5);

   /*
   * WRITE ACDM41
   */

  resp[0] = 5;
  uint8_t ctr = 0;
  while(resp[0] != 0) {
    ctr++;
    cmd55(resp);
    //print_response("CMD55", resp, 1);
    
    acmd41(resp);
    //print_response("ACMD41", resp, 1);
  
    delay(50);
  }
  Serial.print("Accepted ACMD41: ");
  Serial.print(ctr);
  Serial.println(" attempts.");

  cmd58(resp);
  print_response("CMD58", resp, 5);
  close_command();
}

#include <stdint.h>

#include "maple_command.h"

struct maple_memcard {
  uint8_t address;
  uint8_t part;
  uint16_t blocksz;
  uint8_t writecnt;
  uint8_t readcnt;
  uint8_t removable;
  uint32_t funcdata;
  struct maple_mem_info meminfo;
  uint32_t fat_entries;
  uint32_t dir_entries;
  uint16_t fat_entries_per_block;
  uint16_t dir_entries_per_block;
  void *fat_data;
  void *dir_data;
};

struct maple_memcard_timestamp {
  uint8_t century;
  uint8_t year_in_century;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint8_t weekday;
};

struct maple_memcard_direntry {
  uint8_t file_type;
  uint8_t copy_protect;
  uint16_t first_block_loc;
  char filename[12];
  struct maple_memcard_timestamp creation_time;
  uint16_t file_size;
  uint16_t header_offset;
  uint8_t pad[4];
};

#define MAPLE_MEMCARD_FUNCDATA_PARTITIONS(fd) ((((fd)>>24)&0xff)+1)
#define MAPLE_MEMCARD_FUNCDATA_BLOCKSZ(fd)    (((((fd)>>16)&0xff)+1)<<5)
#define MAPLE_MEMCARD_FUNCDATA_WRITECNT(fd)   (((fd)>>12)&0xf)
#define MAPLE_MEMCARD_FUNCDATA_READCNT(fd)    (((fd)>>8)&0xf)
#define MAPLE_MEMCARD_FUNCDATA_REMOVABLE(fd)  (((fd)>>7)&0x1)

#define MAPLE_MEMCARD_FILE_TYPE_DATA 0x33
#define MAPLE_MEMCARD_FILE_TYPE_GAME 0xcc

int maple_open_memcard(uint8_t address, uint32_t funcdata, uint8_t part, struct maple_memcard *mcard);
void maple_close_memcard(struct maple_memcard *mcard);
int maple_read_memcard_blocks(struct maple_memcard *mcard, uint16_t start, uint32_t cnt, void *buf);
int maple_read_memcard_chained_blocks(struct maple_memcard *mcard, uint16_t start, uint32_t cnt, void *buf);
int maple_follow_memcard_fat(struct maple_memcard *mcard, uint16_t *loc);
int maple_get_memcard_fat_entry(struct maple_memcard *mcard, uint16_t loc, uint16_t *entry);
int maple_next_memcard_direntry(struct maple_memcard *mcard, const char *glob, uint32_t *index, struct maple_memcard_direntry *entry);
int maple_get_memcard_direntry(struct maple_memcard *mcard, uint32_t num, struct maple_memcard_direntry *entry);

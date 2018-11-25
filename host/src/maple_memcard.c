#include "maple_memcard.h"
#include "maple_command.h"
#include "maple_function.h"
#include "maple_error_codes.h"
#include "maple_endian.h"

#include <stdlib.h>
#include <string.h>

int maple_open_memcard(uint8_t address, uint32_t funcdata, uint8_t part, struct maple_memcard *mcard)
{
  int r;

  memset(mcard, 0, sizeof(*mcard));
  mcard->fat_data = NULL;
  mcard->dir_data = NULL;
  if (part >= MAPLE_MEMCARD_FUNCDATA_PARTITIONS(funcdata))
    return MAPLE_ERROR_BAD_ARGUMENT;
  mcard->address = address;
  mcard->part = part;
  mcard->blocksz = MAPLE_MEMCARD_FUNCDATA_BLOCKSZ(funcdata);
  mcard->writecnt = MAPLE_MEMCARD_FUNCDATA_WRITECNT(funcdata);
  mcard->readcnt = MAPLE_MEMCARD_FUNCDATA_READCNT(funcdata);
  mcard->removable = MAPLE_MEMCARD_FUNCDATA_REMOVABLE(funcdata);
  mcard->funcdata = funcdata;

  r = maple_get_mem_info(address, 1U<<MAPLE_FUNCTION_MEMORY_CARD, part,
			 &mcard->meminfo);
  if (r != MAPLE_OK)
    return r;

  mcard->fat_entries_per_block = mcard->blocksz / 2;
  mcard->dir_entries_per_block = mcard->blocksz / 32;
  mcard->fat_entries = mcard->fat_entries_per_block * mcard->meminfo.fat_size;
  mcard->dir_entries = mcard->dir_entries_per_block * mcard->meminfo.dir_size;

  if (!(mcard->fat_data = calloc(mcard->meminfo.fat_size, mcard->blocksz)))
    return MAPLE_ERROR_OUT_OF_MEMORY;
  if (!(mcard->dir_data = calloc(mcard->meminfo.dir_size, mcard->blocksz)))
    return MAPLE_ERROR_OUT_OF_MEMORY;

  r = maple_read_memcard_blocks(mcard, mcard->meminfo.fat_loc, mcard->meminfo.fat_size, mcard->fat_data);
  if (r == MAPLE_OK)
    r = maple_read_memcard_chained_blocks(mcard, mcard->meminfo.dir_loc, mcard->meminfo.dir_size, mcard->dir_data);

  return r;
}

void maple_close_memcard(struct maple_memcard *mcard)
{
  if (!mcard)
    return;

  free(mcard->fat_data);
  free(mcard->dir_data);
  memset(mcard, 0, sizeof(*mcard));
}

int maple_read_memcard_blocks(struct maple_memcard *mcard, uint16_t start, uint32_t cnt, void *buf)
{
  int r;
  if (!mcard || start > mcard->meminfo.last_block || !mcard->readcnt)
    return MAPLE_ERROR_BAD_ARGUMENT;
  if (cnt == 0)
    return MAPLE_OK;
  if (cnt - 1 > mcard->meminfo.last_block - start || !buf)
    return MAPLE_ERROR_BAD_ARGUMENT;
  do {
    uint16_t per_phase = (mcard->blocksz + mcard->readcnt - 1) / mcard->readcnt;
    uint16_t offset = 0;
    uint8_t phase = 0;
    while (offset < mcard->blocksz) {
      uint16_t phasesz = (per_phase > mcard->blocksz - offset? mcard->blocksz - offset : per_phase);
      r = maple_block_read(mcard->address, 1U<<MAPLE_FUNCTION_MEMORY_CARD, mcard->part, phase, start, buf, phasesz);
      if (r != MAPLE_OK)
	return r;
      offset += phasesz;
      ++phase;
    }
    start ++;
    buf = ((char *)buf) + mcard->blocksz;
  } while(--cnt);
  return MAPLE_OK;
}

int maple_read_memcard_chained_blocks(struct maple_memcard *mcard, uint16_t start, uint32_t cnt, void *buf)
{
  int r = MAPLE_OK;
  if (!mcard || start > mcard->meminfo.last_block)
    return MAPLE_ERROR_BAD_ARGUMENT;
  if (cnt == 0)
    return MAPLE_OK;
  do {
    r = maple_read_memcard_blocks(mcard, start, 1, buf);
    if (r != MAPLE_OK || !--cnt)
      break;
    buf = ((char *)buf) + mcard->blocksz;
    r = maple_follow_memcard_fat(mcard, &start);
  } while(r == MAPLE_OK);
  return r;
}

int maple_follow_memcard_fat(struct maple_memcard *mcard, uint16_t *loc)
{
  int r;
  uint16_t entry;
  r = maple_get_memcard_fat_entry(mcard, *loc, &entry);
  if (r != MAPLE_OK)
    return r;
  if (entry == 0xfffa || entry == 0xfffc || entry > mcard->meminfo.last_block)
    return MAPLE_ERROR_INVALID_NEXT_FAT_ENTRY;
  *loc = entry;
  return MAPLE_OK;
}

int maple_get_memcard_fat_entry(struct maple_memcard *mcard, uint16_t loc, uint16_t *entry)
{
  if (mcard && entry && loc < mcard->fat_entries) {
    uint16_t e = ((uint16_t *)mcard->fat_data)[loc];
#if MAPLE_HOST_BIG_ENDIAN
    e = maple_bswap16(e);
#endif
    *entry = e;
    return MAPLE_OK;
  } else {
    *entry = 0xfffc;
    return MAPLE_ERROR_BAD_ARGUMENT;
  }
}

static int match_glob(const char *glob, const char *filename, int limit)
{
  while (limit > 0) {
    if (*glob == '*') {
      if (match_glob(glob+1, filename, limit))
	return 1;
    } else if (*glob) {
      char match = *glob++;
      if (match != '?' && *filename != match)
	return 0;
    } else {
      if (!*filename)
	return 1;
      if (*filename != ' ')
	return 0;
    }
    ++filename;
    --limit;
  }
  while(*glob == '*')
    glob++;
  return !*glob;
}

int maple_next_memcard_direntry(struct maple_memcard *mcard, const char *glob, uint32_t *index, struct maple_memcard_direntry *entry)
{
  uint32_t n = (index? *index : 0);
  if (!entry || !mcard)
    return MAPLE_ERROR_BAD_ARGUMENT;
  while (n < mcard->dir_entries) {
    int r = maple_get_memcard_direntry(mcard, n, entry);
    if (r != MAPLE_OK)
      return r;
    n++;
    if (entry->file_type && (!glob || match_glob(glob, entry->filename, 12))) {
      if (index)
	*index = n;
      return MAPLE_OK;
    }
  }
  if (index)
    *index = n;
  return MAPLE_ERROR_NO_MORE_DIR_ENTRIES;
}

int maple_get_memcard_direntry(struct maple_memcard *mcard, uint32_t num, struct maple_memcard_direntry *entry)
{
  if (!entry || !mcard || num >= mcard->dir_entries)
    return MAPLE_ERROR_BAD_ARGUMENT;
  memcpy(entry, &((struct maple_memcard_direntry *)mcard->dir_data)[num], sizeof(*entry));
#if MAPLE_HOST_BIG_ENDIAN
  entry->first_block_loc = maple_bswap16(entry->first_block_loc);
  entry->file_size = maple_bswap16(entry->file_size);
  entry->header_offset = maple_bswap16(entry->header_offset);
#endif
  return MAPLE_OK;
}

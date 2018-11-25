#include "serial.h"
#include "maple_memcard.h"
#include "maple_command.h"
#include "maple_function.h"
#include "maple_error_codes.h"
#include "maple_endian.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct vmi_header {
  char checksum[4];
  char description[32];
  char copyright[32];
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint8_t weekday;
  uint16_t vmi_version;
  uint16_t file_number;
  char vmsname[8];
  char filename[12];
  uint16_t filemode;
  uint16_t dunno;
  uint32_t filesize;
};

static const char *progname;

static uint8_t bcd2int(uint8_t bcd)
{
  return (bcd >> 4) * 10 + (bcd & 0xf);
}

static int exists_in_db(void **db, const unsigned char *name)
{
  uint32_t cnt, *header = *db;
  const unsigned char *p;
  if (!header)
    return 0;
  cnt = header[0];
  p = (void *)(header + 2);
  while (cnt--) {
    if (!memcmp(p, name, 8))
      return 1;
    p += 8;
  }
  return 0;
}

static int store_in_db(void **db, const unsigned char *name)
{
  uint32_t count, capacity, *header = *db;
  if (header) {
    count = header[0];
    capacity = header[1];
  } else {
    count = capacity = 0;
  }
  if (count >= capacity) {
    void *newblock;
    capacity += capacity >> 1;
    if (count >= capacity)
      capacity = count + 100;
    newblock = realloc(header, 2*sizeof(uint32_t) + capacity * 8);
    *db = newblock;
    if (!newblock) {
      fprintf(stderr, "Out of memory\n");
      free(header);
      return 0;
    }
    header = newblock;
    header[1] = capacity;
  }
  memcpy(((char *)(header + 2)) + 8 * count++, name, 8);
  header[0] = count;
  return 1;
}

static int generate_unique_filename(char *dest, const char *src, const char *suffix, void **db)
{
  unsigned char name[8];
  int i;

  strncpy((char *)name, src, 8);
  for (i=0; i<8; i++)
    if (name[i] == '.')
      break;
    else if (name[i] <= ' ' || name[i] == '~')
      name[i] = '_';
  while (i<8)
    name[i++] = '_';
  if (exists_in_db(db, name)) {
    for (i = 7; i > 0; --i)
      if (name[i] >= '0' && name[i] <= '9')
	name[i] = '0';
      else
	break;
    name[i] = '~';
    do {
      for (i = 7; i > 0; --i)
	if (name[i] == '9')
	  name[i] == '0';
	else if (name[i] >= '0' && name[i] < '9') {
	  name[i]++;
	  break;
	} else {
	  name[i] = '0';
	  name[i-1] = '~';
	  break;
	}
      if (!i) {
	fprintf(stderr, "Unable to generate unique name\n");
	return 0;
      }
    } while(exists_in_db(db, name));
  }
  sprintf(dest, "%.8s%s", (const char *)name, suffix);
  return store_in_db(db, name);
}

static int cmd_dump(struct maple_memcard *card, int argc, char **argv)
{
  FILE *f;
  void *buf;
  uint16_t start = 0;
  int dcm = !strcmp(argv[1], "dcmdump");

  if (argc != 3) {
    fprintf(stderr, "Usage: %s %s <filename>\n", progname, argv[1]);
    return 1;
  }

  if (!(buf = calloc(16, card->blocksz))) {
    fprintf(stderr, "Out of memory\n");
    return 1;
  }

  f = fopen(argv[2], "wb");
  if (!f) {
    perror(argv[2]);
    free(buf);
    return 1;
  }

  while (start <= card->meminfo.last_block) {
    uint16_t cnt =
      (card->meminfo.last_block - start > 15? 16 : card->meminfo.last_block - start + 1);
    int r;

    printf("%s: %d%%\r", argv[2], (int)(100L*start/(((long)card->meminfo.last_block)+1)));
    fflush(stdout);

    r = maple_read_memcard_blocks(card, start, cnt, buf);
    if (r != MAPLE_OK) {
      fprintf(stderr, "Read error: %d\n", r);
      fclose(f);
      free(buf);
      return 1;
    }

    if (dcm) {
      uint32_t *w = buf;
      uint32_t n = card->blocksz / 4 * cnt;
      while (n--) {
	*w = maple_bswap32(*w);
	w++;
      }
    }

    if (fwrite(buf, card->blocksz, cnt, f) != cnt) {
      perror(argv[2]);
      fclose(f);
      free(buf);
      return 1;
    }

    if (cnt > card->meminfo.last_block - start)
      break;
    else start += cnt;
  }

  printf("%s: 100%%\n", argv[2]);

  fclose(f);
  free(buf);
  return 0;
}

static int cmd_ls(struct maple_memcard *card, int argc, char **argv)
{
  int r, l = 0;
  const char *glob = "*";
  uint32_t index = 0;
  struct maple_memcard_direntry entry;
  if (argc > 2 && !strcmp(argv[2], "-l")) {
    l = 1;
    argv++;
    argc--;
  }
  if (argc == 3) {
    glob = argv[2];
    argv++;
    argc--;
  }
  if (argc != 2) {
    fprintf(stderr, "Usage: %s ls [-l] [<glob>]\n", progname);
    return 1;
  }
  while ((r = maple_next_memcard_direntry(card, glob, &index, &entry)) == MAPLE_OK) {
    if (l)
      printf("%c%c %5u %02x%02x-%02x-%02x %02x:%02x %.12s\n",
	     (entry.file_type == MAPLE_MEMCARD_FILE_TYPE_GAME? 'g' :
	      (entry.file_type == MAPLE_MEMCARD_FILE_TYPE_DATA? 'd' : '?')),
	     (entry.copy_protect? 'p' : '-'),
	     (unsigned)entry.file_size,
	     (unsigned)entry.creation_time.century,
	     (unsigned)entry.creation_time.year_in_century,
	     (unsigned)entry.creation_time.month,
	     (unsigned)entry.creation_time.day,
	     (unsigned)entry.creation_time.hour,
	     (unsigned)entry.creation_time.minute,
	     entry.filename);
    else
      printf("%.12s\n", entry.filename);
  }
  if (r != MAPLE_ERROR_NO_MORE_DIR_ENTRIES) {
    fprintf(stderr, "Error: %d\n", r);
    return 1;
  }
  return 0;
}

static int cmd_get(struct maple_memcard *card, int argc, char **argv)
{
  int r, rc = 0;
  uint32_t index = 0;
  struct maple_memcard_direntry entry;
  int dci = !strcmp(argv[1], "dciget");
  void *buf, *db = NULL;

  if (argc != 3) {
    fprintf(stderr, "Usage: %s %s <glob>\n", progname, argv[1]);
    return 1;
  }
  while ((r = maple_next_memcard_direntry(card, argv[2], &index, &entry)) == MAPLE_OK) {
    char fnbuf[16];
    FILE *f;
    uint16_t file_size = entry.file_size;
    printf("%-12.12s...", entry.filename);
    fflush(stdout);

    if (!(buf = calloc(file_size, card->blocksz))) {
      fprintf(stderr, "Out of memory\n");
      rc = 1;
      continue;
    }

    if ((r = maple_read_memcard_chained_blocks(card, entry.first_block_loc, file_size, buf)) != MAPLE_OK) {
      free(buf);
      break;
    }
    if (dci) {
      uint32_t *w = buf;
      uint32_t n = card->blocksz / 4 * file_size;
      while (n--) {
	*w = maple_bswap32(*w);
	w++;
      }
#if MAPLE_HOST_BIG_ENDIAN
      /* DCI stores raw direntry as LE */
      entry.first_block_loc = maple_bswap16(entry.first_block_loc);
      entry.file_size = maple_bswap16(entry.file_size);
      entry.header_offset = maple_bswap16(entry.header_offset);
#endif
    }

    if (!generate_unique_filename(fnbuf, entry.filename, (dci? ".dci" : ".vms"), &db)) {
      free(buf);
      rc = 1;
      break;
    }
    if (!(f = fopen(fnbuf, "wb"))) {
      perror(fnbuf);
      free(buf);
      rc = 1;
      continue;
    }
    printf("\b\b\b -> %s...", fnbuf);
    fflush(stdout);
    if (dci && fwrite(&entry, sizeof(entry), 1, f) != 1) {
      perror(fnbuf);
      fclose(f);
      free(buf);
      rc = 1;
      continue;
    }
    if (fwrite(buf, card->blocksz, file_size, f) != file_size) {
      perror(fnbuf);
      fclose(f);
      free(buf);
      rc = 1;
      continue;
    }
    fclose(f);
    free(buf);

    if (!dci) {
      struct vmi_header vmi;
      fnbuf[11] = 'i';
      printf("\b\b\b / %s...", fnbuf);
      fflush(stdout);
      if (!(f = fopen(fnbuf, "wb"))) {
	perror(fnbuf);
	rc = 1;
	continue;
      }
      memset(&vmi, 0, sizeof(vmi));
      memcpy(vmi.filename, entry.filename, 12);
      memcpy(vmi.vmsname, fnbuf, 8);
      memcpy(vmi.copyright, vmi.vmsname, 8);
      memcpy(vmi.description, vmi.vmsname, 8);
      vmi.year = bcd2int(entry.creation_time.century)*100 +
	bcd2int(entry.creation_time.year_in_century);
      vmi.month = bcd2int(entry.creation_time.month);
      vmi.day = bcd2int(entry.creation_time.day);
      vmi.hour = bcd2int(entry.creation_time.hour);
      vmi.minute = bcd2int(entry.creation_time.minute);
      vmi.second = bcd2int(entry.creation_time.second);
      vmi.weekday = entry.creation_time.weekday;
      vmi.vmi_version = 0;
      vmi.file_number = 1;
      vmi.filemode =
	(entry.file_type == MAPLE_MEMCARD_FILE_TYPE_GAME? 2 : 0) |
	(entry.copy_protect? 1 : 0);
      vmi.filesize = card->blocksz * file_size;
#if MAPLE_HOST_BIG_ENDIAN
      vmi.year = maple_bswap16(vmi.year);
      vmi.vmi_version = maple_bswap16(vmi.vmi_version);
      vmi.file_number = maple_bswap16(vmi.file_number);
      vmi.filemode = maple_bswap16(vmi.filemode);
      vmi.filesize = maple_bswap32(vmi.filesize);
#endif
      vmi.checksum[0] = vmi.vmsname[0] & 'S';
      vmi.checksum[1] = vmi.vmsname[1] & 'E';
      vmi.checksum[2] = vmi.vmsname[2] & 'G';
      vmi.checksum[3] = vmi.vmsname[3] & 'A';
      if (fwrite(&vmi, sizeof(vmi), 1, f) != 1) {
	perror(fnbuf);
	fclose(f);
	rc = 1;
	continue;
      }
      fclose(f);
    }

    printf("\b\b\b   \n");
  }
  free(db);
  if (r != MAPLE_ERROR_NO_MORE_DIR_ENTRIES) {
    fprintf(stderr, "Error: %d\n", r);
    return 1;
  }
  return 0;
}

static void usage(void)
{
  fprintf(stderr, "Usage: %s [-S serialport] [-D maple device] command ...\n",
	  progname);
}

static int process_command(struct maple_memcard *card, int argc, char **argv)
{
  if (!strcmp(argv[1], "dump") || !strcmp(argv[1], "dcmdump"))
    return cmd_dump(card, argc, argv);
  else if(!strcmp(argv[1], "ls"))
    return cmd_ls(card, argc, argv);
  else if (!strcmp(argv[1], "vmiget") || !strcmp(argv[1], "dciget"))
    return cmd_get(card, argc, argv);
  else {
    usage();
    fprintf(stderr, "Known commands:\n"
	    "  dump <filename>      Dump memory card contents to file\n"
	    "  dcmdump <filename>   Dump memory card contents to DCM (byteswapped) file\n"
	    "  vmiget <glob>        Read a memory card file into a VMI/VMS\n"
	    "  dciget <glob>        Read a memory card file into a DCI\n"
	    "  ls [-l] [<glob>]     List files on memory card\n");
    return 1;
  }
}

int main(int argc, char **argv)
{
  int port = 0;
  int unit = 1;
  int part = 0;
  const char *serport = "/dev/ttyACM0";
  struct maple_dev_info *info;
  uint32_t func_data;
  int r, rc = 1;

  progname = (argc > 0? argv[0] : "");
  while (argc > 1 && argv[1][0] == '-') {
    char opt = argv[1][1];
    const char *arg = argv[1]+2;
    if (!opt || (!*arg && argc < 3)) {
      usage();
      return 1;
    }
    if (!*arg) {
      arg = argv[2];
      argv += 2;
      argc -= 2;
    } else {
      argv++;
      argc--;
    }
    switch (opt) {
    case 'S':
      serport = arg;
      break;
    case 'D':
      if (arg[0] >= 'A' && arg[0] <= 'D' &&
	  arg[1] >= '0' && arg[1] <= '5' &&
	  (arg[2] == 0 || (arg[2] >= 'a' && arg[2] <= 'z' && arg[3] == 0))) {
	port = arg[0]-'A';
	unit = arg[1]-'0';
	part = arg[2]? arg[2]-'a' : 0;
      } else {
	usage();
	return 1;
      }
    }
  }
  if (argc < 2) {
    usage();
    return 1;
  }

  if (maple_serial_open(serport))
    return 1;
  r = maple_get_dev_info(maple_address(port, unit), &info);
  if (r == MAPLE_OK)
    r = maple_function_check(info, MAPLE_FUNCTION_MEMORY_CARD, &func_data);
  free(info);
  if (r == MAPLE_OK) {
    struct maple_memcard card;
    r = maple_open_memcard(maple_address(port, unit), func_data, part, &card);
    if (r == MAPLE_OK) {
      rc = process_command(&card, argc, argv);
    } else fprintf(stderr, "Failed to open partition %c of %c%d\n",
		   'a'+part, 'A'+port, unit);
    maple_close_memcard(&card);
  } else fprintf(stderr, "No memory card found at %c%d\n", 'A'+port, unit);
  return rc;
}

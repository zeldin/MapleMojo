#include <stdint.h>
#include <stddef.h>

struct maple_sg_block {
  const uint8_t *data;
  size_t len;
};

struct maple_sg_rblock {
  uint8_t *data;
  size_t len;
};

struct maple_header {
  uint8_t payload_words;
  uint8_t src_address;
  uint8_t dest_address;
  uint8_t command;
};

int maple_transaction_sg(const struct maple_header *header,
			 const struct maple_sg_block *block, unsigned nblock,
			 struct maple_header *rheader,
			 const struct maple_sg_rblock *rblock, unsigned nrblock);

int maple_transaction_0(const struct maple_header *header,
			struct maple_header *rheader, uint8_t *rbuf,
			size_t rbufsize);

int maple_transaction_1(const struct maple_header *header,
			const uint8_t *payload, size_t payload_size,
			struct maple_header *rheader, uint8_t *rbuf1,
			size_t rbuf1size, uint8_t *rbuf2, size_t rbuf2size);

int maple_transaction_2(const struct maple_header *header,
			const uint8_t *payload1, size_t payload1_size,
			const uint8_t *payload2, size_t payload2_size,
			struct maple_header *rheader,
			uint8_t *rbuf, size_t rbufsize);

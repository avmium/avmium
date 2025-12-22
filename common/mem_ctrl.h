#ifndef __mem_ctrl_h__
#define __mem_ctrl_h__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#if SVSIM
#include "svdpi.h"
#endif

enum memap_type_e {
	MEM,
	PORT
};

struct memmap {
	uint64_t addr;
	uint64_t end_addr;
	uint64_t size;
	uint8_t type;
	void *ptr;
	struct memmap *nxt;
};

int memmap_init(void *opaque, uint64_t addr, size_t size, uint8_t type, 
				struct port *port);
void memmap_deinit(void *opaque);
int set_memmap(void *opaque, uint64_t addr, uint8_t *data, uint16_t size,
			   uint8_t rw, uint8_t check_port);
void write_memap(void *opaque, uint64_t addr, uint8_t *data, uint16_t size);
void read_memap(void *opaque, uint64_t addr, uint8_t *data, uint16_t size);
uint8_t* get_memmap(void *opaque, uint64_t addr);
#if SVSIM
int sv_set_memmap(void *opaque, uint64_t addr, const svOpenArrayHandle data,
				  uint16_t size, uint8_t rw, uint8_t check_port);
#endif

#ifdef __cplusplus
}
#endif

#endif // __mem_ctrl_h__

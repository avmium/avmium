#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "common.h"
#include "mem_ctrl.h"

int memmap_init(void *opaque, uint64_t addr, size_t size, uint8_t type, 
				struct port *port)
{
	struct blk *blk = (struct blk *) opaque;
	struct memmap *blkm = (struct memmap *) blk->memmap;
	struct memmap *m = (struct memmap *) calloc(1, sizeof(struct memmap));

	if(m == NULL)
		return -ENOMEM;

	m->addr = addr;
	m->end_addr = addr + size;
	m->size = size;
	m->type = type;
	if(type == MEM) {
		m->ptr = calloc(1, size);
		if(m->ptr == NULL) {
			free(m);
			return -ENOMEM;
		}
	} else {
		m->ptr = port;
		port->mmap_addr = addr;
	}

	if(blk->memmap == NULL) {
		blk->memmap = m;
	} else {
		while(blkm->nxt != NULL) {
			blkm = blkm->nxt;
		}
		blkm->nxt = m;
	}
	return 0;
}

void memmap_deinit(void *opaque)
{
	struct blk *blk = (struct blk *) opaque;
	struct memmap *m = (struct memmap *) blk->memmap;
	struct memmap *m_nxt = NULL;

	while(m != NULL) {
		m_nxt = m->nxt;
		if(m->type == MEM) free(m);
		m = m_nxt;
	}
}

#if 0
int sv_set_memmap(void *opaque, uint64_t addr, const svOpenArrayHandle data,
				  uint16_t size, uint8_t rw, uint8_t check_port)
{
	uint8_t* data_ptr = (uint8_t*) svGetArrayPtr(data);
	return set_memmap(opaque, addr, data_ptr, size, rw, check_port);
}
#endif

int set_memmap(void *opaque, uint64_t addr, uint8_t *data, uint16_t size,
			   uint8_t rw, uint8_t check_port)
{
	int ret = 0, tmp__idx = 0, tmp__size = 0, data__idx = 0, copy__size = 0;
	struct port *p = (struct port *) opaque;
	struct blk *blk = p->blk;;
	struct memmap *m = (struct memmap *) blk->memmap;

	while(size) {
		if(m == NULL)
			break;

		if(m->addr <= addr && addr < (m->addr + m->size)) {
			tmp__idx = addr - m->addr;
			tmp__size = m->size - tmp__idx;
			if(size <= tmp__size)
				copy__size = size;
			else
				copy__size = tmp__size;
			size -= copy__size;
			if(m->type == MEM) {
				if(rw) {
					memcpy(m->ptr + tmp__idx, data + data__idx, copy__size);
				} else {
					memcpy(data + data__idx, m->ptr + tmp__idx, copy__size);
				}
				ret = 1;
			} else if(check_port && m->type == PORT) {
				port_add_request(m->ptr, NULL, addr, data + data__idx,
							tmp__size, rw);	
				ret = 1;
			}
			addr += tmp__size;
			data__idx += tmp__size;
		}
		m = m->nxt;
	}

	return ret;
}

void write_memap(void *opaque, uint64_t addr, uint8_t *data, uint16_t size)
{
	set_memmap(opaque, addr, data, size, 1, 1);
}

void read_memap(void *opaque, uint64_t addr, uint8_t *data, uint16_t size)
{
	set_memmap(opaque, addr, data, size, 0, 1);
}

uint8_t* get_memmap(void *opaque, uint64_t addr)
{
	struct port *p = (struct port *) opaque;
	struct blk *blk = p->blk;
	struct memmap *mmap = (struct memmap *) blk->memmap;

	while(mmap != NULL) {
		if(addr == mmap->addr) {
			return mmap->ptr;
		}
		mmap = mmap->nxt;
	}
	return NULL;
}

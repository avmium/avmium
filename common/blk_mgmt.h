#ifndef __blk_mgmt_h__
#define __blk_mgmt_h__

#ifdef __cplusplus
extern "C" {
#endif

struct blk {
	char *name;
	struct port *port, *port_tail;
	void *memmap;
	void *prv_data;
};

struct blk* blk_init(const char *name);
void blk_deinit(struct blk *);

#ifdef __cplusplus
}
#endif

#endif // __blk_mgmt_h__

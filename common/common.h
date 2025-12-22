#ifndef __common_h__
#define __common_h__

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include "que_mgmt.h"
#include "blk_mgmt.h"
#include "port_mgmt.h"
#include "mem_ctrl.h"
#include "route.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TRAN_Q_SIZE 10

struct __attribute__((packed)) breq {
    uint8_t  rw:1,
			 end:1,
			 status:6;
	uint16_t size;
    uint8_t  *data;
	uint8_t  *be;
	uint64_t addr;
	void 	 *ex_signals;
};

struct data_handler {
	uint64_t start_addr;
	uint64_t end_addr;
	void 	 *mem;
	int (*request_acceptor) (void *obj_to, struct breq *req, uint16_t *ridx);
	struct data_handler *nxt;
};

struct port;
struct port_action;

struct bobj {
	struct bobj *parent;
	struct port *ports, *ports_tail;
	struct port_action *action, *action_tail;
	uint8_t proto;
	uint8_t type;
	void *mem;
    struct breq **req;
    uint16_t pidx, cidx;
	uint16_t out_trans;
	void *sigs_width;
	int (*request_acceptor)  (void *obj_to, struct breq *req, uint16_t *ridx);
	void (*clk_handler)(void *mem);
	struct data_handler *dhl, *dhll;
	struct que *que;
};

void dprint(char *mname, char *fname, unsigned int line, char *format, ...);
int default_init(struct bobj **obj, struct bobj *p_obj, int q_size, char *name);
int register_port_action(struct bobj *obj, uint8_t type, uint64_t start, uint64_t end, void (*callback)(struct bobj *obj));
int default_memory_init(struct bobj *obj, size_t size);
int bind_data_handler(struct bobj *src, struct bobj *dest,
					  int (*request_acceptor)(void *obj, struct breq *req,
					  uint16_t *ridx), uint64_t start_addr, uint64_t end_addr);
int bind_request_acceptor(struct bobj *src, struct bobj *dest,
	int (*request_acceptor) (void *obj, struct breq *req, uint16_t *ridx),
	uint64_t start_addr, uint64_t end_addr);
int check_queue_space(struct bobj *obj);
struct breq* alloc_req_mem(uint16_t size);
#if 0
int add_request(struct bobj *obj, uint8_t rw, uint64_t addr, uint8_t *data,
				uint16_t size, uint8_t *be, uint16_t *ridx);
#endif
int submit_request(void *to, struct breq *req, uint16_t *ridx);
void create_tran_que(struct bobj *obj);
void destroy_tran_que(struct bobj *obj);
void default_memory_destroy(uint8_t *mem);
void default_destroy(struct bobj *obj);

#ifdef __cplusplus
}
#endif

#endif // __common_h__

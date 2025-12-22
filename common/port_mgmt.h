#ifndef __port_mgmt_h__
#define __port_mgmt_h__

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "blk_mgmt.h"
#include "que_mgmt.h"

enum port_type_e {
	MASTER,
	SLAVE
};

#if 0
enum port_proto_e {
	APB,
	AHB,
	AXIS
};

#endif

enum port_action_e {
	FORWARD,
	DISCARD,
	FUNC
};

struct port_action {
	uint8_t type;
	uint64_t start;
	uint64_t end;
	void (*callback)(void *);
	struct port_action *nxt;
};

struct port_ctx {
	struct que_ele *send_ele;
	struct data *curr_d, *nxt_d;
	uint8_t error;
};

struct port {
	char *name;
	uint8_t type;
	uint8_t active;
	uint8_t frwd_as_rcvd;
	uint8_t addr_normalize;
	uint64_t mmap_addr;
	struct blk *blk;
	struct que_pair qp;
	void *prv_data;
	struct port_action *action, *action_tail;
	struct port *nxt;
	struct port_ctx ctx;
	void (*user_callback)(void *opaque);
	void (*resp_callback)(void *opaque);
	struct resp_data respd;
};

struct port* port_init(struct blk *blk, const char *name, uint16_t qsize,
						uint8_t type, uint8_t frwd_as_rcvd,
						uint8_t addr_normalize,
						void (*user_callback)(void *opaque),
						void (*resp_callback)(void *opaque));
void port_deinit(struct port *p);
struct port* get_port(void *opaque, const char *name);
int port_register_action(struct port *port, uint8_t type, uint64_t start, uint64_t end, void (*callback)(void *opaque));
char* get_port_name(void *opaque);
int is_que_empty(void *opaque);
int port_add_request(void *opaque, uint16_t *trid, uint64_t addr, uint8_t *usr, size_t size, uint8_t rw);
int port_status(void *opaque);
void port_active(void *opaque);
void port_disable(void *opaque);
uint8_t port_resp_get_rw(struct port *p);
uint8_t port_resp_get_status(struct port *p);
uint16_t port_resp_get_size(struct port *p);
uint16_t port_resp_get_trid(struct port *p);
uint8_t* port_resp_get_data(struct port *p);
uint64_t port_resp_get_addr(struct port *p);
uint64_t port_ctx_get_addr(struct port *p);
uint8_t* port_ctx_get_data(struct port *p);
uint16_t port_ctx_get_size(struct port *p);
uint8_t port_ctx_get_status(struct port *p);
uint8_t port_ctx_get_rw(struct port *p);

#ifdef __cplusplus
}
#endif

#endif // __port_mgmt_h__

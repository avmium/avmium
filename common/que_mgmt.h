#ifndef __que_mgmt_h__
#define __que_mgmt_h__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#if SVSIM
#include "svdpi.h"
#endif

#define PRE_ALLOCATED_QUE_SUB_ELE	500
#define PRE_ALLOCATED_DATA			500

enum que_ele_resp_action_e {
	que_ele_resp__port,
	que_ele_resp__func,
};

enum que_ele_status_e {
	FREE,
	RECEIVING,
	SENDING,
	PENDING,
	DONE
};

enum que_type_e {
	SEND,
	RESPONSE
};

struct data {
	uint64_t addr;
	uint8_t  rw;
	uint8_t status;
	uint16_t didx;
	uint16_t size;
	uint16_t chunk_size;
	uint8_t *ptr;
	struct data *nxt, *prev;
};

struct resp_data {
	uint8_t rw;
	uint8_t  status;
	uint16_t size;
	uint16_t trid;
	uint64_t addr;
	uint8_t *data;
};

struct que_ele_resp_action {
	uint8_t  type;
	uint16_t resp_idx;
	void 	 *port;
};

struct src_info {
	void *port;
	uint16_t idx;
	struct src_info *nxt;
};

struct que_ele {
	struct data *d;
	uint8_t  status;
	uint8_t  sending;
	struct que_ele_resp_action resp_act;
	struct src_info src_info;
};

struct que {
	struct que_ele *ele;
	uint16_t pidx, cidx;
};

struct que_pair {
	struct que *send,
			   *resp;
	size_t 	 size;
	uint8_t is_empty;
};

struct que_mgmt {
	struct data *data_4,
				*data_8,
				*data_128,
				*data_256,
				*data_512,
				*data_1024,
				*data_2048;

};

struct data * alloc_data(uint16_t size);
void free_data(struct data *d);
void mv_nxt_send_que_ele(struct que_pair *qp);
int create_qp(struct que_pair *qp, size_t size, int type);
void destroy_qp(struct que_pair *qp);
int reserve_data(struct data **d, size_t size);
struct que_ele* rsrv_send_que_ele(void *opaque, uint16_t *trid,
								  uint8_t resp_act__type);
struct que_ele* get_send_que_ele_check_resp(void *opaque, uint16_t *trid);
#if 0
int sv_get_send_data(void *opaque, uint16_t *trid, uint64_t *addr, uint8_t *rw,
					 const svOpenArrayHandle data, uint16_t *size);
#endif
int get_send_data(void *opaque, uint16_t *trid, uint64_t *addr,
					uint8_t *rw, uint8_t *data, uint16_t *size);
void copy_data(struct data *dest, struct data *src, size_t size);
void append_data(struct data **dest, struct data *src);

#ifdef __cplusplus
}
#endif

#endif // __que_mgmt_h__

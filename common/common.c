#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "common.h"

int default_init(struct bobj **obj, struct bobj *p_obj, int q_size, char *name)
{
	*obj = (struct bobj *) calloc(1, sizeof(struct bobj));
	if(*obj == NULL) {
		printf("Error: failed to allocate object memory\n");
		return -ENOMEM;
	}

	(*obj)->request_acceptor = submit_request;
	(*obj)->parent = p_obj;

	if(q_size >= 0) {
    	(*obj)->out_trans = q_size ? q_size : TRAN_Q_SIZE;
    	create_tran_que(*obj);
	}
	printf("%s:%u: q_size: %d; out_trans: %d\n", __FUNCTION__, __LINE__, q_size,
			(*obj)->out_trans);

	// if (register_port(*obj, name) != 0) {
	// 	return -1;
	// }

	return 0;
}

int default_memory_init(struct bobj *obj, size_t size)
{
	obj->mem = malloc(size);
	if(obj->mem == NULL) {
		printf("Error: failed to allocate object memory\n");
		return -ENOMEM;
	}
	memset(obj->mem, 0, size);

	return 0;
}

/**
 * @brief This function is used to handle default memory operation
 * @priv  Pointer to memory
 * @req   Struct to request
 */
int default_memory_handler(void *vmem, struct breq *req, uint16_t *ridx)
{
    int i;
	uint16_t be_offset;
	uint8_t *data = req->data;
	uint8_t *mem = (uint8_t *) vmem;

    if(req->rw) {
    	for(i = 0; i < req->size; i++) {
			be_offset = i / 8;
			if(*(req->be + be_offset) & (1 << (i % 8))) {
        		*(mem + req->addr + i) = *(data + i);
			}
    	}
    } else {
		for(i = 0; i < req->size; i++) {
        	*(data + i) = *(mem + req->addr + i);
		}
	}

    return req->size;
}

int bind_data_handler(struct bobj *src, struct bobj *dest,
					  int (*request_acceptor)(void *obj, struct breq *req,
					  uint16_t *ridx), uint64_t start_addr, uint64_t end_addr)
{
	struct data_handler *dhl = (struct data_handler *)
			malloc(sizeof(struct data_handler));
	if(dhl == NULL) {
		printf("Error: failed to register data handler");
		return -1;
	}

	dhl->start_addr = start_addr;
	dhl->end_addr = end_addr;
	dhl->nxt = NULL;

	if(dest) {
		dhl->mem = dest;
		dhl->request_acceptor = dest->request_acceptor;
	} else if(request_acceptor) {
		dhl->mem = src->mem;
		dhl->request_acceptor = request_acceptor;
	} else {
		dhl->mem = malloc(end_addr - start_addr + 1);
		if(dhl->mem == NULL) {
			printf("Error: failed to register data handler");
			return -1;
		}
		memset(dhl->mem, 0, end_addr - start_addr + 1);
		dhl->request_acceptor = default_memory_handler;
	}

	if(src->dhl == NULL) {
		src->dhl = dhl;
		src->dhll = dhl;
	} else {
		src->dhll->nxt = dhl;
		src->dhll = dhl;
	}

	return 0;
}

int bind_request_acceptor(struct bobj *src, struct bobj *dest,
	int (*request_acceptor) (void *obj, struct breq *req, uint16_t *ridx),
	uint64_t start_addr, uint64_t end_addr)
{
	if(dest) {
		return bind_data_handler(src, dest, NULL, start_addr, end_addr);
	} if(request_acceptor) {
		return bind_data_handler(src, NULL, request_acceptor,
									start_addr, end_addr);
	} else {
		return bind_data_handler(src, NULL, NULL, start_addr, end_addr);
	}
}

int check_queue_space(struct bobj *obj)
{
	if ((obj->pidx + 1 == obj->cidx) ||
        ((obj->pidx == obj->out_trans - 1) && (obj->cidx == 0))) {
		return -ENOMEM;
	}

	return 0;
}

struct breq* alloc_req_mem(uint16_t size)
{
	struct breq *req;
	uint16_t num_bits_to_bytes;

	req = (struct breq *) malloc(sizeof(struct breq));
	if(req == NULL) {
		printf("Error: failed to allocate memory\n");
		goto req_error;
	}
	memset(req, 0, sizeof(struct breq));

	req->data = (uint8_t *) malloc(size);
	if(req->data == NULL) {
		printf("Error: failed to allocate memory\n");
		goto data_error;
	}
	memset(req->data, 0, size);

	num_bits_to_bytes = (size / 8) + !!(size % 8);
	req->be = (uint8_t *) malloc(num_bits_to_bytes);
	if(req->be == NULL) {
		printf("Error: failed to allocate memory\n");
		goto be_error;
	}
	memset(req->be, 0, num_bits_to_bytes);

	req->size = size;
	return req;

be_error:
	free(req->data);

data_error:
	free(req);

req_error:
	return NULL;
}

#if 0
int add_request(struct bobj *obj, uint8_t rw, uint64_t addr, uint8_t *data,
				uint16_t size, uint8_t *be, uint16_t *ridx)
{
	uint16_t num_bits_to_bytes;
	struct breq req = {0};

	if(check_queue_space(obj) < 0) {
		printf("Error: request queue full\n");
		return -ENOMEM;
	}

	req.data = (uint8_t *) malloc(size);
	if(req.data == NULL) {
		printf("Error: failed to allocate memory\n");
		return -ENOMEM;
	}

	if(data != NULL)
		memcpy(req.data, data, size);

	if(be != NULL) {
		num_bits_to_bytes = (size / 8) + !!(size % 8);
		req.be = (uint8_t *) malloc(num_bits_to_bytes);
		if(req.be == NULL) {
			printf("Error: failed to allocate memory\n");
			return -ENOMEM;
		}
		memcpy(req.be, be, num_bits_to_bytes);
	}
		
    req.rw = rw;
    req.addr = addr;
    req.size = size;
    return submit_request(obj, &req, ridx);
}
#endif

int submit_request(void *vobj, struct breq *req, uint16_t *ridx)
{
	struct bobj *obj = (struct bobj *) vobj;
	uint16_t idx = obj->pidx;
	struct breq *reqe = obj->req[idx];

	if(check_queue_space(vobj) < 0) {
		printf("Error: request queue full\n");
		return -ENOMEM;
	}

    reqe->rw = req->rw;
    reqe->addr = req->addr;
    reqe->data = req->data;
    reqe->be = req->be;
    reqe->size = req->size;
	req->status = PENDING;
	obj->pidx = (obj->pidx + 1) % obj->out_trans;
	*ridx = idx;
    return req->rw ? req->size : 0;
}

void create_tran_que(struct bobj *obj)
{
    struct breq **req;
    obj->req = (struct breq **) calloc(obj->out_trans, sizeof(struct breq *));
    if(obj->req == NULL) {
        printf("Error: failed to allocate apb request queue");
        exit(-1);
    }

    for(int i = 0; i < obj->out_trans; i++) {
        struct breq *req = (struct breq *) malloc(sizeof(struct breq));
        if(req == NULL) {
            printf("Error: failed to allocate apb request queue elements");
            exit(-1);
        }
        memset(req, 0, sizeof(struct breq));
        obj->req[i] = req;
    }
}

void destroy_tran_que(struct bobj *obj)
{
    for(int i = 0; i < obj->out_trans; i++) {
        free(obj->req[i]);
    }

    free(obj->req);
}

void default_memory_destroy(uint8_t *priv)
{
	free(priv);
}

void default_destroy(struct bobj *obj)
{
	free(obj);
}

int slave_active(struct bobj *obj)
{
	return obj->parent->dhl ? 0 : -1;
}

// int register_port(struct bobj *obj, char *name)
// {
// 	struct bobj *blk = obj->parent;
// 	struct port *p = (struct port *) calloc(1, sizeof(struct port));
// 	if(p == NULL) {
// 		return 1;
// 	}
// 	p->name = name;
// 	p->obj = obj;
// 	if(blk->ports == NULL) {
// 		blk->ports = p;
// 	} else {
// 		blk->ports_tail->nxt = p;
// 	}
// 	blk->ports_tail = p;
// 	return 0;
// }
// 
// int register_port_action(struct bobj *obj, uint8_t type, uint64_t start, uint64_t end, void (*callback)(struct bobj *obj))
// {
// 	struct port_action *act = (struct port_action *) calloc(1, sizeof(struct port_action)); 
// 	if(act == NULL)
// 		return -ENOMEM;
// 
// 	act->type = type;
// 	act->start = start;
// 	act->end = end;
// 	act->callback = callback;
// 
// 	if(obj->action == NULL)
// 		obj->action = act;
// 	else
// 		obj->action_tail->nxt = act;
// 	obj->action_tail = act;
// 		
// 	return 0;
// }

void* memdup(const void* src, size_t size) {
    if (!src || size == 0) return NULL;

    void* dest = malloc(size);
    if (!dest) return NULL;

    memcpy(dest, src, size);
    return dest;
}

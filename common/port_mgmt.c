#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "que_mgmt.h"
#include "port_mgmt.h"

struct port* port_init(struct blk *blk, const char *name, uint16_t qsize,
						uint8_t type, uint8_t frwd_as_rcvd,
						uint8_t addr_normalize,
						void (*user_callback)(void *opaque),
						void (*resp_callback)(void *opaque))
{
	struct port *p = (struct port *) calloc(1, sizeof(struct port));
	if(p == NULL)
		goto err_port;

	p->name = strdup(name);
	p->blk = blk;
	p->active = 1;
	p->type = type;
	p->frwd_as_rcvd = frwd_as_rcvd;
	p->addr_normalize = addr_normalize;
	p->user_callback = user_callback;
	p->resp_callback = resp_callback;
	create_qp(&(p->qp), qsize, RESPONSE);

	if(blk->port == NULL) {
		blk->port = p;
	} else {
		blk->port_tail->nxt = p;
	}
	blk->port_tail = p;
	return p;

err_port:
	return NULL;
}

void port_deinit(struct port *p)
{
	destroy_qp(&(p->qp));
	free(p);
}

struct port* get_port(void *opaque, const char *name)
{
	struct port *p = (struct port *) opaque;
	struct blk *blk = p->blk;
	p = blk->port;
	while(p != NULL) {
		if(strcmp(p->name, name) == 0)
			break;
		else
			p = p->nxt;
	}

	return p;
}

int port_register_action(struct port *port, uint8_t type, uint64_t start, uint64_t end, void (*callback)(void *opaque))
{
	struct port_action *act = (struct port_action *) calloc(1, sizeof(struct port_action)); 
	if(act == NULL)
		return -ENOMEM;

	act->type = type;
	act->start = start;
	act->end = end;
	act->callback = callback;

	if(port->action == NULL)
		port->action = act;
	else
		port->action_tail->nxt = act;
	port->action_tail = act;
		
	return 0;
}


char* get_port_name(void *opaque)
{
	struct port *p = (struct port *) opaque;
	return p->name;
}

int is_que_empty(void *opaque)
{
	struct port *port = (struct port *) opaque;
	return port->qp.is_empty;
}

int port_add_request(void *opaque, uint16_t *trid, uint64_t addr, uint8_t *usr,
					 size_t size, uint8_t rw)
{
	struct port *port = (struct port *) opaque;
	struct que_ele *ele = NULL;
	struct data *d = NULL, *tmp_d = NULL;

	if(!(port->qp.is_empty))
		return -ENOMEM;

	ele = rsrv_send_que_ele(port, trid, que_ele_resp__func);
	if(ele == NULL)
		return -ENOMEM;

	ele->status = RECEIVING;
	while(size > 0) {
		d = alloc_data(size);
		d->addr = addr;
		d->rw = rw;
		d->didx = 0;
		d->nxt = NULL;
		if(size > d->chunk_size) {
			d->size = d->chunk_size;
			size -= d->chunk_size;
		} else {
			d->size = size;
			size -= size;
		}
		if(rw) memcpy(d->ptr, usr, d->size);
		addr += d->size;
		if(ele->d == NULL) {
			ele->d = d;
		} else {
			tmp_d = ele->d;
			while(tmp_d->nxt != NULL) {
				tmp_d = tmp_d->nxt;
			}
			tmp_d->nxt = d;
		}
	}
	ele->status = DONE;
	mv_nxt_send_que_ele(&port->qp);

	return 0;
}

#if 0
int port_add_request(void *opaque, uint16_t *trid, uint64_t addr, uint8_t *usr, size_t size, uint8_t rw, uint8_t usr_call)
{
	struct port *port = (struct port *) opaque;
	struct que_ele *ele = NULL;
	struct data *d = NULL, *tmp_d = NULL;

	if(usr_call && !(port->qp.is_empty))
		return -ENOMEM;

	ele = rsrv_send_que_ele(port, trid);
	if(ele == NULL)
		return -ENOMEM;

	while(size > 0) {
		d = alloc_data(size);
		d->addr = addr;
		d->rw = rw;
		d->didx = 0;
		d->nxt = NULL;
		if(size > d->chunk_size) {
			d->size = d->chunk_size;
			size -= d->chunk_size;
		} else {
			d->size = size;
			size -= size;
		}
		memcpy(d->ptr, usr, d->size);
		addr += d->size;
		if(ele->d == NULL) {
			ele->d = d;
		} else {
			tmp_d = ele->d;
			while(tmp_d->nxt != NULL) {
				tmp_d = tmp_d->nxt;
			}
			tmp_d->nxt = d;
		}
	}
	if(usr_call) mv_nxt_send_que_ele(&port->qp);

	return 0;
}
#endif

int port_status(void *opaque)
{
	struct port *port = (struct port *) opaque;
	return port->active;
}

void port_active(void *opaque)
{
	struct port *port = (struct port *) opaque;
	port->active = 1;
}

void port_disable(void *opaque)
{
	struct port *port = (struct port *) opaque;
	port->active = 0;
}

uint8_t port_resp_get_rw(struct port *p)
{
	return p->respd.rw;
}

uint8_t port_ctx_get_rw(struct port *p)
{
	return p->ctx.curr_d->rw;
}

uint8_t port_resp_get_status(struct port *p)
{
	return p->respd.status;
}

uint8_t port_ctx_get_status(struct port *p)
{
	return p->ctx.curr_d->status;
}

uint16_t port_resp_get_size(struct port *p)
{
	return p->respd.size;
}

uint16_t port_ctx_get_size(struct port *p)
{
	return p->ctx.curr_d->size;
}

uint16_t port_resp_get_trid(struct port *p)
{
	return p->respd.trid;
}

uint8_t* port_resp_get_data(struct port *p)
{
	return p->respd.data;
}

uint8_t* port_ctx_get_data(struct port *p)
{
	return p->ctx.curr_d->ptr;
}

uint64_t port_resp_get_addr(struct port *p)
{
	return p->respd.addr;
}

uint64_t port_ctx_get_addr(struct port *p)
{
	return p->ctx.curr_d->addr;
}

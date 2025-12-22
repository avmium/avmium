#include <string.h>
#include "common.h"
#include "apb.h"

uint64_t apb_get_addr(struct port *port, uint64_t addr)
{
	return (port->addr_normalize) ? addr - port->mmap_addr : addr;
}

#if SVSIM
void sv_apb_process_pready(struct port *port, uint64_t addr,
		const svOpenArrayHandle data, uint16_t size, uint8_t rw,
		uint8_t slverr)
{
	uint8_t* data_ptr = (uint8_t*) svGetArrayPtr(data);
	apb_process_pready(port, addr, data_ptr, size, rw, slverr);
	return;
}
#endif

void apb_process_pready(struct port *port, uint64_t addr, uint8_t *data,
		uint16_t size, uint8_t rw, uint8_t slverr)
{
	struct que_pair *qp = NULL;
	struct que_ele *send_ele = NULL, *resp_ele = NULL;
	struct data *d = NULL, *tmp__d = NULL;

	qp = &(port->qp);
	send_ele = &(qp->send->ele[	qp->send->cidx ]);
	resp_ele = &(qp->resp->ele[	qp->send->cidx ]);
	resp_ele->status = RECEIVING;
	d = alloc_data(size);
	d->addr = (rw) ? 0 : addr;
	d->size = (rw) ? 0 : size;
	d->rw = (rw) ? 0 : rw;
	d->status = slverr;
	d->didx = 0;
	if(!rw) {
		memcpy(d->ptr, data, size);
	}
	if(resp_ele->d == NULL) {
		resp_ele->d = d;
	} else {
		tmp__d = resp_ele->d;
		while(tmp__d->nxt != NULL) tmp__d = tmp__d->nxt;
		tmp__d->nxt = d;
	}
	if(!send_ele->sending) {
		resp_ele->status = DONE;
	}
}

#if SVSIM
void sv_apb_process_slave_pready(struct port *port, const svOpenArrayHandle data,
	uint16_t size, uint8_t *wait_for_response, uint8_t *pready)
{
	uint8_t* data_ptr = (uint8_t*) svGetArrayPtr(data);
	apb_process_slave_pready(port, data_ptr, size, wait_for_response, pready);
	return;
}
#endif

uint8_t apb_process_slave_pready(struct port *port, uint8_t *data, uint16_t size,
		uint8_t *wait_for_response, uint8_t *pready)
{
	struct que_pair *qp = NULL;
	struct que_ele *send_ele = NULL, *resp_ele = NULL;
	struct data *d = NULL, *tmp__d = NULL;

	qp = &( port->qp );
	for(int loop = 0; loop < qp->size; loop++) {
		resp_ele = &( qp->resp->ele[ loop ] );
		if((port->frwd_as_rcvd && resp_ele->status == RECEIVING) ||
				resp_ele->status == DONE) { 
			*wait_for_response = 0;
			*pready = 0;
			d = resp_ele->d;
			if(d != NULL) {
				resp_ele->d = d->nxt;
				if(!d->rw)	{// correct with variable size data
					memcpy(data, d->ptr, size);
				}
				free_data(d);
			}
			resp_ele->status = FREE;
		}
	}
}

#if SVSIM
void sv_apb_slave_process_access(struct port *port, uint8_t *state, uint8_t pwrite,
		uint64_t addr, const svOpenArrayHandle data, uint16_t size, uint8_t *pready,
		uint8_t *pslverr, uint8_t *wait_for_response)
{
	uint8_t* data_ptr = (uint8_t*) svGetArrayPtr(data);
	apb_slave_process_access(port, state, pwrite, addr, data_ptr, size, pready,
		pslverr, wait_for_response);
	return;
}
#endif

void apb_slave_process_access(struct port *port, uint8_t *state, uint8_t pwrite,
		uint64_t addr, uint8_t *data, uint16_t size, uint8_t *pready,
		uint8_t *pslverr, uint8_t *wait_for_response)
{
	struct que_pair *qp = NULL;
	struct que_ele *send_ele = NULL, *resp_ele = NULL;
	struct data *d = NULL, *tmp__d = NULL;
	uint8_t rw;

	*state = 0;
	rw = pwrite;
	d = alloc_data(size);
	if(rw) memcpy(d->ptr, data, size);
	d->size = size;
	d->addr = addr;
	d->rw = rw;
	d->didx = 0;
	d->status = 0;
	port->ctx.send_ele = NULL;
	port->ctx.curr_d = d;
	port->ctx.nxt_d = NULL;
	port->user_callback(port);
	if(set_memmap(port, d->addr, data, size, rw, 0)) {
		*pslverr = 0;
		*pready = 1;
		*wait_for_response = 0;
		free_data(d);	d = NULL;
	} else if(port->qp.is_empty) {
		send_ele = rsrv_send_que_ele(port, NULL, que_ele_resp__port);
		send_ele->status = RECEIVING;
		if(send_ele->d == NULL) {
			send_ele->d = d;
		} else {
			tmp__d = send_ele->d;
			while(tmp__d->nxt != NULL) tmp__d = tmp__d->nxt;
			tmp__d->nxt = d;
		}
		send_ele->status = DONE;
		mv_nxt_send_que_ele(&port->qp);
		*pslverr = 0;
		*pready = 0;
		*wait_for_response = 1;
	} else {
		*pslverr = 1;
		*pready = 1;
	}

}

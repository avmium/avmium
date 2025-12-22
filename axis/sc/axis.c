#include <string.h>
#include "common.h"
#include "port_mgmt.h"
#include "que_mgmt.h"
#include "axis.h"

void axis_process_resp(void *opaque, uint16_t trid)
{
	struct port *port = (struct port *) opaque;
	struct que_pair *qp = NULL;
	struct que_ele *send_ele = NULL, *resp_ele = NULL;

	qp = &(port->qp);
	resp_ele = &(qp->resp->ele[	trid ]);
	resp_ele->status = DONE;
}

#if SVSIM
int sv_axis_process_tvalid(void *opaque, uint64_t addr,
		const svOpenArrayHandle data, uint16_t size, uint8_t tlast)
{
	uint8_t* data_ptr = (uint8_t*) svGetArrayPtr(data);
	return axis_process_tvalid(opaque, addr, data_ptr, size, tlast);
}
#endif

int axis_process_tvalid(void *opaque, uint64_t addr, uint8_t *data, uint16_t size, uint8_t tlast)
{
	struct port *port = (struct port *) opaque;
	struct que_ele *send_ele = NULL, *resp_ele = NULL;
	int tready = 0;
	struct data *d = NULL, *tmp__d = NULL;
	d = alloc_data(size);
	memcpy(d->ptr, data, size);
	d->size = size;
	d->addr = addr;
	d->size = size;
	d->addr = addr;
	d->rw = 1;
	d->didx = 0;
	d->status = 0;
	port->ctx.send_ele = rsrv_send_que_ele(port, NULL, que_ele_resp__port);
	port->ctx.curr_d = d;
	port->ctx.nxt_d = port->ctx.send_ele->d;
	port->user_callback(port);
	if(set_memmap(port, addr, data, size, 1, 0)) {
		tready = 1;
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
		if(tlast == 1) {
			send_ele->status = DONE;
		}
		tready = 1;
	}
	return tready;
}

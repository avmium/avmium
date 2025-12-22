#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "port_mgmt.h"
#include "que_mgmt.h"

struct que_mgmt g_qm = {0};

#if SVSIM
int sv_get_send_data(void *opaque, uint16_t *trid, uint64_t *addr, uint8_t *rw,
					 const svOpenArrayHandle data, uint16_t *size)
{
	uint8_t* data_ptr = (uint8_t*) svGetArrayPtr(data);
	return get_send_data(opaque, trid, addr, rw, data_ptr, size);
}
#endif

int get_send_data(void *opaque, uint16_t *trid, uint64_t *addr,
					uint8_t *rw, uint8_t *data, uint16_t *size)
{
	int ret = 0;
	struct port *p = (struct port *) opaque;
	struct que_pair *qp = &(p->qp);
	int temp__idx = 0, transaction_end = 0;
	struct data *d = NULL;
	int requested_size = *size, return_size = 0;
	struct que_ele *ele = get_send_que_ele_check_resp(p, trid);

	if(ele == NULL || ele->d == NULL)
		return ret;

	d = ele->d;
	*addr = d->addr + d->didx;
	*rw = d->rw;

	while(return_size != *size) {
		if(ele->d == NULL) break;
		ret = 1;
		d = ele->d;
		if(d->size <= requested_size) {
			return_size += d->size;
			requested_size -= d->size;
			memcpy(data + temp__idx , d->ptr + d->didx, d->size);
			temp__idx += d->size;
			ele->d = d->nxt;
			if(ele->d != NULL) {
				if((ele->d->addr != (d->addr + d->didx + d->size)) ||
						(ele->d->rw != d->rw)) {
					transaction_end = 1;
					free_data(d);
					break;
				}
			}
			free_data(d);
		} else {
			return_size += requested_size;
			memcpy(data + temp__idx, d->ptr + d->didx, requested_size);
			d->didx += requested_size;
			d->size -= requested_size;
		}
	}

	if(transaction_end) {
		ret = 2;
		ele->status = DONE;
		ele->sending = 0;
		qp->send->cidx = (qp->send->cidx + 1) % qp->size;
	} else if(ele->d == NULL && ele->status == DONE) {
		ret = 2;
		ele->status = FREE;
		ele->sending = 0;
		qp->is_empty = 1;
		qp->send->cidx = (qp->send->cidx + 1) % qp->size;
	}

	*size = return_size;
	return ret;
}

struct data * alloc_data(uint16_t size)
{
	struct que_mgmt *qm = &g_qm;
	struct data **d = NULL;
	struct data *ret_d = NULL;
	uint16_t max_size;

	if(size <= 4) {
		d = &qm->data_4;
		max_size = 4;
	} else if(size <= 8) {
		d = &qm->data_8;
		max_size = 8;
	} else if(size <= 128) {
		d = &qm->data_128;
		max_size = 128;
	} else if(size <= 256) {
		d = &qm->data_256;
		max_size = 256;
	} else if(size <= 512) {
		d = &qm->data_512;
		max_size = 512;
	} else if(size <= 1024) {
		d = &qm->data_1024;
		max_size = 1024;
	} else {
		d = &qm->data_2048;
		max_size = 2048;
	}

	if(*d == NULL && reserve_data(d, max_size) < 0) {
		return NULL;
	}

	ret_d = *d;
	*d = (*d)->prev;
	ret_d->nxt = NULL;
	return ret_d;
}

void free_data(struct data *d)
{
	struct que_mgmt *qm = &g_qm;
	struct data **td = NULL;

	if(d == NULL) return;

	d->size = 0;
	d->nxt = NULL;
	switch(d->chunk_size) {
		case 4:
			td = &qm->data_4;
			break;
		case 8:
			td = &qm->data_8;
			break;
		case 128:
			td = &qm->data_128;
			break;
		case 256:
			td = &qm->data_256;
			break;
		case 512:
			td = &qm->data_512;
			break;
		case 1024:
			td = &qm->data_1024;
			break;
		case 2048:
			td = &qm->data_2048;
	}

	d->prev = *td;
	*td = d;
}

void copy_data(struct data *dest, struct data *src, size_t size)
{
	dest->addr = src->addr;
	dest->rw = src->rw;
	dest->status = src->status;
	dest->didx = 0;
	dest->size = size;
	memcpy(dest->ptr, src->ptr + src->didx, size);
	src->addr += size;
	src->didx += size;
	src->size -= size;
}

void append_data(struct data **dest, struct data *src)
{
	if(*dest == NULL) {
		*dest = src;
	} else {
		while((*dest)->nxt != NULL) (*dest) = (*dest)->nxt;
		(*dest)->nxt = src;
	}
}

int create_qp(struct que_pair *qp, size_t size, int type)
{
	qp->send = (struct que *) calloc(1, sizeof(struct que));
	if(qp->send == NULL)
		goto qp_send_error;
	
	qp->send->ele = (struct que_ele *) calloc(size, sizeof(struct que_ele));
	if(qp->send->ele == NULL)
		goto error_qp_send_ele;	

	if(type == RESPONSE) {
		qp->resp = (struct que *) calloc(1, sizeof(struct que));
		if(qp->resp == NULL)
			goto qp_resp_error;

		qp->resp->ele = (struct que_ele *) calloc(size, sizeof(struct que_ele));
		if(qp->resp->ele == NULL)
			goto error_qp_resp_ele;	
	}
	
	qp->size = size;
	qp->is_empty = !!size;

	return 0;

error_qp_resp_ele:
	free(qp->resp);
qp_resp_error:
	free(qp->send->ele);
error_qp_send_ele:
	free(qp->send);
qp_send_error:
	return -ENOMEM;
}

void destroy_qp(struct que_pair *qp)
{
	free(qp->resp->ele);
	free(qp->resp);
	free(qp->send->ele);
	free(qp->send);
}

/*
 * used by slave port to move next send queue element once complete transaction
 * has been received
 */
void mv_nxt_send_que_ele(struct que_pair *qp)
{
	for(int loop = 0; loop < qp->size; loop++) {
		qp->send->pidx = (qp->send->pidx + 1) % qp->size;
		if(qp->send->ele[ qp->send->pidx ].status == FREE) {
			qp->is_empty = !!qp->size;
			return;
		}
	}
	qp->is_empty = 0;
	return;
}

/**
 * used by slave port add store incomming transactions
 */
struct que_ele* rsrv_send_que_ele(void *opaque, uint16_t *trid,
								  uint8_t resp_act__type)
{
	struct port *p = (struct port *) opaque;
	struct que_pair *qp = &(p->qp);
	struct que_ele *ele = &(qp->send->ele[ qp->send->pidx ]);

	if(trid && *trid < qp->size) {
		if(qp->send->ele[ *trid ].status == FREE) {
			qp->send->pidx = *trid;
			ele = &(qp->send->ele[ qp->send->pidx ]); 
		} else {
			ele = NULL;
		}
	}

	if(ele != NULL) {
		ele->status = PENDING;
		ele->resp_act.type = resp_act__type;
		ele->resp_act.resp_idx = qp->send->pidx;
		ele->resp_act.port = opaque;
	}
	return ele;
}

/*
 *	will be used when master transmitting the data
 */
struct que_ele* get_send_que_ele_check_resp(void *opaque, uint16_t *trid)
{
	struct port *p = (struct port *) opaque;
	struct que_pair *qp = &(p->qp);
	int loop = 0;
	struct que_ele *send_ele = NULL, *resp_ele = NULL;

	if(qp->send->ele[ qp->send->cidx ].sending) {
		*trid = qp->send->cidx;
		return &(qp->send->ele[ qp->send->cidx ]);
	}

	for(loop = 0; loop < qp->size; loop++) {
		send_ele = &(qp->send->ele[ qp->send->cidx ]);
		resp_ele = &(qp->resp->ele[ qp->send->cidx ]);
		if(send_ele->status == DONE || (p->frwd_as_rcvd &&
				(send_ele->status == RECEIVING)) && resp_ele->status == FREE) {
			*trid = qp->send->cidx;
			send_ele->sending = 1;
			resp_ele->status = PENDING;
			resp_ele->resp_act.type = send_ele->resp_act.type;
			resp_ele->resp_act.resp_idx = send_ele->resp_act.resp_idx;
			resp_ele->resp_act.port = send_ele->resp_act.port;
			return send_ele;
		}
		qp->send->cidx = (qp->send->cidx + 1) % qp->size;
	}
	return NULL;
}

int reserve_data(struct data **d, size_t size)
{
	int loop;
	struct data *ld = (struct data *) calloc(PRE_ALLOCATED_DATA,
											 sizeof(struct data));
	if(ld == NULL)
		return -ENOMEM;

	uint8_t *dptr = (uint8_t *) calloc(PRE_ALLOCATED_DATA, size);
	if(dptr == NULL)
		goto dptr_error;


	for(loop = 0; loop < PRE_ALLOCATED_DATA; loop++) {
		ld[loop].ptr = &dptr[loop * size];
		ld[loop].chunk_size = size;
		ld[loop].prev = *d;
		*d = &ld[loop];
	}
	return 0;

dptr_error:
	free(ld);
	return -ENOMEM;
}

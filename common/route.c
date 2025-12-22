#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "port_mgmt.h"
#include "route.h"

/*
 * need to modify for other cases as well like data start to map from mid
 * ex: data is starting from 0x0 but port mapping starting from 0x10 in this
 * case exclude starting data and start to transmit from 0x0 from data
 */
static struct port* get_master_from_memmap(struct blk *blk, struct data *d,
											uint16_t *size)
{
	struct memmap *mmap = blk->memmap;
	while(mmap != NULL) {
		if(mmap->addr <= d->addr && d->addr < mmap->end_addr) {
			if(d->size <= (mmap->end_addr - d->addr)) {
				*size = d->size;
			} else {
				*size = mmap->size - (d->addr - mmap->addr);
			}
			return mmap->ptr;
		}
		mmap = mmap->nxt;
	}
	return NULL;
}

static int get_dest_idx(struct port *dest_port, uint16_t *dest_idx,
						struct port *src_port, uint16_t src_idx)
{
	int ret = 0;
	struct que_pair *qp = &( dest_port->qp );
	struct que_ele *qe, *tmp_qe = NULL;
	int loop;
	
	for(loop = 0; loop < qp->size; loop++) {
		qe = &( qp->send->ele[ loop ]);
		if(qe->status == RECEIVING) {
			if((qe->src_info.port == src_port) &&
					(qe->src_info.idx == src_idx)) {
				*dest_idx = loop;
				return 0;
			}
		} else if(tmp_qe == NULL && qe->status == FREE) {
			tmp_qe = &( qp->send->ele[ loop ]);
			*dest_idx = loop;
			ret = 1;
		}
	}

	if(ret) {
		tmp_qe->src_info.port = src_port;
		tmp_qe->src_info.idx = src_idx;
	}
	return !ret;
}

static void slave_send_queue(struct blk *blk)
{
	uint16_t loop, dest_idx;
	uint16_t size = 0;
	struct port *src_port = blk->port;
	struct port *dest_port;
	struct que_pair *qp;
	struct que_ele *src_qe, *dest_qe;
	struct data *d;

	while(src_port != NULL) {
		if(src_port->type == SLAVE) {
			qp = &( src_port->qp );
			for(loop = 0; loop < qp->size; loop++) {
				src_qe = &( qp->send->ele[ loop ] );
				if((src_port->frwd_as_rcvd && src_qe->status == RECEIVING) ||
									  		 (src_qe->status == DONE)) {
					while( src_qe->d != NULL) {
						dest_port = get_master_from_memmap(blk, src_qe->d, &size);
						if(dest_port == NULL) {
							d = src_qe->d;
							src_qe->d = d->nxt;
							free_data(d);
							continue;
						}
						if(get_dest_idx(dest_port, &dest_idx, src_port, loop)) {
							break;
						}
						dest_qe = &( dest_port->qp.send->ele[ dest_idx ]);
						if(size == src_qe->d->size) {
							append_data(&(dest_qe->d), src_qe->d);
							d = src_qe->d;
							src_qe->d = src_qe->d->nxt;
						} else if(size < src_qe->d->size) {
							d = alloc_data(size);
							copy_data(d, src_qe->d, size);
							append_data(&(dest_qe->d), d);
						}
						if(src_qe->d != NULL) {
							if((src_qe->d->addr != (d->addr + d->didx + d->size)) ||
								(src_qe->d->rw != d->rw)) {
								dest_qe->status = DONE;
							}
						} else {
							dest_qe->status = src_qe->status;
						}
						dest_qe->resp_act = src_qe->resp_act;
						if(src_qe->d == NULL) {
							src_qe->status = FREE;
							qp->is_empty = 1;
						}
					}
				}
			}
		}
		src_port = src_port->nxt;
	}
}

void route(struct blk *blk)
{
	slave_send_queue(blk);
}

void master_route_resp(struct port *p)
{
	int loop, func_d = 0;
	struct port *target_port = NULL;
	struct que_pair *qp = &(p->qp);
	struct que_ele *resp_ele, *target_resp_ele = NULL;
	struct que_ele_resp_action *resp_act;
	struct data *tmp__d = NULL;
	struct resp_data *resp_d = NULL;

	for(loop = 0; loop < qp->size; loop++) {
		resp_ele = &(qp->resp->ele[loop]);
		resp_act = &(resp_ele->resp_act);

		if(resp_ele->status == PENDING || resp_ele->status == FREE)
			continue;

		target_port = (struct port *) resp_act->port;
		if(resp_act->type == que_ele_resp__port) {
			if((p->frwd_as_rcvd && resp_ele->status == RECEIVING) ||
					resp_ele->status == DONE) { 
				target_resp_ele = &( target_port->qp.resp->ele[ resp_act->resp_idx ] );
				target_resp_ele->status = resp_ele->status;
				if(target_resp_ele->d == NULL) {
					target_resp_ele->d = resp_ele->d;
				} else {
					tmp__d = target_resp_ele->d;
					while(tmp__d->nxt != NULL) tmp__d = tmp__d->nxt;
					tmp__d->nxt = resp_ele->d;
				}
				resp_ele->d = NULL;
				if(resp_ele->status == DONE) resp_ele->status = FREE;
			}
		} else if(resp_act->type == que_ele_resp__func &&
				resp_ele->status == DONE) {
			tmp__d = resp_ele->d;
			resp_d = &( p->respd );
			resp_d->size = 0;
			if(tmp__d != NULL) {
				resp_d->trid = resp_act->resp_idx;
				resp_d->addr = tmp__d->addr;
				resp_d->rw = tmp__d->rw;
			}
			while(tmp__d != NULL) {
				func_d++;
				resp_d->status = tmp__d->status;
				resp_d->size += tmp__d->size;
				tmp__d = tmp__d->nxt;
			}
			if(func_d > 1) {
				resp_d->data = (uint8_t *) malloc(resp_d->size);
				resp_d->size = 0;
				while(resp_ele->d != NULL) {
					tmp__d = resp_ele->d;
					memcpy(resp_d->data + resp_d->size, tmp__d->ptr, tmp__d->size);
					resp_d->size += tmp__d->size;
					resp_ele->d = resp_ele->d->nxt;
					free_data(tmp__d);
				}
				p->resp_callback(p);
			} else {
				if(resp_ele->d != NULL) {
					resp_d->data = resp_ele->d->ptr;
				} else {
					resp_d->data = NULL;
				}
				p->resp_callback(p);
				if(resp_ele->d != NULL) {
					free_data(resp_ele->d);
				}
				resp_ele->d = NULL;
			}
			resp_ele->status = FREE;
		}
	}
}

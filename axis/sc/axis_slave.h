#ifndef __axis_slave_h__
#define __axis_slave_h__

#include "systemc.h"
#include "common.h"
#include "axis_if.h"
#include "axis.h"

using namespace std;
using namespace sc_core;

namespace axis {

template <int TDATA_WIDTH, int TID_WIDTH, int TDEST_WIDTH, int TUSER_DEST>
class slave : sc_module
{
public:
    sc_in<bool>                  aclk;
    sc_in<sc_logic>              areset_n;
    sc_in<bool>		             tvalid;
    sc_out<bool>	  	         tready;
    sc_in<sc_bv<TDATA_WIDTH> >   tdata;
    sc_in<sc_bv<TDATA_WIDTH/8> > tstrb;
    sc_in<sc_bv<TDATA_WIDTH/8> > tkeep;
    sc_in<bool>		             tlast;
    sc_in<sc_bv<TID_WIDTH> >     tid;
    sc_in<sc_bv<TDEST_WIDTH> >   tdest;
    sc_in<sc_bv<TUSER_DEST> >    tuser;
    sc_in<bool>	 	             twakeup;

	struct port *port = NULL;
	uint64_t addr = 0;
	uint16_t signal_size = (TDATA_WIDTH / 8) + !!(TDATA_WIDTH % 8); // implement this in apb
	uint16_t size;
	uint8_t data[ (TDATA_WIDTH / 8) + !!(TDATA_WIDTH % 8) ];
	struct que_pair *qp = NULL;
	struct que_ele *send_ele = NULL, *resp_ele = NULL;

    SC_HAS_PROCESS(slave);

    /**
     * @brief Constructor for the apb slave class
     * @param name The name of the module
     * @param file Pointer to waveform file
     */
    explicit slave(sc_module_name name, sc_trace_file *file = NULL) :
    sc_module(name), aclk("aclk"), areset_n("areset_n"), tvalid("tvalid"),
    tready("tready"), tdata("tdata"), tstrb("tstrb"), tkeep("tkeep"),
    tlast("tlast"), tid("tid"), tdest("tdest"), tuser("tuser"),
    twakeup("twakeup")
    {
        if(file != NULL)
            this->add_to_wave(file);

        SC_METHOD(seq__slave);
        sensitive << aclk.pos() << areset_n.neg();
        dont_initialize();
    }

#if 0
    ~slave()
    {
        default_destroy(obj);
    }
#endif

    slave& operator () (axis_if
               <TDATA_WIDTH, TID_WIDTH, TDEST_WIDTH, TUSER_DEST>& axisif)
    {
        aclk    (axisif.clk);
        areset_n(axisif.rst_n);
        tvalid  (axisif.tvalid);
        tready  (axisif.tready);
        tdata   (axisif.tdata);
        tstrb   (axisif.tstrb);
        tkeep   (axisif.tkeep);
        tlast   (axisif.tlast);
        tid     (axisif.tid);
        tdest   (axisif.tdest);
        tuser   (axisif.tuser);
        twakeup (axisif.twakeup);
        return *this;
    }

    slave& operator () (axis_if_s
               <TDATA_WIDTH, TID_WIDTH, TDEST_WIDTH, TUSER_DEST>& axisif)
    {
        aclk    (axisif.clk);
        areset_n(axisif.rst_n);
        tvalid  (axisif.tvalid);
        tready  (axisif.tready);
        tdata   (axisif.tdata);
        tstrb   (axisif.tstrb);
        tkeep   (axisif.tkeep);
        tlast   (axisif.tlast);
        tid     (axisif.tid);
        tdest   (axisif.tdest);
        tuser   (axisif.tuser);
        twakeup (axisif.twakeup);
        return *this;
    }

private:

	void seq__slave()
	{
		struct data *d = NULL, *tmp__d = NULL;
        if(areset_n == sc_logic_0) {
			tready.write( 0 );
		} else {
			if(port_status(port)) {
				if(port->qp.is_empty) {
					tready.write( 1 );
				}
				if(tvalid.read() == 1) {
					size = bits_to_bytes<TDATA_WIDTH>(tdata, data, signal_size);
					d = alloc_data(size);
					memcpy(d->ptr, data, size);
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
						tready.write( 1 );
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
						if(tlast.read() == 1) {
							send_ele->status = DONE;
						}
						tready.write( 1 );
					}
					addr += size;
					if(tlast.read() == 1) {
						addr = 0;
					}
				}
			} else {
				tready.write( 0 );
			}
		}
	}
#if 0
private:
    sc_signal<sc_uint<3> > state;
    uint16_t dlen = 0;
    struct data_list *dl, *dlh, *dlt;
    struct breq *req;
    uint64_t req_addr;

    void slave_seq() {
        if(i_areset_n == sc_logic_0) {
            state = IDLE;
        } else {
            if(obj->parent->dhl == NULL) {
                state = BUSY;
                return;
            }

            if(dl == NULL) {
                dl = alloc_dle();
                if(dl) {
                    state = ACCEPT;
                } else {
                    state = BUSY;
                    return;
                }
            }

            if(i_tvalid == sc_logic_1) {
                if(add_data_seg()) {
                    state = ACCEPT;
                    dl = NULL;
                } else {
                    state = BUSY;
                }

                req = alloc_req_mem(dlen);
                req->addr = req_addr;
                req_addr += TDATA_WIDTH / 8;
                if(i_tlast == sc_logic_1) {
                    req_addr = 0;
                    req->end = i_tlast == sc_logic_1;
                }
                add_dl_to_req(req);
                req->rw = 1;
                frwd_req(obj, req);
                dlen = 0;
                dlh = NULL;
                dlt = NULL;
            }
        }
    }

    void slave_comb()
    {
        sc_uint<3> lstate = state;
        switch(lstate) {
            case IDLE:
            case BUSY:
                o_tready = sc_logic_0;
                break;
            case ACCEPT:
                o_tready = sc_logic_1;
                break;
        }
    }

    struct data_list* alloc_dle()
    {
        struct data_list *ldl;
        ldl = (struct data_list *) malloc(sizeof(struct data_list));
        if(ldl == NULL)
            goto error;
        memset(ldl, 0, sizeof(struct data_list));

        ldl->data = (uint8_t *) malloc(TDATA_WIDTH / 8);
        if(ldl->data == NULL)
            goto data_error;
        memset(ldl->data, 0, TDATA_WIDTH / 8);

        return ldl;

        data_error:
            free(ldl);
        error:
            return NULL;
    }

    int add_data_seg()
    {
        sc_bv<8> ltmp_data;
        sc_bv<TDATA_WIDTH> tmp_i_tdata = i_tdata;
        sc_bv<TDATA_WIDTH/8> tmp_i_tstrb = i_tstrb;

        for(int i = 0; i < TDATA_WIDTH / 8; i++) {
            if(tmp_i_tstrb[i] == sc_logic_1) {
                ltmp_data = tmp_i_tdata.range((i * 8) + 7, i * 8);
                *(dl->data + i) = static_cast<sc_uint<8> >(ltmp_data);
            }
        }
        dlen += (TDATA_WIDTH / 8);

        if(dlh == NULL) {
            dlh = dl;
        } else {
            dlt->nxt = dl;
        }
        dlt = dl;

        return (TDATA_WIDTH / 8);
    }

    void add_dl_to_req(struct breq *req)
    {
        uint8_t *dptr = req->data;
        struct data_list *rdl, *ldl = dlh;

        while(ldl != NULL) {
            rdl = ldl;
            memcpy(dptr, ldl->data, (TDATA_WIDTH / 8));
            dptr += (TDATA_WIDTH / 8);
            ldl = ldl->nxt;
            free(rdl->data);
            free(rdl);
        }
    }

    void frwd_req(struct bobj *obj, struct breq* req)
    {
        uint16_t ret = 0, set_resp = 1;
        uint16_t chk_idx;
        struct data_handler *dh = obj->parent->dhl;
        while(dh != NULL) {
            ret = dh->request_acceptor(dh->mem, req, &chk_idx);
            if(set_resp) {
                if(ret) {
                    state = ACCEPT;
                } else {
                    state = BUSY;
                }
                set_resp = 0;
            }
            dh = dh->nxt;
        }
    }
#endif

	template<int N>
	uint16_t bits_to_bytes(const sc_bv<N>& in, uint8_t* arr, int nbytes) {
		int size = 0;
		sc_bv<N/8> strb = tstrb.read();
	    for (int i = 0; i < nbytes && (i*8+7) < N; i++) {
	        // Extract 8 bits into a uint8_t
	        sc_bv<8> byte_bv = in.range(i*8+7, i*8);
	        arr[i] = static_cast<uint8_t>(byte_bv.to_uint());
			size += (strb[i] == '1');
	    }
		return size;
	}

    /**
     * @brief Function to add signals to waveforms
     * @param file Pointer to waveform file
     */
    void add_to_wave(sc_trace_file *file)
    {
        sc_trace(file, aclk,     aclk.name());
        sc_trace(file, areset_n, areset_n.name());
        sc_trace(file, tvalid,   tvalid.name());
        sc_trace(file, tready,   tready.name());
        sc_trace(file, tdata,    tdata.name());
        sc_trace(file, tstrb,    tstrb.name());
        sc_trace(file, tkeep,    tkeep.name());
        sc_trace(file, tlast,    tlast.name());
        sc_trace(file, tid,      tid.name());
        sc_trace(file, tdest,    tdest.name());
        sc_trace(file, tuser,    tuser.name());
        sc_trace(file, twakeup,  twakeup.name());
    }
};

} // namespace axis

#endif // __axis_slave_h__

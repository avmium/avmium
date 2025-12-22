#ifndef __apb_master_hpp__
#define __apb_master_hpp__

#include "systemc.h"
#include "common.h"
#include "apb_if.h"
#include "apb.h"

using namespace std;
using namespace sc_core;

namespace apb {

template <int ADDR_WIDTH, int DATA_WIDTH, int USER_REQ_WIDTH,
            int USER_DATA_WIDTH, int USER_RESP_WIDTH>
class master : sc_module
{
public:
    sc_in<bool>                     pclk;
    sc_in<sc_logic>                 preset_n;
    sc_out<sc_lv<ADDR_WIDTH> >      paddr;
    sc_out<sc_lv<3> >               pprot;
    sc_out<sc_logic>                pnse;
    sc_out<sc_logic>                psel_x;
    sc_out<sc_logic>                penable;
    sc_out<sc_logic>                pwrite;
    sc_out<sc_bv<DATA_WIDTH> >      pwdata;
    sc_out<sc_lv<DATA_WIDTH/8> >    pstrb;
    sc_in<sc_logic>                 pready;
    sc_in<sc_bv<DATA_WIDTH> >       prdata;
    sc_in<sc_logic>                 pslverr;
    sc_out<sc_logic>                pwakeup;
    sc_out<sc_lv<USER_REQ_WIDTH> >  pauser;
    sc_out<sc_lv<USER_DATA_WIDTH> > pwuser;
    sc_in<sc_lv<USER_DATA_WIDTH> >  pruser;
    sc_in<sc_lv<USER_RESP_WIDTH> >  pbuser;

	uint8_t rw;
	uint16_t signal_size = DATA_WIDTH/8;
	uint16_t size;
	uint16_t trid = 0;
	uint64_t addr;
	uint8_t data[ DATA_WIDTH/8 ];
	struct port *port = NULL;
	struct que_pair *qp = NULL;
	struct que_ele *send_ele = NULL, *resp_ele = NULL;
	struct data *d = NULL, *tmp__d = NULL;

    SC_HAS_PROCESS(master);

    /**
     * @brief Constructor for the apb_master class
     * @param name The name of the module
     * @param file Pointer to waveform file
     */
    explicit master(sc_module_name name, sc_trace_file *file = NULL) :
    sc_module(name), pclk("pclk"), preset_n("preset_n"),
	paddr("paddr"), pprot("pprot"), pnse("pnse"),
	psel_x("psel_x"), penable("penable"), pwrite("pwrite"),
	pwdata("pwdata"), pstrb("pstrb"), pready("pready"),
    prdata("prdata"), pslverr("pslverr"), pwakeup("pwakeup"),
    pauser("pauser"), pwuser("pwuser"), pruser("pruser"),
	pbuser("pbuser")
    {
        if(file != NULL)
            this->add_to_wave(file);

		SC_METHOD(seq__state);
        sensitive << pclk.pos() << preset_n.neg();
        dont_initialize();

        SC_METHOD(comb__state);
        sensitive << state;
        dont_initialize();

        SC_METHOD(master_route);
        sensitive << pclk;
        dont_initialize();
    }

    master& operator () (apb_if<ADDR_WIDTH, DATA_WIDTH, USER_REQ_WIDTH,
                                USER_DATA_WIDTH, USER_RESP_WIDTH>& apbif)
    {
        pclk     (apbif.clk);
        preset_n (apbif.rst_n);
        paddr    (apbif.paddr);
        pprot    (apbif.pprot);
        pnse     (apbif.pnse);
        psel_x   (apbif.psel_x);
        penable  (apbif.penable);
        pwrite   (apbif.pwrite);
        pwdata   (apbif.pwdata);
        pstrb    (apbif.pstrb);
        pready   (apbif.pready);
        prdata   (apbif.prdata);
        pslverr  (apbif.pslverr);
        pwakeup  (apbif.pwakeup);
        pauser   (apbif.pauser);
        pwuser   (apbif.pwuser);
        pruser   (apbif.pruser);
        pbuser   (apbif.pbuser);
        return *this;
    }

    master& operator () (apb_if_m<ADDR_WIDTH, DATA_WIDTH, USER_REQ_WIDTH,
                                USER_DATA_WIDTH, USER_RESP_WIDTH>& apbifm)
    {
        pclk     (apbifm.clk);
        preset_n (apbifm.rst_n);
        paddr    (apbifm.paddr);
        pprot    (apbifm.pprot);
        pnse     (apbifm.pnse);
        psel_x   (apbifm.psel_x);
        penable  (apbifm.penable);
        pwrite   (apbifm.pwrite);
        pwdata   (apbifm.pwdata);
        pstrb    (apbifm.pstrb);
        pready   (apbifm.pready);
        prdata   (apbifm.prdata);
        pslverr  (apbifm.pslverr);
        pwakeup  (apbifm.pwakeup);
        pauser   (apbifm.pauser);
        pwuser   (apbifm.pwuser);
        pruser   (apbifm.pruser);
        pbuser   (apbifm.pbuser);
        return *this;
    }

private:
	uint8_t wait_for_response = 0;
    sc_signal<sc_lv<2> > state, nxt_state;

	void seq__state()
	{
        if(preset_n == sc_logic_0)
            state = IDLE;
        else {
			if(state.read() == ACCESS) {
				if(pready == sc_logic_1) {
					qp = &(port->qp);
					send_ele = &(qp->send->ele[	qp->send->cidx ]);
					resp_ele = &(qp->resp->ele[	qp->send->cidx ]);
					resp_ele->status = RECEIVING;
					d = alloc_data(size);
					d->addr = (rw) ? 0 : addr;
					d->size = (rw) ? 0 : size;
					d->rw = (rw) ? 0 : rw;
					d->status = pslverr.read().to_bool();
					d->didx = 0;
					if(!rw) {
						bits_to_bytes<DATA_WIDTH>(prdata, d->ptr, size);
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
					wait_for_response = 0;
				} else {
					wait_for_response = 1;
				}
			}
			if(!wait_for_response) {
				if(port_status(port) && nxt_state.read() == IDLE) {
					// call	 user function
					size = signal_size;
				   	if(get_send_data(port, &trid, &addr, &rw, data, &size)) {
						state = SETUP;
					} else {
            			state = nxt_state;
					}
				} else {
            		state = nxt_state;
				}
			}
		}
	}

	void master_route() { master_route_resp(port); }

    /**
     * @brief This function executes the combinational logic of apb master
     */
    void comb__state()
    {
        sc_uint<2> t_state = state.read();
        switch(t_state) {
            case IDLE:
                idle_op();
                break;
            case SETUP:
                setup_op();
                break;
            case ACCESS:
                access_op();
                break;
        }
    }

    /**
     * @brief This function is used to operate in idle phase
     */
    void idle_op()
    {
        paddr       = (uint32_t) 0x0;
        pprot       = (uint32_t) 0x0;
        pnse        = sc_logic_0;
        psel_x      = sc_logic_0;
        penable     = sc_logic_0;
        pwrite      = sc_logic_0;
        pwdata      = (uint32_t) 0x0;
        pstrb       = (uint32_t) 0x0;
        pwakeup     = sc_logic_0;
        pauser      = (uint32_t) 0x0;
        pwuser      = (uint32_t) 0x0;
		nxt_state	= IDLE;
    }

    /**
     * @brief This function is used to operate in setup phase
     */
    void setup_op()
    {
        sc_lv<DATA_WIDTH>   tmp_pwdata;
        sc_lv<DATA_WIDTH/8> tmp_pstrb;
        sc_lv<DATA_WIDTH> allzeros ('0');

        paddr     = (port->addr_normalize) ? addr - port->mmap_addr : addr;
        pwrite 	  = rw ? sc_logic_1 : sc_logic_0;
        psel_x    = sc_logic_1;
        penable   = sc_logic_0;
		pwdata.write(bytes_to_bits<DATA_WIDTH>(data, size));
		if(!rw) {
			pwdata = allzeros;
		}
        nxt_state = ACCESS;
	}

    /**
     * @brief This function is used to operate in access phase
     */
    void access_op()
    {
        penable = sc_logic_1;
        nxt_state = IDLE;
    }
	
	template<int N>
	void bits_to_bytes(const sc_bv<N>& in, uint8_t* arr, int nbytes) {
	    for (int i = 0; i < nbytes && (i*8+7) < N; i++) {
	        // Extract 8 bits into a uint8_t
	        sc_bv<8> byte_bv = in.range(i*8+7, i*8);
	        arr[i] = static_cast<uint8_t>(byte_bv.to_uint());
	    }
	}

	template<int N>
	sc_bv<N> bytes_to_bits(const uint8_t* arr, int nbytes)
	{
	    sc_bv<N> result;
	    sc_bv<N/8> strb = 0;
	
	    for (int i = 0; i < nbytes && (i*8+7) < N; i++) {
	        result.range(i*8+7, i*8) = arr[i];
			strb[i] = '1';
	    }
		pstrb.write( strb );
	
	    return result;
	}
#if 0
    struct breq *curr_trans = NULL;
    uint16_t wdidx = 0, rdidx = 0, curr_data_len_left = 0;

    void master_sm() {
        if(preset_n == sc_logic_0)
            curr_state = IDLE;
        else {
            curr_state = nxt_state;

            if(pready == sc_logic_1) {
                if(pwrite == sc_logic_0) {
                    sc_lv<DATA_WIDTH> tmp_prdata = prdata;
                    sc_lv<8> ltmp_data;
                    for(int i = 0; i < DATA_WIDTH / 8; i++, rdidx++) {
                        if(curr_data_len_left) {
                            ltmp_data = tmp_prdata.range((i * 8) + 7, i * 8);
                            *(curr_trans->data + rdidx) =
                                static_cast<sc_uint<8> >(ltmp_data);
                            curr_data_len_left--;
                        }
                    }
                }
                if(!curr_data_len_left) {
                    curr_trans->status = SUCCESS;
                    curr_trans = NULL;
                }
            }

            if(curr_trans == NULL || (pready == sc_logic_1 &&
                                      nxt_state.read() == ACCESS)) {
                if(curr_data_len_left) {
                    curr_state = SETUP;
                } else if (obj->pidx != obj->cidx) {
                    curr_trans = get_new_trans(obj);
                    wdidx = 0;
                    rdidx = 0;
                    curr_data_len_left = curr_trans->size;
                    curr_state = SETUP;
                } else {
                    curr_state = IDLE;
                }
            }
        }
    }

    /**
     * @brief This function is used to operate in setup phase
     */
    void setup_op()
    {
        sc_lv<DATA_WIDTH>   tmp_pwdata;
        sc_lv<DATA_WIDTH/8> tmp_pstrb;
        sc_lv<DATA_WIDTH> allxs ('X');
        uint16_t be_offset;

        paddr     = (uint64_t) (curr_trans->addr + wdidx);
        psel_x    = sc_logic_1;
        penable   = sc_logic_0;
        nxt_state = ACCESS;

        if(curr_trans->rw) {
            pwrite = sc_logic_1;
            for(int i = 0; i < DATA_WIDTH / 8; i++, wdidx++) {
                if(curr_data_len_left) {
                    if(curr_trans->be) {
                        be_offset = wdidx / 8;
                        if(*(curr_trans->be + be_offset) & (1 << (i % 8))) {
                            tmp_pstrb[i] = sc_logic_1;
                        } else {
                            tmp_pstrb[i] = sc_logic_0;
                        }
                    } else {
                        tmp_pstrb[i] = sc_logic_1;
                    }
                    tmp_pwdata.range((i * 8) + 7, i * 8) =
                            *(curr_trans->data + wdidx);
                    curr_data_len_left--;
                } else {
                    tmp_pwdata.range((i * 8) + 7, i * 8) = 0x0;
                    tmp_pstrb[i] = sc_logic_0;
                }
            }
            pwdata = tmp_pwdata;
            pstrb = tmp_pstrb;
        } else {
            pwdata = allxs;
            pstrb = allxs;
            pwrite = sc_logic_0;
        }
    }

#endif

    void add_to_wave(sc_trace_file *file)
    {
        sc_trace(file, pclk,     pclk.name());
        sc_trace(file, preset_n, preset_n.name());
        sc_trace(file, paddr,    paddr.name());
        sc_trace(file, pprot,    pprot.name());
        sc_trace(file, pnse,     pnse.name());
        sc_trace(file, psel_x,   psel_x.name());
        sc_trace(file, penable,  penable.name());
        sc_trace(file, pwrite,   pwrite.name());
        sc_trace(file, pwdata,   pwdata.name());
        sc_trace(file, pstrb,    pstrb.name());
        sc_trace(file, pready,   pready.name());
        sc_trace(file, prdata,   prdata.name());
        sc_trace(file, pslverr,  pslverr.name());
        sc_trace(file, pwakeup,  pwakeup.name());
        sc_trace(file, pauser,   pauser.name());
        sc_trace(file, pwuser,   pwuser.name());
        sc_trace(file, pruser,   pruser.name());
        sc_trace(file, pbuser,   pbuser.name());
    }
};

} // namespace apb

#endif // __apb_master_hpp__

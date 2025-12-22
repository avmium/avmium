#ifndef __apb_slave_h__
#define __apb_slave_h__

#include "systemc.h"
#include "common.h"
#include "apb_if.h"
#include "apb.h"

using namespace std;
using namespace sc_core;

namespace apb {

template <int ADDR_WIDTH, int DATA_WIDTH,
          int USER_REQ_WIDTH, int USER_DATA_WIDTH,
          int USER_RESP_WIDTH>
class slave : public sc_core::sc_module
{
public:
    sc_in<bool>                     pclk;
    sc_in<sc_logic>                 preset_n;
    sc_in<sc_lv<ADDR_WIDTH> >       paddr;
    sc_in<sc_lv<3> >                pprot;
    sc_in<sc_logic>                 pnse;
    sc_in<sc_logic>                 psel;
    sc_in<sc_logic>                 penable;
    sc_in<sc_logic>                 pwrite;
    sc_in<sc_bv<DATA_WIDTH> >       pwdata;
    sc_in<sc_lv<DATA_WIDTH/8> >     pstrb;
    sc_out<sc_logic>                pready;
    sc_out<sc_bv<DATA_WIDTH> >      prdata;
    sc_out<sc_logic>                pslverr;
    sc_in<sc_logic>                 pwakeup;
    sc_in<sc_lv<USER_REQ_WIDTH> >   pauser;
    sc_in<sc_lv<USER_DATA_WIDTH> >  pwuser;
    sc_out<sc_lv<USER_DATA_WIDTH> > pruser;
    sc_out<sc_lv<USER_RESP_WIDTH> > pbuser;

	uint8_t rw, wait_for_response = 0;
	uint16_t signal_size = DATA_WIDTH/8;
	uint16_t size;
	uint8_t data[ DATA_WIDTH/8 ];
	struct port *port = NULL;
	struct que_pair *qp = NULL;
	struct que_ele *send_ele = NULL, *resp_ele = NULL;
	uint8_t state = IDLE;

    SC_HAS_PROCESS(slave);

    /**
     * @brief Constructor for the apb slave class
     * @param name The name of the module
     * @param file Pointer to waveform file
     */
    explicit slave(sc_module_name name, sc_trace_file *file = NULL) :
    sc_module(name), pclk("pclk"), preset_n("preset_n"), paddr("paddr"),
    pprot("pprot"), pnse("pnse"), psel("psel"), penable("penable"),
    pwrite("pwrite"), pwdata("pwdata"), pstrb("pstrb"), pready("pready"),
    prdata("prdata"), pslverr("pslverr"), pwakeup("pwakeup"),
    pauser("pauser"), pwuser("pwuser"), pruser("pruser"), pbuser("pbuser")
    {
        if(file != NULL)
            this->add_to_wave(file);

        SC_METHOD(pready_comb);
        sensitive << pclk.pos() << penable;
        dont_initialize();

        SC_METHOD(psel_comb);
        sensitive << preset_n.neg() << psel.pos() << penable.neg();
        dont_initialize();
    }

    ~slave()
    {
#if 0
        default_destroy(obj);
#endif
    }

	void psel_comb()
	{
        if(preset_n == sc_logic_0)
            state = IDLE;
		else
			state = ACCESS;
	}

	void pready_comb() {
		struct que_ele *send_ele = NULL;
		struct data *d = NULL, *tmp__d = NULL;
		if(wait_for_response) {
			qp = &( port->qp );
			for(int loop = 0; loop < qp->size; loop++) {
				resp_ele = &( qp->resp->ele[ loop ] );
				if((port->frwd_as_rcvd && resp_ele->status == RECEIVING) ||
						resp_ele->status == DONE) { 
					wait_for_response = 0;
					pready = sc_logic_1;
					d = resp_ele->d;
					if(d != NULL) {
						resp_ele->d = d->nxt;
						if(!d->rw)	{// correct with variable size data
							prdata.write(bytes_to_bits<DATA_WIDTH>(d->ptr, signal_size));
						}
						free_data(d);
					}
					resp_ele->status = FREE;
				}
			}
		} else if(port_status(port) && state == ACCESS && penable == sc_logic_1
					&& !wait_for_response) {
			state = IDLE;
			rw = pwrite.read().to_bool();
			size = bits_to_bytes<DATA_WIDTH>(pwdata, data, signal_size);
			d = alloc_data(size);
			if(rw) memcpy(d->ptr, data, size);
			d->size = size;
			d->addr = paddr.read().to_uint64();
			d->rw = rw;
			d->didx = 0;
			d->status = 0;
			port->ctx.send_ele = NULL;
			port->ctx.curr_d = d;
			port->ctx.nxt_d = NULL;
			port->user_callback(port);
			if(set_memmap(port, d->addr, data, size, rw, 0)) {
				pslverr = sc_logic_0;
				pready = sc_logic_1;
				wait_for_response = 0;
				if(!rw) {
					prdata.write(bytes_to_bits<DATA_WIDTH>(data, size));
				}
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
				pslverr = sc_logic_0;
				pready = sc_logic_0;
				wait_for_response = 1;
			} else {
					pslverr = sc_logic_1;
					pready = sc_logic_1;
			}
		} else {
			pready = sc_logic_0;
		}
	}

	template<int N>
	uint16_t  bits_to_bytes(const sc_bv<N>& in, uint8_t* arr, int nbytes) {
		uint16_t size = 0;
		sc_bv<N/8> strb = pstrb.read();
	    for (int i = 0; i < nbytes && (i*8+7) < N; i++) {
	        // Extract 8 bits into a uint8_t
	        sc_bv<8> byte_bv = in.range(i*8+7, i*8);
	        arr[i] = static_cast<uint8_t>(byte_bv.to_uint());
			size += (strb[i] == '1');
	    }
		return size;
	}

	template<int N>
	sc_bv<N> bytes_to_bits(const uint8_t* arr, int nbytes)
	{
	    sc_bv<N> result;
	
	    for (int i = 0; i < nbytes && (i*8+7) < N; i++) {
	        result.range(i*8+7, i*8) = arr[i];
	    }
	
	    return result;
	}

    slave& operator () (apb_if<ADDR_WIDTH, DATA_WIDTH, USER_REQ_WIDTH,
                                USER_DATA_WIDTH, USER_RESP_WIDTH>& apbif)
    {
        pclk     (apbif.clk);
        preset_n (apbif.rst_n);
        paddr    (apbif.paddr);
        pprot    (apbif.pprot);
        pnse     (apbif.pnse);
        psel     (apbif.psel_x);
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

    slave& operator () (apb_if_s<ADDR_WIDTH, DATA_WIDTH, USER_REQ_WIDTH,
                                USER_DATA_WIDTH, USER_RESP_WIDTH>& apbifs)
    {
        pclk     (apbifs.clk);
        preset_n (apbifs.rst_n);
        paddr    (apbifs.paddr);
        pprot    (apbifs.pprot);
        pnse     (apbifs.pnse);
        psel     (apbifs.psel);
        penable  (apbifs.penable);
        pwrite   (apbifs.pwrite);
        pwdata   (apbifs.pwdata);
        pstrb    (apbifs.pstrb);
        pready   (apbifs.pready);
        prdata   (apbifs.prdata);
        pslverr  (apbifs.pslverr);
        pwakeup  (apbifs.pwakeup);
        pauser   (apbifs.pauser);
        pwuser   (apbifs.pwuser);
        pruser   (apbifs.pruser);
        pbuser   (apbifs.pbuser);
        return *this;
    }

    /**
     * @brief Function to add signals to waveforms
     * @param file Pointer to waveform file
     */
    void add_to_wave(sc_trace_file *file)
    {
        sc_trace(file, pclk,     pclk.name());
        sc_trace(file, preset_n, preset_n.name());
        sc_trace(file, paddr,    paddr.name());
        sc_trace(file, pprot,    pprot.name());
        sc_trace(file, pnse,     pnse.name());
        sc_trace(file, psel,     psel.name());
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

#endif // __apb_slave_h__

#ifndef __AXIS_MASTER_H__
#define __AXIS_MASTER_H__

#include "systemc.h"
#include "common.h"
#include "axis_if.h"
#include "axis.h"

using namespace std;
using namespace sc_core;

namespace axis {

enum apbm_os_e {
    IDLE,
    SETUP
};

template <int TDATA_WIDTH, int TID_WIDTH, int TDEST_WIDTH, int TUSER_WIDTH>
class master : sc_module
{
public:
    sc_in<bool>                   aclk;
    sc_in<sc_logic>               areset_n;
    sc_out<bool> 		          tvalid;
    sc_in<bool>	 	              tready;
    sc_out<sc_bv<TDATA_WIDTH> >   tdata;
    sc_out<sc_bv<TDATA_WIDTH/8> > tstrb;
    sc_out<sc_bv<TDATA_WIDTH/8> > tkeep;
    sc_out<bool> 	              tlast;
    sc_out<sc_bv<TID_WIDTH> >     tid;
    sc_out<sc_bv<TDEST_WIDTH> >   tdest;
    sc_out<sc_bv<TUSER_WIDTH> >   tuser;
    sc_out<bool> 	              twakeup;

	int ret = 0;
	uint8_t rw;
	uint16_t signal_size = (TDATA_WIDTH / 8) + !!(TDATA_WIDTH % 8);
	uint16_t size;// implement this in apb
	uint16_t trid = 0;
	uint64_t addr;
	uint8_t data[ (TDATA_WIDTH / 8) + !!(TDATA_WIDTH % 8) ];
	struct port *port = NULL;

    SC_HAS_PROCESS(master);

    /* @brief Constructor for the apb master class
     * @param name The name of the module
     * @param file Pointer to waveform file
     */
    explicit master(sc_module_name name, sc_trace_file *file = NULL,
                    struct bobj *parent_obj = NULL, uint16_t num_ot = 0,
                    int (*uinit_ptr)(void **mem) = NULL) :
    sc_module(name), aclk("aclk"), areset_n("areset_n"),
	tvalid("tvalid"), tready("tready"), tdata("tdata"),
	tstrb("tstrb"), tkeep("tkeep"), tlast("tlast"), tid("tid"),
	tdest("tdest"), tuser("tuser"), twakeup("twakeup")
    {
        if(file != NULL)
            this->add_to_wave(file);

		SC_METHOD(seq__state);
        sensitive << aclk.pos();
        dont_initialize();

        SC_METHOD(master_route);
        sensitive << aclk;
        dont_initialize();
    }

    ~master()
    {
    }

    master& operator () (axis_if
               <TDATA_WIDTH, TID_WIDTH, TDEST_WIDTH, TUSER_WIDTH>& axisif)
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

    master& operator () (axis_if_m
               <TDATA_WIDTH, TID_WIDTH, TDEST_WIDTH, TUSER_WIDTH>& axisif)
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
	uint8_t wait_for_response = 0;

	void seq__state()
	{
        if(areset_n == sc_logic_0) {
            idle_op();
		} else {
			if(wait_for_response) {
				wait_for_response = !tready.read();
			}
			if(!wait_for_response) {
				if(tlast.read() == 1) {
					axis_process_resp(port, trid);
				}
				if(port_status(port)) {
					size = signal_size;
					ret = get_send_data(port, &trid, &addr, &rw, data, &size);
					if(rw && ret) {
						setup_op();
					} else {
            			idle_op();
					}
				} else {
            		idle_op();
				}
			}
		}
	}

	void master_route() { master_route_resp(port); }

    void idle_op()
    {
        sc_bv<TDATA_WIDTH> allzeros ('0');

     	tvalid.write( 0 );
     	tlast.write( 0 );
     	twakeup.write( 0 );
     	tdata 		= allzeros;
     	tstrb 		= allzeros;
     	tkeep 		= allzeros;
     	tid   		= allzeros;
     	tdest 		= allzeros;
     	tuser 		= allzeros;

		addr		= 0x0;
    }

    void setup_op()
    {
        sc_bv<TDATA_WIDTH>   tmp_pwdata;
        sc_bv<TDATA_WIDTH/8> tmp_pstrb;
        sc_bv<TDATA_WIDTH> allzeros ('0');

		tdata.write(bytes_to_bits<TDATA_WIDTH>(data, size));
        tvalid.write( 1 );
		tlast.write( (ret == 2) );
		wait_for_response = (tready.read() == false);
	}

#if 0
private:

    /**
     * @brief This function executes the sequential logic axis master
     */
    void master_seq()
    {
        if(i_areset_n == sc_logic_0) {
            o_tvalid = sc_logic_0;
        } else {
            if(curr_trans != NULL && i_tready == sc_logic_0)
                return;

            if(curr_data_len_left) {
                drive_data();
                return;
            } else if(curr_trans != NULL) {
                curr_trans->status = SUCCESS;
                curr_trans = NULL;
            }

            if(obj->pidx != obj->cidx) {
                curr_trans = get_new_trans(obj);
                curr_data_len_left = curr_trans->size;
                widx = 0;
                drive_data();
            } else {
                drive_xs();
            }
        }
    }

    void drive_data()
    {
        sc_bv<TDATA_WIDTH> tmp_o_tdata;
        sc_bv<TDATA_WIDTH/8> tmp_o_tstrb;

        for(int i = 0; i < TDATA_WIDTH / 8; i++, widx++) {
            if(curr_data_len_left) {
                tmp_o_tdata.range((i * 8) + 7, i * 8)
                        = *(curr_trans->data + widx);
                tmp_o_tstrb[i] = sc_logic_1;
                curr_data_len_left--;
            } else {
                tmp_o_tdata.range((i * 8) + 7, i * 8) = 0x0;
                tmp_o_tstrb[i] = sc_logic_0;
            }
        }

        o_tvalid = sc_logic_1;
        o_tdata = tmp_o_tdata;
        o_tstrb = tmp_o_tstrb;
        o_tkeep = tmp_o_tstrb;
        o_tlast = curr_data_len_left ? sc_logic_0 : sc_logic_1;
    }
#endif

	template<int N>
	sc_bv<N> bytes_to_bits(const uint8_t* arr, int nbytes)
	{
	    sc_bv<N> result = 0;
	    sc_bv<N/8> strb = 0;
	
	    for (int i = 0; i < nbytes && (i*8+7) < N; i++) {
	        result.range(i*8+7, i*8) = arr[i];
			strb[i] = '1';
	    }

		tstrb.write( strb );
		tkeep.write( strb );
	
	    return result;
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

#endif // __AXIS_MASTER_H__

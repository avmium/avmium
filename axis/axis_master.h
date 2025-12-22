#ifndef __AXIS_MASTER_H__
#define __AXIS_MASTER_H__

#include "systemc.h"
#include "common.h"
#include "axis_if.h"
#include "axis.h"

using namespace std;
using namespace sc_core;

namespace axis {

enum states_e {
    IDLE,
    BUSY,
    ACCEPT
};

enum axis_status_e {
    PENDING,
    IN_PROGRESS,
    SUCCESS,
    W_ERROR,
    R_ERROR
};

template <int TDATA_WIDTH, int TID_WIDTH, int TDEST_WIDTH, int TUSER_WIDTH>
class master : sc_module
{
public:
    sc_in<bool>                   i_aclk;
    sc_in<sc_logic>               i_areset_n;
    sc_out<sc_logic>              o_tvalid;
    sc_in<sc_logic>               i_tready;
    sc_out<sc_lv<TDATA_WIDTH> >   o_tdata;
    sc_out<sc_lv<TDATA_WIDTH/8> > o_tstrb;
    sc_out<sc_lv<TDATA_WIDTH/8> > o_tkeep;
    sc_out<sc_logic>              o_tlast;
    sc_out<sc_lv<TID_WIDTH> >     o_tid;
    sc_out<sc_lv<TDEST_WIDTH> >   o_tdest;
    sc_out<sc_lv<TUSER_WIDTH> >   o_tuser;
    sc_out<sc_logic>              o_twakeup;

    struct bobj *obj;

    SC_HAS_PROCESS(master);

    /* @brief Constructor for the apb master class
     * @param name The name of the module
     * @param file Pointer to waveform file
     */
    explicit master(sc_module_name name, sc_trace_file *file = NULL,
                    struct bobj *parent_obj = NULL, uint16_t num_ot = 0,
                    int (*uinit_ptr)(void **mem) = NULL) :
    sc_module(name), i_aclk("i_aclk"), i_areset_n("i_areset_n"),
	o_tvalid("o_tvalid"), i_tready("i_tready"), o_tdata("o_tdata"),
	o_tstrb("o_tstrb"), o_tkeep("o_tkeep"), o_tlast("o_tlast"), o_tid("o_tid"),
	o_tdest("o_tdest"), o_tuser("o_tuser"), o_twakeup("o_twakeup")
    {
        if(file != NULL)
            this->add_to_wave(file);

        default_init(&obj, parent_obj, num_ot);
        obj->proto = AXIS;
        obj->type = MASTER;

        if(uinit_ptr)
            uinit_ptr(&obj->mem);

        SC_METHOD(master_seq);
        sensitive << i_aclk.pos();
        dont_initialize();
    }

    ~master()
    {
        destroy_tran_que(obj);
        default_destroy(obj);
    }

    master& operator () (axis_if
               <TDATA_WIDTH, TID_WIDTH, TDEST_WIDTH, TUSER_WIDTH>& axisif)
    {
        i_aclk    (axisif.clk);
        i_areset_n(axisif.rst_n);
        o_tvalid  (axisif.tvalid);
        i_tready  (axisif.tready);
        o_tdata   (axisif.tdata);
        o_tstrb   (axisif.tstrb);
        o_tkeep   (axisif.tkeep);
        o_tlast   (axisif.tlast);
        o_tid     (axisif.tid);
        o_tdest   (axisif.tdest);
        o_tuser   (axisif.tuser);
        o_twakeup (axisif.twakeup);
        return *this;
    }

    master& operator () (axis_if_m
               <TDATA_WIDTH, TID_WIDTH, TDEST_WIDTH, TUSER_WIDTH>& axisif)
    {
        i_aclk    (axisif.i_clk);
        i_areset_n(axisif.i_rst_n);
        o_tvalid  (axisif.o_tvalid);
        i_tready  (axisif.i_tready);
        o_tdata   (axisif.o_tdata);
        o_tstrb   (axisif.o_tstrb);
        o_tkeep   (axisif.o_tkeep);
        o_tlast   (axisif.o_tlast);
        o_tid     (axisif.o_tid);
        o_tdest   (axisif.o_tdest);
        o_tuser   (axisif.o_tuser);
        o_twakeup (axisif.o_twakeup);
        return *this;
    }

    void clk_handler(void) {
        obj->clk_handler(obj->mem);
    }

    void bind_clk_handler(void (*usr_clk_handler)(uint8_t *mem), uint8_t edge)
    {
        obj->clk_handler = usr_clk_handler;
        SC_METHOD(clk_handler);
        sensitive << (edge ? i_aclk.pos() : i_aclk.neg());
    }

private:
    struct breq *curr_trans = NULL;
    uint16_t widx, curr_data_len_left = 0;

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
        sc_lv<TDATA_WIDTH> tmp_o_tdata;
        sc_lv<TDATA_WIDTH/8> tmp_o_tstrb;

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

    void drive_xs()
    {
        sc_lv<TDATA_WIDTH> allxs ('X');
        o_tdata = allxs;
        o_tdata = allxs;
        o_tstrb = allxs;
        o_tkeep = allxs;
        o_tvalid = sc_logic_0;
        o_tlast = sc_logic_0;
    }

    /**
     * @brief Function to add signals to waveforms
     * @param file Pointer to waveform file
     */
    void add_to_wave(sc_trace_file *file)
    {
        sc_trace(file, i_aclk,     i_aclk.name());
        sc_trace(file, i_areset_n, i_areset_n.name());
        sc_trace(file, o_tvalid,   o_tvalid.name());
        sc_trace(file, i_tready,   i_tready.name());
        sc_trace(file, o_tdata,    o_tdata.name());
        sc_trace(file, o_tstrb,    o_tstrb.name());
        sc_trace(file, o_tkeep,    o_tkeep.name());
        sc_trace(file, o_tlast,    o_tlast.name());
        sc_trace(file, o_tid,      o_tid.name());
        sc_trace(file, o_tdest,    o_tdest.name());
        sc_trace(file, o_tuser,    o_tuser.name());
        sc_trace(file, o_twakeup,  o_twakeup.name());
    }
};

} // namespace axis

#endif // __AXIS_MASTER_H__

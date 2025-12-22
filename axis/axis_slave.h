#ifndef __AXIS_SLAVE_H__
#define __AXIS_SLAVE_H__

#include "systemc.h"
#include "common.h"
#include "axis_if.h"
#include "axis.h"

using namespace std;
using namespace sc_core;

struct data_list {
    uint8_t *data;
    struct data_list *nxt;
};

namespace axis {

template <int TDATA_WIDTH, int TID_WIDTH, int TDEST_WIDTH, int TUSER_DEST>
class slave : sc_module
{
public:
    sc_in<bool>                  i_aclk;
    sc_in<sc_logic>              i_areset_n;
    sc_in<sc_logic>              i_tvalid;
    sc_out<sc_logic>             o_tready;
    sc_in<sc_lv<TDATA_WIDTH> >   i_tdata;
    sc_in<sc_lv<TDATA_WIDTH/8> > i_tstrb;
    sc_in<sc_lv<TDATA_WIDTH/8> > i_tkeep;
    sc_in<sc_logic>              i_tlast;
    sc_in<sc_lv<TID_WIDTH> >     i_tid;
    sc_in<sc_lv<TDEST_WIDTH> >   i_tdest;
    sc_in<sc_lv<TUSER_DEST> >    i_tuser;
    sc_in<sc_logic>              i_twakeup;

    struct bobj *obj;

    SC_HAS_PROCESS(slave);

    /**
     * @brief Constructor for the apb slave class
     * @param name The name of the module
     * @param file Pointer to waveform file
     */
    explicit slave(sc_module_name name, sc_trace_file *file = NULL,
                    struct bobj *parent_obj = NULL, uint16_t num_ot = 0,
                    int (*uinit_ptr)(void **mem) = NULL) :
    sc_module(name), i_aclk("i_aclk"), i_areset_n("i_areset_n"), i_tvalid("i_tvalid"),
    o_tready("o_tready"), i_tdata("i_tdata"), i_tstrb("i_tstrb"), i_tkeep("i_tkeep"),
    i_tlast("i_tlast"), i_tid("i_tid"), i_tdest("i_tdest"), i_tuser("i_tuser"),
    i_twakeup("i_twakeup")
    {
        if(file != NULL)
            this->add_to_wave(file);

        default_init(&obj, parent_obj, -1);
        obj->proto = AXIS;
        obj->type = SLAVE;

        if(uinit_ptr)
            uinit_ptr(&obj->mem);

        dl = NULL;
        dlh = NULL;
        dlt = NULL;
        dlen = 0;
        req_addr = 0;

        SC_METHOD(slave_seq);
        sensitive << i_aclk.pos() << i_areset_n.neg();
        dont_initialize();

        SC_METHOD(slave_comb);
        sensitive << state;
    }

    ~slave()
    {
        default_destroy(obj);
    }

    slave& operator () (axis_if
               <TDATA_WIDTH, TID_WIDTH, TDEST_WIDTH, TUSER_DEST>& axisif)
    {
        i_aclk    (axisif.clk);
        i_areset_n(axisif.rst_n);
        i_tvalid  (axisif.tvalid);
        o_tready  (axisif.tready);
        i_tdata   (axisif.tdata);
        i_tstrb   (axisif.tstrb);
        i_tkeep   (axisif.tkeep);
        i_tlast   (axisif.tlast);
        i_tid     (axisif.tid);
        i_tdest   (axisif.tdest);
        i_tuser   (axisif.tuser);
        i_twakeup (axisif.twakeup);
        return *this;
    }

    slave& operator () (axis_if_s
               <TDATA_WIDTH, TID_WIDTH, TDEST_WIDTH, TUSER_DEST>& axisif)
    {
        i_aclk    (axisif.i_clk);
        i_areset_n(axisif.i_rst_n);
        i_tvalid  (axisif.i_tvalid);
        o_tready  (axisif.o_tready);
        i_tdata   (axisif.i_tdata);
        i_tstrb   (axisif.i_tstrb);
        i_tkeep   (axisif.i_tkeep);
        i_tlast   (axisif.i_tlast);
        i_tid     (axisif.i_tid);
        i_tdest   (axisif.i_tdest);
        i_tuser   (axisif.i_tuser);
        i_twakeup (axisif.i_twakeup);
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
        sc_lv<8> ltmp_data;
        sc_lv<TDATA_WIDTH> tmp_i_tdata = i_tdata;
        sc_lv<TDATA_WIDTH/8> tmp_i_tstrb = i_tstrb;

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

    /**
     * @brief Function to add signals to waveforms
     * @param file Pointer to waveform file
     */
    void add_to_wave(sc_trace_file *file)
    {
        sc_trace(file, i_aclk,     i_aclk.name());
        sc_trace(file, i_areset_n, i_areset_n.name());
        sc_trace(file, i_tvalid,   i_tvalid.name());
        sc_trace(file, o_tready,   o_tready.name());
        sc_trace(file, i_tdata,    i_tdata.name());
        sc_trace(file, i_tstrb,    i_tstrb.name());
        sc_trace(file, i_tkeep,    i_tkeep.name());
        sc_trace(file, i_tlast,    i_tlast.name());
        sc_trace(file, i_tid,      i_tid.name());
        sc_trace(file, i_tdest,    i_tdest.name());
        sc_trace(file, i_tuser,    i_tuser.name());
        sc_trace(file, i_twakeup,  i_twakeup.name());
    }
};

} // namespace axis

#endif // __AXIS_SLAVE_H__

#ifndef __axis_if_h__
#define __axis_if_h__

using namespace sc_core;
using namespace sc_dt;

template <int TDATA_WIDTH,
          int TID_WIDTH,
          int TDEST_WIDTH,
          int TUSER_WIDTH>
class axis_if : public sc_module
{
public:
    sc_in<bool>                      clk;
    sc_in<sc_logic>                  rst_n;
    sc_signal<sc_logic>              tvalid;
    sc_signal<sc_logic>              tready;
    sc_signal<sc_lv<TDATA_WIDTH> >   tdata;
    sc_signal<sc_lv<TDATA_WIDTH/8> > tstrb;
    sc_signal<sc_lv<TDATA_WIDTH/8> > tkeep;
    sc_signal<sc_logic>              tlast;
    sc_signal<sc_lv<TID_WIDTH> >     tid;
    sc_signal<sc_lv<TDEST_WIDTH> >   tdest;
    sc_signal<sc_lv<TUSER_WIDTH> >   tuser;
    sc_signal<sc_logic>              twakeup;

    SC_HAS_PROCESS(axis_if);

    explicit axis_if(sc_module_name name, sc_trace_file *file = NULL,
    sc_clock& tclk = NULL, sc_signal<sc_logic>& trst_n = NULL) :
    sc_module(name), clk("clk"), rst_n("rst_n"), tvalid("tvalid"),
    tready("tready"), tdata("tdata"), tstrb("tstrb"), tkeep("tkeep"),
    tlast("tlast"), tid("tid"), tdest("tdest"), tuser("tuser"),
    twakeup("twakeup")
    {
        clk(tclk);
        rst_n(trst_n);
        if(file != NULL)
            this->add_to_wave(file);
    }

    explicit axis_if(sc_module_name name, sc_trace_file *file = NULL,
    sc_in<bool>& tclk = NULL, sc_in<sc_logic>& trst_n = NULL) :
    sc_module(name), clk("clk"), rst_n("rst_n"), tvalid("tvalid"),
    tready("tready"), tdata("tdata"), tstrb("tstrb"), tkeep("tkeep"),
    tlast("tlast"), tid("tid"), tdest("tdest"), tuser("tuser"),
    twakeup("twakeup")
    {
        clk(tclk);
        rst_n(trst_n);
        if(file != NULL)
            this->add_to_wave(file);
    }

    void add_to_wave(sc_trace_file *file)
    {
        sc_trace(file, clk,     clk.name());
        sc_trace(file, rst_n,   rst_n.name());
        sc_trace(file, tvalid,  tvalid.name());
        sc_trace(file, tready,  tready.name());
        sc_trace(file, tdata,   tdata.name());
        sc_trace(file, tstrb,   tstrb.name());
        sc_trace(file, tkeep,   tkeep.name());
        sc_trace(file, tlast,   tlast.name());
        sc_trace(file, tid,     tid.name());
        sc_trace(file, tdest,   tdest.name());
        sc_trace(file, tuser,   tuser.name());
        sc_trace(file, twakeup, twakeup.name());
    }
};

template <int TDATA_WIDTH,
          int TID_WIDTH,
          int TDEST_WIDTH,
          int TUSER_WIDTH>
class axis_if_m : public sc_module
{
public:
    sc_in<bool>                   i_clk;
    sc_in<sc_logic>               i_rst_n;
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

    SC_HAS_PROCESS(axis_if_m);

    explicit axis_if_m(sc_module_name name, sc_trace_file *file = NULL,
    sc_clock& tclk = NULL, sc_signal<sc_logic>& trst_n = NULL) :
    sc_module(name), i_clk("i_clk"), i_rst_n("i_rst_n"), o_tvalid("o_tvalid"),
    i_tready("i_tready"), o_tdata("o_tdata"), o_tstrb("o_tstrb"), o_tkeep("o_tkeep"),
    o_tlast("o_tlast"), o_tid("o_tid"), o_tdest("o_tdest"), o_tuser("o_tuser"),
    o_twakeup("o_twakeup")
    {
        if(file != NULL)
            this->add_to_wave(file);
    }

    explicit axis_if_m(sc_module_name name, sc_trace_file *file = NULL,
    sc_in<bool>& tclk = NULL, sc_in<sc_logic>& trst_n = NULL) :
    sc_module(name), i_clk("i_clk"), i_rst_n("i_rst_n"), o_tvalid("o_tvalid"),
    i_tready("i_tready"), o_tdata("o_tdata"), o_tstrb("o_tstrb"), o_tkeep("o_tkeep"),
    o_tlast("o_tlast"), o_tid("o_tid"), o_tdest("o_tdest"), o_tuser("o_tuser"),
    o_twakeup("o_twakeup")
    {
        if(file != NULL)
            this->add_to_wave(file);
    }

    axis_if_m<TDATA_WIDTH, TID_WIDTH, TDEST_WIDTH, TUSER_WIDTH>& operator () 
			(axis_if<TDATA_WIDTH, TID_WIDTH, TDEST_WIDTH, TUSER_WIDTH>& axisif)
    {
        i_clk     (axisif.clk);
        i_rst_n   (axisif.rst_n);
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

    void add_to_wave(sc_trace_file *file)
    {
        sc_trace(file, i_clk,     i_clk.name());
        sc_trace(file, i_rst_n,   i_rst_n.name());
        sc_trace(file, o_tvalid,  o_tvalid.name());
        sc_trace(file, i_tready,  i_tready.name());
        sc_trace(file, o_tdata,   o_tdata.name());
        sc_trace(file, o_tstrb,   o_tstrb.name());
        sc_trace(file, o_tkeep,   o_tkeep.name());
        sc_trace(file, o_tlast,   o_tlast.name());
        sc_trace(file, o_tid,     o_tid.name());
        sc_trace(file, o_tdest,   o_tdest.name());
        sc_trace(file, o_tuser,   o_tuser.name());
        sc_trace(file, o_twakeup, o_twakeup.name());
    }
};

template <int TDATA_WIDTH,
          int TID_WIDTH,
          int TDEST_WIDTH,
          int TUSER_WIDTH>
class axis_if_s : public sc_module
{
public:
    sc_in<bool>                  i_clk;
    sc_in<sc_logic>              i_rst_n;
    sc_in<sc_logic>              i_tvalid;
    sc_out<sc_logic>             o_tready;
    sc_in<sc_lv<TDATA_WIDTH> >   i_tdata;
    sc_in<sc_lv<TDATA_WIDTH/8> > i_tstrb;
    sc_in<sc_lv<TDATA_WIDTH/8> > i_tkeep;
    sc_in<sc_logic>              i_tlast;
    sc_in<sc_lv<TID_WIDTH> >     i_tid;
    sc_in<sc_lv<TDEST_WIDTH> >   i_tdest;
    sc_in<sc_lv<TUSER_WIDTH> >   i_tuser;
    sc_in<sc_logic>              i_twakeup;

    SC_HAS_PROCESS(axis_if_s);

    explicit axis_if_s(sc_module_name name, sc_trace_file *file = NULL,
    sc_clock& tclk = NULL, sc_signal<sc_logic>& trst_n = NULL) :
    sc_module(name), i_clk("i_clk"), i_rst_n("i_rst_n"), i_tvalid("i_tvalid"),
    o_tready("o_tready"), i_tdata("i_tdata"), i_tstrb("i_tstrb"), i_tkeep("i_tkeep"),
    i_tlast("i_tlast"), i_tid("i_tid"), i_tdest("i_tdest"), i_tuser("i_tuser"),
    i_twakeup("i_twakeup")
    {
        if(file != NULL)
            this->add_to_wave(file);
    }

    explicit axis_if_s(sc_module_name name, sc_trace_file *file = NULL,
    sc_in<bool>& tclk = NULL, sc_in<sc_logic>& trst_n = NULL) :
    sc_module(name), i_clk("i_clk"), i_rst_n("i_rst_n"), i_tvalid("i_tvalid"),
    o_tready("o_tready"), i_tdata("i_tdata"), i_tstrb("i_tstrb"), i_tkeep("i_tkeep"),
    i_tlast("i_tlast"), i_tid("i_tid"), i_tdest("i_tdest"), i_tuser("i_tuser"),
    i_twakeup("i_twakeup")
    {
        if(file != NULL)
            this->add_to_wave(file);
    }

    axis_if_s<TDATA_WIDTH, TID_WIDTH, TDEST_WIDTH, TUSER_WIDTH>& operator () 
			(axis_if<TDATA_WIDTH, TID_WIDTH, TDEST_WIDTH, TUSER_WIDTH>& axisif)
    {
        i_clk     (axisif.clk);
        i_rst_n   (axisif.rst_n);
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

    void add_to_wave(sc_trace_file *file)
    {
        sc_trace(file, i_clk,     i_clk.name());
        sc_trace(file, i_rst_n,   i_rst_n.name());
        sc_trace(file, i_tvalid,  i_tvalid.name());
        sc_trace(file, o_tready,  o_tready.name());
        sc_trace(file, i_tdata,   i_tdata.name());
        sc_trace(file, i_tstrb,   i_tstrb.name());
        sc_trace(file, i_tkeep,   i_tkeep.name());
        sc_trace(file, i_tlast,   i_tlast.name());
        sc_trace(file, i_tid,     i_tid.name());
        sc_trace(file, i_tdest,   i_tdest.name());
        sc_trace(file, i_tuser,   i_tuser.name());
        sc_trace(file, i_twakeup, i_twakeup.name());
    }
};

#endif /* __axis_if_h__ */

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
    sc_signal<bool>		             tvalid;
    sc_signal<bool>	                 tready;
    sc_signal<sc_bv<TDATA_WIDTH> >   tdata;
    sc_signal<sc_bv<TDATA_WIDTH/8> > tstrb;
    sc_signal<sc_bv<TDATA_WIDTH/8> > tkeep;
    sc_signal<bool>		             tlast;
    sc_signal<sc_bv<TID_WIDTH> >     tid;
    sc_signal<sc_bv<TDEST_WIDTH> >   tdest;
    sc_signal<sc_bv<TUSER_WIDTH> >   tuser;
    sc_signal<bool>		             twakeup;

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
    sc_in<bool>                   clk;
    sc_in<sc_logic>               rst_n;
    sc_out<bool>    	          tvalid;
    sc_in<bool> 	              tready;
    sc_out<sc_bv<TDATA_WIDTH> >   tdata;
    sc_out<sc_bv<TDATA_WIDTH/8> > tstrb;
    sc_out<sc_bv<TDATA_WIDTH/8> > tkeep;
    sc_out<bool>	              tlast;
    sc_out<sc_bv<TID_WIDTH> >     tid;
    sc_out<sc_bv<TDEST_WIDTH> >   tdest;
    sc_out<sc_bv<TUSER_WIDTH> >   tuser;
    sc_out<bool>	              twakeup;

    SC_HAS_PROCESS(axis_if_m);

    explicit axis_if_m(sc_module_name name, sc_trace_file *file = NULL,
    sc_clock& tclk = NULL, sc_signal<sc_logic>& trst_n = NULL) :
    sc_module(name), clk("clk"), rst_n("rst_n"), tvalid("tvalid"),
    tready("tready"), tdata("tdata"), tstrb("tstrb"), tkeep("tkeep"),
    tlast("tlast"), tid("tid"), tdest("tdest"), tuser("tuser"),
    twakeup("twakeup")
    {
        if(file != NULL)
            this->add_to_wave(file);
    }

    explicit axis_if_m(sc_module_name name, sc_trace_file *file = NULL,
    sc_in<bool>& tclk = NULL, sc_in<sc_logic>& trst_n = NULL) :
    sc_module(name), clk("clk"), rst_n("rst_n"), tvalid("tvalid"),
    tready("tready"), tdata("tdata"), tstrb("tstrb"), tkeep("tkeep"),
    tlast("tlast"), tid("tid"), tdest("tdest"), tuser("tuser"),
    twakeup("twakeup")
    {
        if(file != NULL)
            this->add_to_wave(file);
    }

    axis_if_m<TDATA_WIDTH, TID_WIDTH, TDEST_WIDTH, TUSER_WIDTH>& operator () 
			(axis_if<TDATA_WIDTH, TID_WIDTH, TDEST_WIDTH, TUSER_WIDTH>& axisif)
    {
        clk     (axisif.clk);
        rst_n   (axisif.rst_n);
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
class axis_if_s : public sc_module
{
public:
    sc_in<bool>                  clk;
    sc_in<sc_logic>              rst_n;
    sc_in<bool>	 	             tvalid;
    sc_out<bool>	           	 tready;
    sc_in<sc_bv<TDATA_WIDTH> >   tdata;
    sc_in<sc_bv<TDATA_WIDTH/8> > tstrb;
    sc_in<sc_bv<TDATA_WIDTH/8> > tkeep;
    sc_in<bool>		             tlast;
    sc_in<sc_bv<TID_WIDTH> >     tid;
    sc_in<sc_bv<TDEST_WIDTH> >   tdest;
    sc_in<sc_bv<TUSER_WIDTH> >   tuser;
    sc_in<bool>		             twakeup;

    SC_HAS_PROCESS(axis_if_s);

    explicit axis_if_s(sc_module_name name, sc_trace_file *file = NULL,
    sc_clock& tclk = NULL, sc_signal<sc_logic>& trst_n = NULL) :
    sc_module(name), clk("clk"), rst_n("rst_n"), tvalid("tvalid"),
    tready("tready"), tdata("tdata"), tstrb("tstrb"), tkeep("tkeep"),
    tlast("tlast"), tid("tid"), tdest("tdest"), tuser("tuser"),
    twakeup("twakeup")
    {
        if(file != NULL)
            this->add_to_wave(file);
    }

    explicit axis_if_s(sc_module_name name, sc_trace_file *file = NULL,
    sc_in<bool>& tclk = NULL, sc_in<sc_logic>& trst_n = NULL) :
    sc_module(name), clk("clk"), rst_n("rst_n"), tvalid("tvalid"),
    tready("tready"), tdata("tdata"), tstrb("tstrb"), tkeep("tkeep"),
    tlast("tlast"), tid("tid"), tdest("tdest"), tuser("tuser"),
    twakeup("twakeup")
    {
        if(file != NULL)
            this->add_to_wave(file);
    }

    axis_if_s<TDATA_WIDTH, TID_WIDTH, TDEST_WIDTH, TUSER_WIDTH>& operator () 
			(axis_if<TDATA_WIDTH, TID_WIDTH, TDEST_WIDTH, TUSER_WIDTH>& axisif)
    {
        clk     (axisif.clk);
        rst_n   (axisif.rst_n);
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

#endif /* __axis_if_h__ */

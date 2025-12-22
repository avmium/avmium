#ifndef __apb_if_h__
#define __apb_if_h__

using namespace sc_core;
using namespace sc_dt;

template <int ADDR_WIDTH,
          int DATA_WIDTH,
          int USER_REQ_WIDTH,
          int USER_DATA_WIDTH,
          int USER_RESP_WIDTH>
class apb_if : public sc_module
{
public:
    sc_in<bool>                         clk;
    sc_in<sc_logic>                     rst_n;
    sc_signal<sc_lv<ADDR_WIDTH> >       paddr;
    sc_signal<sc_lv<3> >                pprot;
    sc_signal<sc_logic>                 pnse;
    sc_signal<sc_logic>                 psel_x;
    sc_signal<sc_logic>                 penable;
    sc_signal<sc_logic>                 pwrite;
    sc_signal<sc_bv<DATA_WIDTH> >       pwdata;
    sc_signal<sc_lv<DATA_WIDTH/8> >     pstrb;
    sc_signal<sc_logic>                 pready;
    sc_signal<sc_bv<DATA_WIDTH> >       prdata;
    sc_signal<sc_logic>                 pslverr;
    sc_signal<sc_logic>                 pwakeup;
    sc_signal<sc_lv<USER_REQ_WIDTH> >   pauser;
    sc_signal<sc_lv<USER_DATA_WIDTH> >  pwuser;
    sc_signal<sc_lv<USER_DATA_WIDTH> >  pruser;
    sc_signal<sc_lv<USER_RESP_WIDTH> >  pbuser;

    SC_HAS_PROCESS(apb_if);

    explicit apb_if(sc_module_name name, sc_trace_file *file = NULL,
    sc_clock& tclk = NULL, sc_signal<sc_logic>& trst_n = NULL) :
    sc_module(name), clk("clk"), rst_n("rst_n"), paddr("paddr"),
    pprot("pprot"), pnse("pnse"), psel_x("psel_x"), penable("penable"),
    pwrite("pwrite"), pwdata("pwdata"), pstrb("pstrb"), pready("pready"),
    prdata("prdata"), pslverr("pslverr"), pwakeup("pwakeup"),
    pauser("pauser"), pwuser("pwuser"), pruser("pruser"), pbuser("pbuser")
    {
        clk(tclk);
        rst_n(trst_n);
        if(file != NULL)
            this->add_to_wave(file);
    }

    explicit apb_if(sc_module_name name, sc_trace_file *file = NULL,
    sc_in<bool>& tclk = NULL, sc_in<sc_logic>& trst_n = NULL) :
    sc_module(name), clk("clk"), rst_n("rst_n"), paddr("paddr"),
    pprot("pprot"), pnse("pnse"), psel_x("psel_x"), penable("penable"),
    pwrite("pwrite"), pwdata("pwdata"), pstrb("pstrb"), pready("pready"),
    prdata("prdata"), pslverr("pslverr"), pwakeup("pwakeup"),
    pauser("pauser"), pwuser("pwuser"), pruser("pruser"), pbuser("pbuser")
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
        sc_trace(file, paddr,   paddr.name());
        sc_trace(file, pprot,   pprot.name());
        sc_trace(file, pnse,    pnse.name());
        sc_trace(file, psel_x,  psel_x.name());
        sc_trace(file, penable, penable.name());
        sc_trace(file, pwrite,  pwrite.name());
        sc_trace(file, pwdata,  pwdata.name());
        sc_trace(file, pstrb,   pstrb.name());
        sc_trace(file, pready,  pready.name());
        sc_trace(file, prdata,  prdata.name());
        sc_trace(file, pslverr, pslverr.name());
        sc_trace(file, pwakeup, pwakeup.name());
        sc_trace(file, pauser,  pauser.name());
        sc_trace(file, pwuser,  pwuser.name());
        sc_trace(file, pruser,  pruser.name());
        sc_trace(file, pbuser,  pbuser.name());
    }
};

template <int ADDR_WIDTH,
          int DATA_WIDTH,
          int USER_REQ_WIDTH,
          int USER_DATA_WIDTH,
          int USER_RESP_WIDTH>
class apb_if_m : public sc_module
{
public:
    sc_in<bool>                     clk;
    sc_in<sc_logic>                 rst_n;
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

    SC_HAS_PROCESS(apb_if_m);

    explicit apb_if_m(sc_module_name name, sc_trace_file *file = NULL,
    sc_clock& tclk = NULL, sc_signal<sc_logic>& trst_n = NULL) :
    sc_module(name), clk("clk"), rst_n("rst_n"), paddr("paddr"),
    pprot("pprot"), pnse("pnse"), psel_x("psel_x"),
	penable("penable"), pwrite("pwrite"), pwdata("pwdata"),
	pstrb("pstrb"), pready("pready"), prdata("prdata"),
	pslverr("pslverr"), pwakeup("pwakeup"), pauser("pauser"),
	pwuser("pwuser"), pruser("pruser"), pbuser("pbuser")
    {
        if(file != NULL)
            this->add_to_wave(file);
    }

    explicit apb_if_m(sc_module_name name, sc_trace_file *file = NULL,
    sc_in<bool>& tclk = NULL, sc_in<sc_logic>& trst_n = NULL) :
    sc_module(name), clk("clk"), rst_n("rst_n"), paddr("paddr"),
    pprot("pprot"), pnse("pnse"), psel_x("psel_x"),
	penable("penable"), pwrite("pwrite"), pwdata("pwdata"),
	pstrb("pstrb"), pready("pready"), prdata("prdata"),
	pslverr("pslverr"), pwakeup("pwakeup"), pauser("pauser"),
	pwuser("pwuser"), pruser("pruser"), pbuser("pbuser")
    {
        if(file != NULL)
            this->add_to_wave(file);
    }

    apb_if_m<ADDR_WIDTH, DATA_WIDTH, USER_REQ_WIDTH, USER_DATA_WIDTH,
           USER_RESP_WIDTH>& operator () (apb_if<ADDR_WIDTH, DATA_WIDTH,
           USER_REQ_WIDTH, USER_DATA_WIDTH, USER_RESP_WIDTH>& apbif)
    {
        clk     (apbif.clk);
        rst_n   (apbif.rst_n);
        paddr   (apbif.paddr);
        pprot   (apbif.pprot);
        pnse    (apbif.pnse);
        psel_x  (apbif.psel_x);
        penable (apbif.penable);
        pwrite  (apbif.pwrite);
        pwdata  (apbif.pwdata);
        pstrb   (apbif.pstrb);
        pready  (apbif.pready);
        prdata  (apbif.prdata);
        pslverr (apbif.pslverr);
        pwakeup (apbif.pwakeup);
        pauser  (apbif.pauser);
        pwuser  (apbif.pwuser);
        pruser  (apbif.pruser);
        pbuser  (apbif.pbuser);
        return *this;
    }

    void add_to_wave(sc_trace_file *file)
    {
        sc_trace(file, clk,     clk.name());
        sc_trace(file, rst_n,   rst_n.name());
        sc_trace(file, paddr,   paddr.name());
        sc_trace(file, pprot,   pprot.name());
        sc_trace(file, pnse,    pnse.name());
        sc_trace(file, psel_x,  psel_x.name());
        sc_trace(file, penable, penable.name());
        sc_trace(file, pwrite,  pwrite.name());
        sc_trace(file, pwdata,  pwdata.name());
        sc_trace(file, pstrb,   pstrb.name());
        sc_trace(file, pready,  pready.name());
        sc_trace(file, prdata,  prdata.name());
        sc_trace(file, pslverr, pslverr.name());
        sc_trace(file, pwakeup, pwakeup.name());
        sc_trace(file, pauser,  pauser.name());
        sc_trace(file, pwuser,  pwuser.name());
        sc_trace(file, pruser,  pruser.name());
        sc_trace(file, pbuser,  pbuser.name());
    }
};

template <int ADDR_WIDTH,
          int DATA_WIDTH,
          int USER_REQ_WIDTH,
          int USER_DATA_WIDTH,
          int USER_RESP_WIDTH>
class apb_if_s : public sc_module
{
public:
    sc_in<bool>                     clk;
    sc_in<sc_logic>                 rst_n;
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

    SC_HAS_PROCESS(apb_if_s);

    explicit apb_if_s(sc_module_name name, sc_trace_file *file = NULL,
    sc_clock& tclk = NULL, sc_signal<sc_logic>& trst_n = NULL) :
    sc_module(name), clk("clk"), rst_n("rst_n"), paddr("paddr"),
    pprot("pprot"), pnse("pnse"), psel("psel"),
	penable("penable"), pwrite("pwrite"), pwdata("pwdata"),
	pstrb("pstrb"), pready("pready"), prdata("prdata"),
	pslverr("pslverr"), pwakeup("pwakeup"), pauser("pauser"),
	pwuser("pwuser"), pruser("pruser"), pbuser("pbuser")
    {
        if(file != NULL)
            this->add_to_wave(file);
    }

    explicit apb_if_s(sc_module_name name, sc_trace_file *file = NULL,
    sc_in<bool>& tclk = NULL, sc_in<sc_logic>& trst_n = NULL) :
    sc_module(name), clk("clk"), rst_n("rst_n"), paddr("paddr"),
    pprot("pprot"), pnse("pnse"), psel("psel"),
	penable("penable"), pwrite("pwrite"), pwdata("pwdata"),
	pstrb("pstrb"), pready("pready"), prdata("prdata"),
	pslverr("pslverr"), pwakeup("pwakeup"), pauser("pauser"),
	pwuser("pwuser"), pruser("pruser"), pbuser("pbuser")
    {
        if(file != NULL)
            this->add_to_wave(file);
    }

    apb_if_s<ADDR_WIDTH, DATA_WIDTH, USER_REQ_WIDTH, USER_DATA_WIDTH,
           USER_RESP_WIDTH>& operator () (apb_if<ADDR_WIDTH, DATA_WIDTH,
           USER_REQ_WIDTH, USER_DATA_WIDTH, USER_RESP_WIDTH>& apbif)
    {
        clk     (apbif.clk);
        rst_n   (apbif.rst_n);
        paddr   (apbif.paddr);
        pprot   (apbif.pprot);
        pnse    (apbif.pnse);
        psel    (apbif.psel_x);
        penable (apbif.penable);
        pwrite  (apbif.pwrite);
        pwdata  (apbif.pwdata);
        pstrb   (apbif.pstrb);
        pready  (apbif.pready);
        prdata  (apbif.prdata);
        pslverr (apbif.pslverr);
        pwakeup (apbif.pwakeup);
        pauser  (apbif.pauser);
        pwuser  (apbif.pwuser);
        pruser  (apbif.pruser);
        pbuser  (apbif.pbuser);
        return *this;
    }

    void add_to_wave(sc_trace_file *file)
    {
        sc_trace(file, clk,     clk.name());
        sc_trace(file, rst_n,   rst_n.name());
        sc_trace(file, paddr,   paddr.name());
        sc_trace(file, pprot,   pprot.name());
        sc_trace(file, pnse,    pnse.name());
        sc_trace(file, psel,    psel.name());
        sc_trace(file, penable, penable.name());
        sc_trace(file, pwrite,  pwrite.name());
        sc_trace(file, pwdata,  pwdata.name());
        sc_trace(file, pstrb,   pstrb.name());
        sc_trace(file, pready,  pready.name());
        sc_trace(file, prdata,  prdata.name());
        sc_trace(file, pslverr, pslverr.name());
        sc_trace(file, pwakeup, pwakeup.name());
        sc_trace(file, pauser,  pauser.name());
        sc_trace(file, pwuser,  pwuser.name());
        sc_trace(file, pruser,  pruser.name());
        sc_trace(file, pbuser,  pbuser.name());
    }
};

#endif /* __apb_if_h__ */

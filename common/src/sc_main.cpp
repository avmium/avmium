#include "systemc.h"

#define TO_STRING(x) #x
#define STRINGIFY(x) TO_STRING(x)
#define CONCAT_TOKENS(a, b) a##b
#define MAKE_IDENTIFIER(a, b) CONCAT_TOKENS(a, b)

#define INCLUDE_FILE(x) TO_STRING(x_top.h)
#define TOP_MOD STRINGIFY(MAKE_IDENTIFIER(sc_, TOP))
#define INCLUDE STRINGIFY(MAKE_IDENTIFIER(TOP, _top.h))

#include <csignal>
#include INCLUDE

sc_trace_file *file = NULL;

void signal_handler(int signal) {
    std::cout << "\n[SystemC] Caught Ctrl+C (SIGINT), stopping simulation..." ;
    sc_stop();  // Gracefully stop simulation
}

int sc_main(int argc, char *argv[])
{
	signal(SIGINT, signal_handler);
#if WAVEDUMP
    file = sc_create_vcd_trace_file("sc_waveform");
#endif

    sc_clock            clk("clk", 1, SC_NS);
    sc_signal<sc_logic> rst_n("rst_n");

#if WAVEDUMP
    sc_trace(file, clk,   clk.name());
    sc_trace(file, rst_n, rst_n.name());
#endif
    rst_n = sc_logic_0;

    MAKE_IDENTIFIER(TOP, _top) sc_TOP_top(TOP_MOD, file);

    sc_TOP_top.clk   ( clk );
    sc_TOP_top.rst_n ( rst_n );

#if SIM_CLK_CYCLES
    for(int i = 0; i < SIM_CLK_CYCLES; i++) {
        if(i == 1)
            rst_n = sc_logic_1;
        sc_start(1, SC_NS);
    }
#else
	sc_start(1, SC_NS);
	rst_n = sc_logic_1;
	sc_start();
#endif

#if WAVEDUMP
    sc_close_vcd_trace_file(file);
#endif

    return 0;
}

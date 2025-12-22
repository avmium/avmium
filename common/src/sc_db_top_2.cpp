#include "systemc.h"
#include "common.h"
#include "axis_master.h"
#include "axis_slave.h"
#include "apb_mst.h"
#include "apb_slv.h"
#include "build/top.h"

int sc_main(int argc, char *argv[])
{
    struct bobj obj = {0};

    sc_trace_file *file = sc_create_vcd_trace_file("waveform");

    sc_clock            clk("clk", 1, SC_NS);
    sc_signal<sc_logic> rst_n("rst_n");

    sc_trace(file, clk,   clk.name());
    sc_trace(file, rst_n, rst_n.name());
    rst_n = sc_logic_0;

    axis_if<32, 8, 8, 8> axisifs("axisifs", file, clk, rst_n);
    apb_if<32, 32, 32, 32, 32> apbifm("apbifm", file, clk, rst_n);
    apb_if<32, 32, 32, 32, 32> apbifs("apbifs", file, clk, rst_n);

    apb::master<32, 32, 32, 32, 32> apbm ("apb_master", file, &obj);
    axis::slave<32, 8, 8, 8> axiss ("axis_slave", file, &obj);
	apb::slave<32, 32, 32, 32, 32> apbs ("apb_slave", file, &obj);

    db_top top("db_top", file);

    apbm( apbifm );
    axiss( axisifs );
	apbs( apbifs );

    top.i_clk   ( clk );
    top.i_rst_n ( rst_n );
    top.apbifs ( apbifm );
    top.axisifm ( axisifs );
	top.apbifm  ( apbifs );

    bind_request_acceptor(&obj, NULL, NULL, 0x0, 0xffffffffffffffff);

    uint16_t ridx;
    uint16_t size = 4;
    uint8_t *data = (uint8_t *)malloc(size);
    for(int i = 0; i < size; i++) {
        *(data + i) = i;
    }

    add_request(apbm.obj, 1, 0x100, data, size, NULL, &ridx);

    for(int i = 1; i <= 150; i++) {
        if(i == 2)
            rst_n = sc_logic_1;
        sc_start(1, SC_NS);
    }

    free(data);
    sc_close_vcd_trace_file(file);

    return 0;
}

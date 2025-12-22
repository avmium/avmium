// DESCRIPTION: Verilator: Verilog example module
//
// This file ONLY is placed under the Creative Commons Public Domain, for
// any use, without warranty, 2017 by Wilson Snyder.
// SPDX-License-Identifier: CC0-1.0
//======================================================================

// Include common routines
#include <verilated.h>
#include "verilated_vcd_c.h"
#include <iostream>

// Include model header, generated from Verilating "top.v"
//#define TO_STRING(x) #x
//#define STRINGIFY(x) TO_STRING(x)
//#define CONCAT_TOKENS(a, b) a##b
//#define MAKE_IDENTIFIER(a, b) CONCAT_TOKENS(a, b)
//
//#define INCLUDE_FILE(x) STRINGIFY(MAKE_IDENTIFIER(V, x).h)

#include "Vprj_top.h"
//#include INCLUDE_FILE(TOP)

int main(int argc, char** argv, char** env) {
    VerilatedContext* contextp = new VerilatedContext;

    // Prevent unused variable warnings
    if (false && env) {}

    contextp->debug(0);
    contextp->randReset(2);
    contextp->commandArgs(argc, argv);
    Verilated::traceEverOn(true);
	VerilatedVcdC* tfp = new VerilatedVcdC;

    Vprj_top* top = new Vprj_top{contextp};
	top->trace(tfp, 99);            // match your --trace-depth
	tfp->open("sv_waveform.vcd");
    //MAKE_IDENTIFIER(V, TOP)* TOP = new MAKE_IDENTIFIER(V, TOP){contextp};

    // Simulate until $finish
    while (!Verilated::gotFinish()) {
        top->eval();
		tfp->dump(Verilated::time());
        contextp->timeInc(1);
    }

    // Final model cleanup
    top->final();

	tfp->close();
    // Destroy model
    delete top;

    // Return good completion status
    return 0;
}

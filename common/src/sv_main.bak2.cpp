#include <verilated.h>
#include <verilated_vcd_c.h>
#include "Vprj_top.h"

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
	top->trace(tfp, 0);
	tfp->open("sv_waveform.vcd");

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

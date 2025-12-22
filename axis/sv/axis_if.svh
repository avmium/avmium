`ifndef __axis_if_svh__
`define __axis_if_svh__

interface axis_if
	#(int TDATA_WIDTH = 32,
	  int TID_WIDTH = 32,
      int TDEST_WIDTH = 32,
      int TUSER_WIDTH = 32)
    (
    input clk,
    input rst_n
);
	logic                       tvalid;
	logic                       tready;
	logic [TDATA_WIDTH-1:0]     tdata;
	logic [(TDATA_WIDTH/8)-1:0] tstrb;
	logic [(TDATA_WIDTH/8)-1:0] tkeep;
	logic 					    tlast;
	logic [TID_WIDTH-1:0]       tid;
	logic [TDEST_WIDTH-1:0]     tdest;
	logic [TUSER_WIDTH-1:0] 	tuser;

    modport master (
        input  	clk,
        input  	rst_n,
    	output	tvalid,
    	input	tready,
    	output	tdata,
    	output	tstrb,
    	output	tkeep,
    	output	tlast,
    	output	tid,
    	output	tdest,
    	output	tuser
    );

    modport slave (
    	input	clk,
    	input	rst_n,
    	input	tvalid,
    	output	tready,
    	input	tdata,
    	input	tstrb,
    	input	tkeep,
    	input	tlast,
    	input	tid,
    	input	tdest,
    	input	tuser
    );

endinterface : axis_if

`endif /* __axis_if_svh__ */

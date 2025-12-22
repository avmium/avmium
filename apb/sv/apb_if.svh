`ifndef __apb_if_svh__
`define __apb_if_svh__

interface apb_if
    #(parameter ADDR_WIDTH = 32,
      parameter DATA_WIDTH = 32,
      parameter USER_REQ_WIDTH = 128,
      parameter USER_DATA_WIDTH = DATA_WIDTH/2,
      parameter USER_RESP_WIDTH = 16)
    (
    input clk,
    input rst_n
);
    logic [ADDR_WIDTH-1:0]      paddr;
    logic [2:0]                 pprot;
    logic                       pnse;
    logic                       psel;
    logic                       penable;
    logic                       pwrite;
    logic [DATA_WIDTH-1:0]      pwdata;
    logic [(DATA_WIDTH/8)-1:0]  pstrb;
    logic                       pready;
    logic [DATA_WIDTH-1:0]      prdata;
    logic                       pslverr;
    logic                       pwakeup;
    logic [USER_REQ_WIDTH-1:0]  pauser;
    logic [USER_DATA_WIDTH-1:0] pwuser;
    logic [USER_DATA_WIDTH-1:0] pruser;
    logic [USER_RESP_WIDTH-1:0] pbuser;

    modport master (
        input  clk,
        input  rst_n,
        output paddr,
        output pprot,
        output pnse,
        output psel,
        output penable,
        output pwrite,
        output pwdata,
        output pstrb,
        input  pready,
        input  prdata,
        input  pslverr,
        output pwakeup,
        output pauser,
        output pwuser,
        input  pruser,
        input  pbuser
    );

    modport slave (
        input  clk,
        input  rst_n,
        input  paddr,
        input  pprot,
        input  pnse,
        input  psel,
        input  penable,
        input  pwrite,
        input  pwdata,
        input  pstrb,
        output pready,
        output prdata,
        output pslverr,
        input  pwakeup,
        input  pauser,
        input  pwuser,
        output pruser,
        output pbuser
    );

endinterface : apb_if

`endif /* __apb_if_svh__ */

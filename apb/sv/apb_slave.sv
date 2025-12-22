`ifndef __apb_slave__
`define __apb_slave__

module apb_slave (apb_if.slave slave);

	typedef enum logic[1:0] {IDLE = 2'b00, SETUP = 2'b01,
							 ACCESS = 2'b10} apb_state_t;

	import "DPI-C" function void sv_apb_process_slave_pready(
		input chandle port, 
		output byte data[],
		input shortint unsigned size,
		output byte wait_for_response, 
		output int pready
	);

	import "DPI-C" function void sv_apb_slave_process_access(
		input chandle port, 
		output byte unsigned state,
		input byte unsigned pwrite,
		input longint unsigned addr,
		inout byte data[],
		input shortint unsigned size,
		output byte unsigned pready,
		output byte unsigned pslverr,
		output byte unsigned wait_for_response
	);

	byte unsigned rw;
	shortint unsigned signal_size = slave.DATA_WIDTH/8;
	shortint unsigned size;
	shortint unsigned trid = 0;
	longint unsigned addr;
	byte unsigned data[ (slave.DATA_WIDTH/8) - 1: 0 ];
	chandle port = null;
	apb_state_t state, nxt_state;
	bit wait_for_response;

	always_ff @(negedge slave.rst_n or posedge slave.psel or negedge slave.penable) begin
        if(!slave.rst_n)
            state <= IDLE;
		else
			state <= ACCESS;
	end

	always_ff @(posedge slave.clk or slave.penable) begin
		if(wait_for_response) begin
			sv_apb_process_slave_pready(port, data, signal_size, 
				wait_for_response, slave.pready);
		end else if(port_status(port) && state == ACCESS && slave.penable
			&& !wait_for_response) begin
				size = $countones(slave.pstrb);
				if(slave.pwrite) begin
    				for (int i = 0; i < size; i++) begin
    					data[i] = slave.pwdata[i*8 +: 8];
    				end
				end
				addr = slave.paddr;
				sv_apb_slave_process_access(port, state, slave.pwrite, addr, data,
					size, slave.pready, slave.pslverr, wait_for_response);
				if(!slave.pwrite) begin
    				for (int i = 0; i < size; i++) begin
    					slave.prdata[i*8 +: 8] = data[i];
    				end
				end
		end else begin
			slave.pready = '0;
		end
	end

endmodule

`endif // __apb_slave__

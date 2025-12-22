`ifndef __apb_master__
`define __apb_master__

module apb_master (apb_if.master master);

	typedef enum logic[1:0] {IDLE = 2'b00, SETUP = 2'b01,
							 ACCESS = 2'b10} apb_state_t;

	import "DPI-C" function void master_route_resp(chandle port);
	import "DPI-C" function void sv_apb_process_pready(
		input chandle port,
		input longint unsigned addr,
		input byte unsigned data[],
		input shortint unsigned size,
		input byte unsigned rw,
		input int slverr
	);
	import "DPI-C" function longint unsigned apb_get_addr(
		input chandle port,
		input longint unsigned addr
	);

	byte unsigned rw;
	shortint unsigned signal_size = master.DATA_WIDTH/8;
	shortint unsigned size;
	shortint unsigned trid = 0;
	longint unsigned addr;
	byte unsigned data[ (master.DATA_WIDTH/8) - 1: 0 ];
	chandle port = null;
	bit wait_for_response = 0;
	apb_state_t state, nxt_state;

	always_ff @( posedge master.clk or negedge master.rst_n ) begin
		if(~master.rst_n) begin
			state <= IDLE;
		end else begin
			if(state == ACCESS) begin
				if(master.pready) begin
    				for (int i = 0; i < size; i++) begin
    					data[i] = master.prdata[i*8 +: 8];
    				end
					sv_apb_process_pready(port, addr, data, size, rw, master.pslverr);
					wait_for_response = 0;
				end else begin
					wait_for_response = 1;
				end
			end
			if(!wait_for_response) begin
				if(port_status(port) && nxt_state == IDLE) begin
					size = signal_size;
					if(sv_get_send_data(port, trid, addr, rw, data, size)) begin
						state = SETUP;
					end else begin
            			state = nxt_state;
					end
				end else begin
            		state = nxt_state;
				end
			end
		end
	end

	always_comb begin
		case (state) 
			IDLE : begin
				master.paddr	= '0;
				master.pprot  	= '0;
				master.pnse   	= '0;
				master.psel 	= '0;
				master.penable	= '0;
				master.pwrite 	= '0;
				master.pwdata 	= '0;
				master.pstrb  	= '0;
				master.pwakeup	= '0;
				master.pauser 	= '0;
				master.pwuser 	= '0;
				nxt_state 	= IDLE;
			end
			SETUP : begin
        		master.paddr 	= apb_get_addr(port, addr);
        		master.pwrite   = rw;
        		master.psel   = 1;
        		master.penable  = 0;
    			for (int i = 0; i < size; i++) begin
    			    master.pwdata[i*8 +: 8] = data[i];
    			end
				master.pstrb = (1 << size) - 1;
        		nxt_state = ACCESS;
			end
			ACCESS : begin
				master.penable	= '1;
				nxt_state 	= IDLE;
			end
		endcase
	end

	always_ff @(master.clk) begin
		master_route_resp(port);
	end

endmodule

`endif // __apb_master__

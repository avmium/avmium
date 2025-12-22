`ifndef __axis_master__
`define __axis_master__

module axis_master (axis_if.master master);

	int ret = 0;
	byte unsigned rw;
	shortint unsigned signal_size = 
		(master.TDATA_WIDTH / 8) + !!(master.TDATA_WIDTH % 8);
	shortint unsigned size;
	shortint unsigned trid = 0;
	byte data[ 
		(master.TDATA_WIDTH / 8) + !!(master.TDATA_WIDTH % 8) - 1 : 0 ];
	longint unsigned addr;
	chandle port = null;
	bit wait_for_response = 0;

	import "DPI-C" function void master_route_resp(chandle port);
	import "DPI-C" function void axis_process_resp(
		input chandle opaque,
		input shortint unsigned trid
	);

	always_ff @(posedge master.clk) begin
		if(master.rst_n == 0) begin
			idle_op();
		end else begin
			if(wait_for_response) begin
				wait_for_response = !master.tready;
			end
			if(!wait_for_response) begin
			 	if(master.tlast) begin
			 		axis_process_resp(port, trid);
			 	end
			 	if(port_status(port)) begin
					size = signal_size;
			 		ret = sv_get_send_data(port, trid, addr, rw, data, size);
			 		if(rw && ret) begin
			 			setup_op();
			 		end else begin
             			idle_op();
			 		end
			 	end else begin
             		idle_op();
			 	end
			end
		end
	end

	always_ff @(master.clk) begin
		master_route_resp(port);
	end

	task automatic idle_op();
		master.tvalid	<= '0;
		master.tlast	<= '0;
     	master.tstrb 	<= '0;
     	master.tkeep 	<= '0;
     	master.tid   	<= '0;
     	master.tdest 	<= '0;
     	master.tuser 	<= '0;
		addr			<= '0;
	endtask

	task automatic setup_op();
		master.tvalid <= '1;
		master.tlast <= (ret == 2);
		wait_for_response = (master.tready == 0);
		master.tstrb = (1 << size) - 1;
		master.tkeep = (1 << size) - 1;
    	for (int i = 0; i < size; i++) begin
    	    master.tdata[i*8 +: 8] = data[i];
    	end
	endtask

endmodule

`endif // __axis_master__

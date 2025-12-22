`ifndef __axis_slave__
`define __axis_slave__

module axis_slave (axis_if.slave slave);

	int ret = 0;
	chandle port = null;
	longint unsigned addr = 0;
	shortint unsigned size = (slave.TDATA_WIDTH / 8) + !!(slave.TDATA_WIDTH % 8);
	byte unsigned data[ (slave.TDATA_WIDTH / 8) + !!(slave.TDATA_WIDTH % 8) : 0 ];

	import "DPI-C" function int sv_axis_process_tvalid(
		input chandle opaque,
		input longint unsigned addr, 
		input byte data[], 
		input shortint unsigned size, 
		input int tlast
	);

	always_comb begin
    	for (int i = 0; i < size; i++) begin
    	     data[i] = slave.tdata[i*8 +: 8];
    	end
	end

	always_ff @( posedge slave.clk or negedge slave.rst_n ) begin
		if(!slave.rst_n) begin
			slave.tready <= '0;
		end else begin
			if(port_status(port)) begin
				if(is_que_empty(port)) begin
			 		slave.tready <= '1;
			 	end
			 	if(slave.tvalid) begin
					size = $countones(slave.tstrb);
					slave.tready = sv_axis_process_tvalid(port, addr, data, size, slave.tlast);
			 		addr <= addr + size;
			 		if(slave.tlast) begin
			 			addr <= 0;
			 		end
			 	end
			end else begin
				slave.tready <= '0;
			end
		end
	end

endmodule

`endif // __axis_slave__

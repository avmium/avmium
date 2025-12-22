module clk_gen(
	output clk,
	output rst_n
);
	reg clk_gen;
	reg rst_n_internal;
	reg counter;

	assign clk = clk_gen;
	assign rst_n = rst_n_internal;

	initial begin
		$dumpfile("waveform.vcd");
		$dumpvars;
		clk_gen = '0;
		rst_n_internal = '0;
		counter = '0;
	end

	always @(posedge clk_gen) begin
		if(counter == 2) begin
			rst_n_internal <= 2;
		end
		counter <= counter + 1;
	end

  	always #1 clk_gen = ~clk_gen;

endmodule

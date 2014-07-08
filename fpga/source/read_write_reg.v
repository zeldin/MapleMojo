module read_write_reg(
		input rst,
		input clk,
		input cs,
		input we,
		inout [7:0] regdata_out,
		input [7:0] regdata_in,
		output [7:0] reg_contents
		);

   parameter reset_val = 0;

   reg [7:0] reg_d, reg_q;

   assign regdata_out = (cs && !we? reg_q : 8'bz);
   assign reg_contents = reg_q;
   
   always @(*) begin
      reg_d = reg_q;
      if (cs && we) begin
	 reg_d = regdata_in;
      end
   end

   always @(posedge clk) begin
      if (rst) begin
	 reg_q <= reset_val;
      end else begin
	 reg_q <= reg_d;
      end
   end
   
endmodule // read_write_reg


module maple_out(
		 input rst,
		 input clk,
		 input cs_ctrl,
		 input we,
		 inout [7:0] regdata_out,
		 input [7:0] regdata_in,
		 output pin1,
		 output pin5,
		 output oe
		 );

   wire [7:0] ctrl_reg_read;
   assign ctrl_reg_read = {8'b0};

   assign regdata_out = (cs_ctrl && !we? ctrl_reg_read : 8'bz);
   
   assign pin1 = out_p1_q;
   assign pin5 = out_p5_q;
   assign oe = maple_oe_q;

   reg 	  out_p1_d, out_p1_q;
   reg 	  out_p5_d, out_p5_q;
   reg 	  maple_oe_d, maple_oe_q;

   always @(out_p1_q) begin
      out_p1_d = 1'b1;
   end
   always @(out_p5_q) begin
      out_p5_d = 1'b1;
   end
   always @(maple_oe_q) begin
      maple_oe_d = 1'b0;
   end
   
   always @(posedge clk) begin
      if (rst) begin
	 out_p1_q <= 1'b1;
	 out_p5_q <= 1'b1;
	 maple_oe_q <= 1'b0;
      end else begin
	 out_p1_q <= out_p1_d;
	 out_p5_q <= out_p5_d;
	 maple_oe_q <= maple_oe_d;
      end
   end
   
endmodule // maple_out

		 
module maple_out(
		 input rst,
		 input clk,
		 input cs_ctrl,
		 input we,
		 inout [7:0] regdata_out,
		 input [7:0] regdata_in,
		 output pin1,
		 output pin5,
		 output oe,
		 input tick
		 );

   wire [7:0] ctrl_reg_read;
   assign ctrl_reg_read = {5'b0, maple_oe_q, op_end_q, op_start_q};

   assign regdata_out = (cs_ctrl && !we? ctrl_reg_read : 8'bz);
   
   assign pin1 = out_p1_q;
   assign pin5 = out_p5_q;
   assign oe = maple_oe_q;

   reg 	  out_p1_d, out_p1_q;
   reg 	  out_p5_d, out_p5_q;
   reg 	  maple_oe_d, maple_oe_q;

   reg    op_start_d, op_start_q;
   reg    op_end_d, op_end_q;

   reg [4:0] cnt_d, cnt_q;
 	  
   always @(*) begin
      out_p1_d = out_p1_q;
      out_p5_d = out_p5_q;
      maple_oe_d = maple_oe_q;
      op_start_d = op_start_q;
      op_end_d = op_end_q;
      cnt_d = cnt_q;
      if (cs_ctrl && we && (|regdata_in[1:0])) begin
	 op_start_d = regdata_in[0];
	 op_end_d = regdata_in[1];
	 maple_oe_d = 1'b1;
	 cnt_d = 5'b0;
      end else begin
	 if (op_start_q) begin
	    // Start pattern
	    out_p1_d = (cnt_q < 3) || (cnt_q >= 26);
	    out_p5_d = (cnt_q != 6) && (cnt_q != 7) &&
		       (cnt_q != 11) && (cnt_q != 12) &&
		       (cnt_q != 16) && (cnt_q != 17) &&
		       (cnt_q != 21) && (cnt_q != 22);
	    if (tick) begin
	       if (cnt_q == 27) begin
		  op_start_d = 1'b0;
		  cnt_d = 5'b0;
	       end else begin
		  cnt_d = cnt_q + 1'b1;
	       end
	    end
	 end else if(op_end_q) begin
	    // End pattern
	    out_p1_d = (cnt_q != 6) && (cnt_q != 7) &&
		       (cnt_q != 11) && (cnt_q != 12);
	    out_p5_d = (cnt_q < 3) || (cnt_q >= 16);
	    if (tick) begin
	       if (cnt_q == 16) begin
		  op_end_d = 1'b0;
		  maple_oe_d = 1'b0;
		  cnt_d = 5'b0;
	       end else begin
		  cnt_d = cnt_q + 1'b1;
	       end
	    end
	 end else if(maple_oe_q) begin
	    // Data transmit
	    if (tick) begin
	       cnt_d = (cnt_q+1'b1)&5'b00011;
	    end
	 end
      end
   end
   
   always @(posedge clk) begin
      if (rst) begin
	 out_p1_q <= 1'b1;
	 out_p5_q <= 1'b1;
	 maple_oe_q <= 1'b0;
	 op_start_q <= 1'b0;
	 op_end_q <= 1'b0;
	 cnt_q <= 5'b0;
      end else begin
	 out_p1_q <= out_p1_d;
	 out_p5_q <= out_p5_d;
	 maple_oe_q <= maple_oe_d;
	 op_start_q <= op_start_d;
	 op_end_q <= op_end_d;
	 cnt_q <= cnt_d;
      end
   end
   
endmodule // maple_out

		 
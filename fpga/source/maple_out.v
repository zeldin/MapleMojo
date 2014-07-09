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
		 input tick,
		 input [7:0] fifo_data,
		 input data_avail,
		 output data_consume
		 );

   wire [7:0] ctrl_reg_read;
   assign ctrl_reg_read = {5'b0, maple_oe_q, op_end_q, op_start_q};

   (* keep="soft" *)
   wire [7:2] ctrl_reg_ignore;
   assign ctrl_reg_ignore = regdata_in[7:2];
   
   assign regdata_out = (cs_ctrl && !we? ctrl_reg_read : 8'bz);
   
   assign pin1 = out_p1_q;
   assign pin5 = out_p5_q;
   assign oe = maple_oe_q;

   assign data_consume = data_avail && latch_ready_q;
   
   reg 	  out_p1_d, out_p1_q;
   reg 	  out_p5_d, out_p5_q;
   reg 	  maple_oe_d, maple_oe_q;

   reg    op_start_d, op_start_q;
   reg    op_end_d, op_end_q;

   reg [4:0] cnt_d, cnt_q;

   reg [7:0] data_latch;
   reg 	     latch_ready_d, latch_ready_q;
	  
   always @(*) begin
      out_p1_d = out_p1_q;
      out_p5_d = out_p5_q;
      maple_oe_d = maple_oe_q;
      op_start_d = op_start_q;
      op_end_d = op_end_q;
      cnt_d = cnt_q;
      latch_ready_d = latch_ready_q;
      if (cs_ctrl && we && (|regdata_in[1:0])) begin
	 op_start_d = regdata_in[0];
	 op_end_d = regdata_in[1];
	 maple_oe_d = 1'b1;
	 cnt_d = 5'b0;
	 latch_ready_d = 1'b0;
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
		  latch_ready_d = 1'b1;
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
	    if (tick && !latch_ready_q) begin
	       cnt_d = cnt_q+1'b1;
	       case (cnt_q)
		 5'b00000: out_p5_d = data_latch[7];
		 5'b00100: out_p1_d = data_latch[6];
		 5'b01000: out_p5_d = data_latch[5];
		 5'b01100: out_p1_d = data_latch[4];
		 5'b10000: out_p5_d = data_latch[3];
		 5'b10100: out_p1_d = data_latch[2];
		 5'b11000: out_p5_d = data_latch[1];
		 5'b11100: out_p1_d = data_latch[0];

		 5'b00010, 5'b01010, 5'b10010, 5'b11010: out_p1_d = 1'b0;
		 5'b00011, 5'b01011, 5'b10011, 5'b11011: out_p5_d = 1'b1;
		 5'b00110, 5'b01110, 5'b10110, 5'b11110: out_p5_d = 1'b0;
		 5'b00111, 5'b01111, 5'b10111          : out_p1_d = 1'b1;

		 5'b11111: begin
		    out_p1_d = 1'b1;
		    cnt_d = 5'b0;
		    latch_ready_d = 1'b1;
		 end
	       endcase
	    end
	 end
      end
      if (data_consume) begin
	latch_ready_d = 1'b0;
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
	 latch_ready_q <= 1'b0;
      end else begin
	 out_p1_q <= out_p1_d;
	 out_p5_q <= out_p5_d;
	 maple_oe_q <= maple_oe_d;
	 op_start_q <= op_start_d;
	 op_end_q <= op_end_d;
	 cnt_q <= cnt_d;
	 latch_ready_q <= latch_ready_d;
      end // else: !if(rst)
      if (data_consume) begin
	 data_latch <= fifo_data;
      end
   end
   
endmodule // maple_out

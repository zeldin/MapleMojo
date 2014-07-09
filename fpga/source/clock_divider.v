module clock_divider(
		     input clk,
		     input rst,
		     input [7:0] divider,
		     output tick
		     );

   reg [7:0] cnt_d, cnt_q;
   reg tick_d, tick_q;

   assign tick = tick_q;
   
   always @(*) begin
      if (cnt_q == 0) begin
	 cnt_d = divider;
	 tick_d = 1'b1;
      end else begin
	 cnt_d = cnt_q - 1'b1;
	 tick_d = 1'b0;
      end
   end

   always @(posedge clk) begin
      if (rst) begin
	 cnt_q <= 0;
	 tick_q <= 0;
      end else begin
	 cnt_q <= cnt_d;
	 tick_q <= tick_d;
      end
   end
   

endmodule // clock_divider

module clock_divider(
		     input clk,
		     input rst,
		     input [7:0] divider,
		     output tick
		     );

   reg [7:0] cnt_d, cnt_q;

   assign tick = (cnt_q == 1);
   
   always @(*) begin
      cnt_d = cnt_q + 1'b1;
      if (cnt_q == divider) begin
	 cnt_d = 0;
      end
   end

   always @(posedge clk) begin
      if (rst) begin
	 cnt_q <= 0;
      end else begin
	 cnt_q <= cnt_d;
      end
   end
   

endmodule // clock_divider

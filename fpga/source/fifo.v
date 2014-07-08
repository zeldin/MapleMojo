module fifo(
	    input rst,
	    input clk,
	    input [7:0] indata,
	    input instrobe,
	    output inavail,
	    output [7:0] inavail_cnt,
	    output [7:0] outdata,
	    input outstrobe,
	    output outavail,
	    output [7:0] outavail_cnt
	    );

   function integer log2;
      input integer value;
      begin
	 value = value-1;
	 for (log2=0; value>0; log2=log2+1)
	   value = value>>1;
      end
   endfunction

   parameter depth = 16;
   localparam count_bits = log2(depth);
   
   reg [count_bits-1:0] write_pos_d, write_pos_q;
   reg [count_bits-1:0] read_pos_d, read_pos_q;
   reg [7:0] inavail_cnt_d, inavail_cnt_q;
   reg [7:0] outavail_cnt_d, outavail_cnt_q;
   reg inavail_d, inavail_q;
   reg outavail_d, outavail_q;

   reg [7:0] fifo [0:depth-1];

   assign inavail = inavail_q;
   assign outavail = outavail_q;
   assign inavail = inavail_q;
   assign outavail = outavail_q;
   assign inavail_cnt = inavail_cnt_q;
   assign outavail_cnt = outavail_cnt_q;

   assign outdata = fifo[read_pos_q];

   always @(*) begin
      write_pos_d = write_pos_q;
      read_pos_d = read_pos_q;
      inavail_cnt_d = inavail_cnt_q;
      outavail_cnt_d = outavail_cnt_q;
      inavail_d = inavail_q;
      outavail_d = outavail_q;

      if (instrobe && outstrobe && inavail_q && outavail_q) begin
	 write_pos_d = (write_pos_q == depth-1? {count_bits{1'b0}} : write_pos_q+1'b1);
	 read_pos_d = (read_pos_q == depth-1? {count_bits{1'b0}} : read_pos_q+1'b1);
      end else begin
	 if (instrobe) begin
	    if (inavail_q) begin
	       write_pos_d = (write_pos_q == depth-1? {count_bits{1'b0}} : write_pos_q+1'b1);
	       inavail_cnt_d = inavail_cnt_q - 1'b1;
	       outavail_cnt_d = outavail_cnt_q + 1'b1;
	       if (inavail_cnt_q == 1) begin
		  inavail_d = 1'b0;
	       end
	       outavail_d = 1'b1;
	    end else begin
	       // Overrun
	    end
	 end
	 if (outstrobe) begin
	    if (outavail_q) begin
	       read_pos_d = (read_pos_q == depth-1? {count_bits{1'b0}} : read_pos_q+1'b1);
	       inavail_cnt_d = inavail_cnt_q + 1'b1;
	       outavail_cnt_d = outavail_cnt_q - 1'b1;
	       if (outavail_cnt_q == 1) begin
		  outavail_d = 1'b0;
	       end
	       inavail_d = 1'b1;
	    end else begin
	       // Underrun
	    end
	 end
      end
   end

   always @(posedge clk) begin
      if (rst) begin
	 write_pos_q <= 0;
	 read_pos_q <= 0;
	 inavail_cnt_q <= depth;
	 outavail_cnt_q <= 0;
	 inavail_q <= 1'b1;
	 outavail_q <= 0;
      end else begin
	 write_pos_q <= write_pos_d;
	 read_pos_q <= read_pos_d;
	 inavail_cnt_q <= inavail_cnt_d;
	 outavail_cnt_q <= outavail_cnt_d;
	 inavail_q <= inavail_d;
	 outavail_q <= outavail_d;
      end
      if (instrobe && inavail_q) begin
	 fifo[write_pos_q] <= indata;
      end
   end
   
endmodule // fifo

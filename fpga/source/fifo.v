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
	    output [7:0] outavail_cnt,
	    output overflow,
	    output underflow,
	    input manual_reset
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
   localparam pos_bits = log2(depth);
   localparam count_bits = log2(depth+1);
   
   reg [pos_bits-1:0] write_pos_d, write_pos_q;
   reg [pos_bits-1:0] read_pos_d, read_pos_q;
   reg [count_bits-1:0] inavail_cnt_d, inavail_cnt_q;
   reg [count_bits-1:0] outavail_cnt_d, outavail_cnt_q;
   reg inavail_d, inavail_q;
   reg outavail_d, outavail_q;
   reg overflow_d, overflow_q;
   reg underflow_d, underflow_q;
   
   reg [7:0] fifo [0:depth-1];

   assign inavail = inavail_q;
   assign outavail = outavail_q;
   assign inavail = inavail_q;
   assign outavail = outavail_q;
   assign inavail_cnt = inavail_cnt_q;
   assign outavail_cnt = outavail_cnt_q;

   assign outdata = fifo[read_pos_q];

   assign overflow = overflow_q;
   assign underflow = underflow_q;
   
   always @(*) begin
      write_pos_d = write_pos_q;
      read_pos_d = read_pos_q;
      inavail_cnt_d = inavail_cnt_q;
      outavail_cnt_d = outavail_cnt_q;
      inavail_d = inavail_q;
      outavail_d = outavail_q;
      overflow_d = overflow_q;
      underflow_d = underflow_q;
      
      if (instrobe && outstrobe && inavail_q && outavail_q) begin
	 write_pos_d = (write_pos_q == depth-1? {pos_bits{1'b0}} : write_pos_q+1'b1);
	 read_pos_d = (read_pos_q == depth-1? {pos_bits{1'b0}} : read_pos_q+1'b1);
      end else begin
	 if (instrobe) begin
	    if (inavail_q) begin
	       write_pos_d = (write_pos_q == depth-1? {pos_bits{1'b0}} : write_pos_q+1'b1);
	       inavail_cnt_d = inavail_cnt_q - 1'b1;
	       outavail_cnt_d = outavail_cnt_q + 1'b1;
	       if (inavail_cnt_q == 1) begin
		  inavail_d = 1'b0;
	       end
	       outavail_d = 1'b1;
	    end else begin
	       // Overrun
	       overflow_d = 1'b1;
	    end
	 end
	 if (outstrobe) begin
	    if (outavail_q) begin
	       read_pos_d = (read_pos_q == depth-1? {pos_bits{1'b0}} : read_pos_q+1'b1);
	       inavail_cnt_d = inavail_cnt_q + 1'b1;
	       outavail_cnt_d = outavail_cnt_q - 1'b1;
	       if (outavail_cnt_q == 1) begin
		  outavail_d = 1'b0;
	       end
	       inavail_d = 1'b1;
	    end else begin
	       // Underrun
	       underflow_d = 1'b1;
	    end
	 end
      end
   end

   always @(posedge clk) begin
      if (rst || manual_reset) begin
	 write_pos_q <= 0;
	 read_pos_q <= 0;
	 inavail_cnt_q <= depth;
	 outavail_cnt_q <= 0;
	 inavail_q <= 1'b1;
	 outavail_q <= 0;
	 overflow_q <= 0;
	 underflow_q <= 0;
      end else begin
	 write_pos_q <= write_pos_d;
	 read_pos_q <= read_pos_d;
	 inavail_cnt_q <= inavail_cnt_d;
	 outavail_cnt_q <= outavail_cnt_d;
	 inavail_q <= inavail_d;
	 outavail_q <= outavail_d;
	 overflow_q <= overflow_d;
	 underflow_q <= underflow_d;
      end
      if (instrobe && inavail_q) begin
	 fifo[write_pos_q] <= indata;
      end
   end
   
endmodule // fifo

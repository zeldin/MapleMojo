module regaccess(
   input  clk,
   input  rst,
   input  ss,
   input  mosi,
   output miso,
   input  sck,
   output [6:0] regnum,
   input  [7:0] regdata_read,
   output [7:0] regdata_write,
   output read,
   output write
 );
   
   wire done;
   wire [7:0] din;
   wire [7:0] dout;
   
   reg [6:0] regnum_d, regnum_q;
   reg we_d, we_q;
   reg ss_d, ss_q;
   reg first_d, first_q;
   reg read_d, read_q;
   reg write_d, write_q;

   assign regnum = regnum_q;
   assign read = read_q;
   assign write = write_q;
   assign regdata_write = dout;
   assign din = (read_q? regdata_read : 8'b0);

   spi_slave reg_spi
     (
      .clk(clk), .rst(rst),
      .ss(ss), .mosi(mosi), .miso(miso), .sck(sck),
      .done(done), .din(din), .din_update(read_q), .dout(dout)
     );

   always @(*) begin
      ss_d = ss;
      we_d = we_q;
      first_d = first_q;
      regnum_d = regnum_q;
      read_d = 1'b0;
      write_d = 1'b0;
      if (ss_q) begin
	 we_d = 1'b0;
	 first_d = 1'b1;
      end else if (done) begin
	 if (first_q) begin
	    regnum_d = dout[6:0];
	    we_d = dout[7];
	    read_d = !dout[7];
	 end else if (we_q) begin
	    write_d = 1'b1;
	 end else begin
	    regnum_d = dout[6:0];
	    read_d = 1'b1;
	 end
	 first_d = 1'b0;
      end
   end
   
   always @(posedge clk) begin
      if (rst) begin
	 we_q <= 1'b0;
	 first_q <= 1'b1;
	 regnum_q <= 6'b0;
	 read_q <= 1'b0;
	 write_q <= 1'b0;
      end else begin
	 we_q <= we_d;
	 first_q <= first_d;
	 regnum_q <= regnum_d;
	 read_q <= read_d;
	 write_q <= write_d;
      end

      ss_q <= ss_d;
   end
   
endmodule // regaccess

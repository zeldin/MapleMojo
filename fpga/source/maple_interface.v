module maple_interface(
    input clk,
    input rst,

    inout[0:3] pin1,
    inout[0:3] pin5,
    inout[0:3] pin4,

    inout spi_miso,
    input spi_ss,
    input spi_mosi,
    input spi_sck,

    output tick
   );

   localparam VERSION = 8'ha6;

   localparam CLOCKDIV_INIT = 8'h6;
   
   localparam REG_VERSION = 0;
   localparam REG_SCRATCHPAD = 1;
   localparam REG_CLOCKDIV = 2;
   localparam REG_PORTSEL = 3;
   localparam REG_OUTCTRL = 4;
   localparam REG_INCTRL = 5;
   localparam REG_OUTFIFO_CNT = 6;
   localparam REG_OUTFIFO_FREE = 7;
   localparam REG_FIFO = 8;
   localparam NUM_REGS = 9;
   
   assign pin4 = 4'bzzzz;

   wire[7:0] port_select;

   wire [NUM_REGS-1:0] reg_cs;
   wire reg_we;
   wire [7:0] reg_data_read;
   wire [7:0] reg_data_write;

   wire spi_miso_out;

   wire [7:0] clock_div;
   
   wire   in_p1;
   wire   in_p5;
   wire   out_p1;
   wire   out_p5;
   wire   maple_oe;

   wire [7:0] write_data;
   wire       write_fifo_consume;
   wire       write_data_avail;

   wire [7:0] write_fifo_inavail;
   wire [7:0] write_fifo_outavail;
   
   maple_ports phys_ports
     (
      .pin1(pin1),
      .pin5(pin5),
      .port_select(port_select[1:0]),
      .out_p1(out_p1),
      .out_p5(out_p5),
      .oe(maple_oe),
      .in_p1(in_p1),
      .in_p5(in_p5)
      );

   regaccess #(NUM_REGS) reg_file
     (
      .clk(clk),
      .rst(rst),
      .ss(spi_ss),
      .mosi(spi_mosi),
      .miso(spi_miso_out),
      .sck(spi_sck),
      .cs(reg_cs),
      .regdata_read(reg_data_read),
      .regdata_write(reg_data_write),
      .we(reg_we)
     );

   assign spi_miso = (spi_ss? 1'bz : spi_miso_out);
   
   read_only_reg version_reg(reg_cs[REG_VERSION], reg_we, reg_data_read, VERSION);
   read_write_reg scratchpad_reg(rst, clk, reg_cs[REG_SCRATCHPAD], reg_we, reg_data_read, reg_data_write, );
   read_write_reg #(CLOCKDIV_INIT) clockdiv_reg(rst, clk, reg_cs[REG_CLOCKDIV], reg_we, reg_data_read, reg_data_write, clock_div);
   read_write_reg port_select_reg(rst, clk, reg_cs[REG_PORTSEL], reg_we, reg_data_read, reg_data_write, port_select);
   maple_out out_ctrl(rst, clk, reg_cs[REG_OUTCTRL], reg_we, reg_data_read, reg_data_write, out_p1, out_p5, maple_oe, tick, write_data, write_data_avail, write_fifo_consume);
   read_only_reg outfifo_cnt(reg_cs[REG_OUTFIFO_CNT], reg_we, reg_data_read, write_fifo_outavail);
   read_only_reg outfifo_free(reg_cs[REG_OUTFIFO_FREE], reg_we, reg_data_read, write_fifo_inavail);
   
   fifo #(16) write_fifo(rst, clk, reg_data_write, reg_we&&reg_cs[REG_FIFO], , write_fifo_inavail, write_data, write_fifo_consume, write_data_avail, write_fifo_outavail);
   
   clock_divider clkdiv(clk, rst, clock_div, tick);
   
endmodule // maple_interface

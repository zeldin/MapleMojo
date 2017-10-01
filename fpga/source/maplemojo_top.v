module maplemojo_top(
    input clk,
    input rst_n,
    input cclk,
    output[7:0]led,

    inout porta_pin1,
    inout porta_pin5,
    inout portb_pin1,
    inout portb_pin5,
    inout portc_pin1,
    inout portc_pin5,
    inout portd_pin1,
    inout portd_pin5,

    inout porta_pin4,
    inout portb_pin4,
    inout portc_pin4,
    inout portd_pin4,

    inout spi_miso,
    input spi_ss,
    input spi_mosi,
    input spi_sck
    );

   wire   rst = ~rst_n;

   assign led[7:0] = 8'b0;

   maple_interface maple
     (
      .clk(clk),
      .rst(rst),
      .pin1({porta_pin1, portb_pin1, portc_pin1, portd_pin1}),
      .pin5({porta_pin5, portb_pin5, portc_pin5, portd_pin5}),
      .pin4({porta_pin4, portb_pin4, portc_pin4, portd_pin4}),
      .spi_miso(spi_miso),
      .spi_ss(spi_ss),
      .spi_mosi(spi_mosi),
      .spi_sck(spi_sck)
      );
   
endmodule // maplemojo_top


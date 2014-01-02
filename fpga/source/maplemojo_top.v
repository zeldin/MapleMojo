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
    inout portd_pin4
    );

   wire   rst = ~rst_n;

   assign led[7:0] = 8'b0;

   assign porta_pin4 = 1'bz;
   assign portb_pin4 = 1'bz;
   assign portc_pin4 = 1'bz;
   assign portd_pin4 = 1'bz;

   wire[1:0] port_select;
   assign port_select = 2'b00;
   
   reg 	  out_p1_d, out_p1_q;
   reg 	  out_p5_d, out_p5_q;
   reg 	  maple_oe_d, maple_oe_q;
   wire   in_p1;
   wire   in_p5;

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
   
   maple_ports phys_ports
     (
      .pin1({porta_pin1, portb_pin1, portc_pin1, portd_pin1}),
      .pin5({porta_pin5, portb_pin5, portc_pin5, portd_pin5}),
      .port_select(port_select),
      .out_p1(out_p1_q),
      .out_p5(out_p5_q),
      .oe(maple_oe_q),
      .in_p1(in_p1),
      .in_p5(in_p5)
      );
   
endmodule // maplemojo_top


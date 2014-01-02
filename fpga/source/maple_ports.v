module maple_ports(
    inout[3:0] pin1,
    inout[3:0] pin5,
    input[1:0] port_select,
    input out_p1,
    input out_p5,
    input oe,
    output in_p1,
    output in_p5
    );

   genvar  i;
   generate
      for (i=0; i<4; i=i+1) begin : OUTPIN
	 ocpin p1 (.pin(pin1[i]), .oe(oe && port_select == i), .out(out_p1));
	 ocpin p5 (.pin(pin5[i]), .oe(oe && port_select == i), .out(out_p5));
      end
   endgenerate

   assign in_p1 = pin1[port_select];
   assign in_p5 = pin5[port_select];
   
endmodule // mapleports

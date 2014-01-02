module ocpin(
    inout pin,
    input oe,
    input out
    );

   assign pin = (oe && out == 0)? 1'b0 : 1'bz;

endmodule // ocpin

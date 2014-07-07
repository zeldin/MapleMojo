module read_only_reg(
		     input cs,
		     input we,
		     inout [7:0] regdata,
		     input [7:0] value
		     );

   assign regdata = (cs && !we? value : 8'bz);

endmodule // read_only_reg

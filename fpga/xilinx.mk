
.SECONDEXPANSION:

%.ngc : $$($$*_VERILOG_SOURCES)
	@for s in $($*_VERILOG_SOURCES); do echo 'verilog work "'"$$s"'"'; done > $*.prj
	@echo -e > $*.xst 'set -loop_iteration_limit 1000\nrun\n-ifn $*.prj\n-top $($*_TOP_MODULE)\n-p $($*_DEVICE)\n-ofn $@\n-opt_mode speed\n-opt_level 1\n-netlist_hierarchy rebuilt'
	$(XILINXBIN)/xst -ifn $*.xst -ofn $*.srp

%.ngd : %.ngc $$($$*_CONSTRAINT_FILES)
	$(XILINXBIN)/ngdbuild -p $($*_DEVICE) -dd _ngo $(foreach ucf,$($*_CONSTRAINT_FILES),-uc $(ucf)) $< $@

%_routed.ncd : %.ncd %.pcf
	$(XILINXBIN)/par -w $< $@

%.ncd %.pcf : %.ngd
	$(XILINXBIN)/map -p $($*_DEVICE) -w -o $*.ncd $<

%.twr : %_routed.ncd %.pcf
	$(XILINXBIN)/trce -o $@ -v 30 -l 30 $< $*.pcf

%.bit : %_routed.ncd %.pcf
	$(XILINXBIN)/bitgen $< $@ $*.pcf -g Binary:Yes -g Compress -w


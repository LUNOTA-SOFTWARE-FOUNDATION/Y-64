SV_FILES =						\
	../sv/soc.sv				\
	../sv/chipset/memctl.sv		\
	../sv/chipset/busctl.sv		\
	../sv/cpu/domain.sv			\
	../sv/cpu/rbank.sv

TB_FILES =				\
	test_core.cpp

.PHONY: all
all: obj vcd

.PHONY: obj
obj:
	verilator -Wall --trace -cc $(SV_FILES) --exe $(TB_FILES)

.PHONY: vcd
vcd:
	make -C obj_dir/ -f Vsoc.mk Vsoc
	./obj_dir/Vsoc

SV_FILES =						\
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
	make -C obj_dir/ -f Vrbank.mk Vrbank
	./obj_dir/Vrbank

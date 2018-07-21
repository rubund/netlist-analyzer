
LDFLAGS := $(LDFLAGS) -ltcl -lreadline
CPPFLAGS := -g

OBJECTS := main.o program_state.o

default:
	make -C input
	make -C tclembed
	make -C constraints
	make -C timing
	make netlist_analyzer

netlist_analyzer: $(OBJECTS) input/liberty/liberty.a tclembed/tclembed.a input/verilog/verilog.a constraints/constraints.a timing/timing.a
	g++ $(CPPFLAGS) -o netlist_analyzer $(OBJECTS) input/verilog/verilog.a input/liberty/liberty.a tclembed/tclembed.a constraints/constraints.a timing/timing.a -ltcl -lreadline

clean:
	rm -rf netlist_analyzer $(OBJECTS)
	make -C input clean
	make -C tclembed clean
	make -C constraints clean
	make -C timing clean
	rm -f dependencies

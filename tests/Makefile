CXXFLAGS ?= -g

DEPENDENCIES := program_state.o ../input/liberty/liberty.a ../input/verilog/verilog.a ../constraints/constraints.a ../timing/timing.a


default: test1

../dependencies:
	make -C ../input
	make -C ../constraints
	make -C ../timing
	touch ../dependencies



program_state.o: ../program_state.cc
	g++ $(CPPFLAGS) $(CXXFLAGS) -c -o program_state.o ../program_state.cc

test1: test1.o ../dependencies program_state.o
	g++ $(CPPFLAGS) $(CXXFLAGS) -o test1 test1.o $(DEPENDENCIES)

test1.o: test1.cc
	g++ $(CPPFLAGS) $(CXXFLAGS) -c test1.cc
#netlist_analyzer: $(OBJECTS) input/liberty/liberty.a tclembed/tclembed.a input/verilog/verilog.a constraints/constraints.a timing/timing.a
#	g++ -ggdb -o netlist_analyzer $(OBJECTS) input/verilog/verilog.a input/liberty/liberty.a tclembed/tclembed.a constraints/constraints.a timing/timing.a -ltcl -lreadline


clean:
	rm -f ../dependencies
	rm -f *.o
	rm -f test1

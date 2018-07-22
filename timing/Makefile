CXXFLAGS ?= -g

default: timing.a

timing.a: timing_path.o timing_node.o
	ar rvs timing.a timing_path.o timing_node.o

clean:
	$(RM) -rf *.o *.a

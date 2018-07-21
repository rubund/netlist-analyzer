CPPFLAGS := -g

default: tclembed.a

tclembed.a: interpreter.o
	ar rvs tclembed.a interpreter.o

interpreter.o: interpreter.cc
	g++ $(CPPFLAGS) -c interpreter.cc -I/usr/include/tcl8.6

clean:
	$(RM) -rf *.o *.a

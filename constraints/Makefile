CXXFLAGS ?= -g

OBJECTS := constraints.o constraint_clock.o

constraints.a: $(OBJECTS)
	ar rvs constraints.a $(OBJECTS)

clean:
	rm -rf constraints.a $(OBJECTS)

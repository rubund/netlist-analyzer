CXXFLAGS ?= -g

OBJECTS := verilog_module.o verilog_preprocess.o verilog_module.o verilog_net.o verilog_instance.o verilog_design.o verilog_connection.o connection_builder.o verilog_assignment.o

verilog.a: verilog_parser.tab.o lex.verilog_parser_yy.o $(OBJECTS)
	ar rvs verilog.a verilog_parser.tab.o lex.verilog_parser_yy.o $(OBJECTS)

verilog_parser.tab.c verilog_parser.tab.h:	verilog_parser.y
	bison -d verilog_parser.y

lex.verilog_parser_yy.c: verilog_parser.l verilog_parser.tab.h
	flex verilog_parser.l

verilog_parser.tab.o: verilog_parser.tab.c
	g++ $(CPPFLAGS) $(CXXFLAGS) -c verilog_parser.tab.c

lex.verilog_parser_yy.o: lex.verilog_parser_yy.c
	g++ $(CPPFLAGS) $(CXXFLAGS) -c lex.verilog_parser_yy.c

clean:
	rm -rf verilog_parser.tab.c vefilog_parser.tab.h lex.verilog_parser_yy.c verilog_parser.tab.o lex.verilog_parser.yy.o lex.verilog_parser_yy.o verilog_parser.tab.h verilog.a $(OBJECTS)

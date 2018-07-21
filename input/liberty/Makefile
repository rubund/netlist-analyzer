CPPFLAGS := -g

liberty.a: liberty_parser.tab.o lex.liberty_parser_yy.o lutable.o liberty.o liberty_cell.o liberty_pin.o lutable_numbers.o
	ar rvs liberty.a liberty_parser.tab.o lex.liberty_parser_yy.o lutable.o liberty.o liberty_cell.o liberty_pin.o lutable_numbers.o

liberty_parser.tab.c liberty_parser.tab.h: liberty_parser.y
	bison -d liberty_parser.y

lex.liberty_parser_yy.c: liberty_parser.l liberty_parser.tab.h
	flex liberty_parser.l


liberty_parser.tab.o: liberty_parser.tab.c liberty_parser.tab.h 
	g++ $(CPPFLAGS) -c liberty_parser.tab.c

lex.liberty_parser_yy.o: lex.liberty_parser_yy.c
	g++ $(CPPFLAGS) -c lex.liberty_parser_yy.c


clean:
	rm -rf liberty_parser.tab.h liberty_parser.tab.c lex.liberty_parser_yy.c lutable.o liberty.o liberty_cell.o liberty_pin.o lutable_numbers.o lex.liberty_parser_yy.o liberty_parser.tab.o liberty.a

%{
#define YYDEBUG 1

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "lutable.h"
#include "lutable_numbers.h"
#include "liberty.h"

extern int yylex();
extern int yyparse();
FILE* liberty_parser_mine;

void liberty_parser_yyerror(const char* s);
int liberty_parser_yyget_lineno  (void);


int current_index = -1;
std::string *current_cell = NULL;
std::string *current_pin = NULL;


LutTable *current_lut = NULL;
LutTableNumbers *current_table = NULL;


Liberty *current_file = NULL;

int liberty_error_reading = 0;
int idx = 0;
std::string *current_keyword;
%}

%name-prefix "liberty_parser_yy"

%union {
	int ival;
	float fval;
    std::string *string;
}

%token<ival> T_INT
%token<fval> T_FLOAT
%token<string> TOK_STRING
%token T_NAME
%token T_LEFT T_RIGHT
%token T_NEWLINE
%token T_SEMICOLON
%token T_COMMA
%token T_DOT
%token T_LEFTBRACKET
%token T_RIGHTBRACKET
%token T_LEFTCURLY
%token T_RIGHTCURLY
%token T_COLON
%token T_WIRE
%token T_LIBRARY
%token T_NAMEMINUS
%token T_QUOTATION
%token T_EXPONENT
%token T_BACKSLASH
%token T_CELL
%token T_USEFUL_TABLE
%token T_PIN
%token T_INDEX
%token T_VALUES
%token T_SWITCHING_CONDITION
%token T_DIRECTION
%token T_RISE_CAPACITANCE
%token T_RISE_CAPACITANCE_RANGE
%token T_FALL_CAPACITANCE_RANGE
%token T_FALL_CAPACITANCE
%left T_PLUS T_MINUS
%left T_MULTIPLY T_DIVIDE

/*%type<ival> expression
%type<fval> mixed_expression
*/

%start calculation

%%

calculation: 
	   | calculation line
;

line: T_NEWLINE
    | T_LIBRARY T_LEFT T_NAMEMINUS T_RIGHT {printf("Biblioteket heter: %s\n", liberty_parser_yylval.string->c_str()); delete(liberty_parser_yylval.string);}  curlycontent
;

content:
        simpleval 
    |   bracketvals ending
 /*   |   bracketvals*/

simpleval:
    T_COLON genval T_SEMICOLON

ending:
      T_SEMICOLON
    | T_LEFTCURLY statementlines T_RIGHTCURLY

ending_cell:
      T_SEMICOLON
    | T_LEFTCURLY statementlines_at_cell_level_only T_RIGHTCURLY

genval:
      TOK_STRING {current_file->set_direction_to_last(liberty_parser_yylval.string); delete(liberty_parser_yylval.string);}
    | generic_number
    | T_NAMEMINUS {current_file->set_direction_to_last(liberty_parser_yylval.string); delete(liberty_parser_yylval.string);}

curlycontent:
    T_LEFTCURLY statementlines T_RIGHTCURLY

statementlines:
    statementlines statementline | statementline    
    |

statementlines_at_cell_level_only:
    statementlines_at_cell_level_only statementline_at_cell_level_only | statementline_at_cell_level_only
    |

statementline:
      T_NAMEMINUS {delete(liberty_parser_yylval.string);} content
    | T_USEFUL_TABLE  {current_file->add_lut_to_last(liberty_parser_yylval.string); } content { current_file->set_end_lut();}
    | T_CELL T_LEFT T_NAMEMINUS {current_cell = liberty_parser_yylval.string; current_file->add_cell(liberty_parser_yylval.string);} T_RIGHT ending_cell { current_file->last_added_cell = NULL;}
    | T_INDEX {current_index = liberty_parser_yylval.ival; } T_LEFT {current_file->add_table_to_last(0); current_table = current_file->get_current_table(); } lists_of_numbers T_RIGHT  T_SEMICOLON {current_index = -1; current_file->set_end_table(); current_table = NULL;}
    | T_VALUES T_LEFT { current_file->add_table_to_last(1); current_table = current_file->get_current_table();} lists_of_numbers T_RIGHT { current_file->set_end_table(); current_table = NULL; } T_SEMICOLON 
    | T_SWITCHING_CONDITION T_SEMICOLON
    | T_PIN T_LEFT T_NAMEMINUS {delete(liberty_parser_yylval.string);} T_RIGHT ending
    | T_DIRECTION {current_file->to_find_direction=1;} simpleval {current_file->to_find_direction = 0;}
    | T_RISE_CAPACITANCE {current_keyword = liberty_parser_yylval.string;} T_COLON T_FLOAT { current_file->set_value_generic(current_keyword, 0, liberty_parser_yylval.fval);} T_SEMICOLON { delete(current_keyword);}
    | T_FALL_CAPACITANCE {current_keyword = liberty_parser_yylval.string;} T_COLON T_FLOAT { current_file->set_value_generic(current_keyword, 0, liberty_parser_yylval.fval);} T_SEMICOLON { delete(current_keyword);}
    | T_FALL_CAPACITANCE_RANGE {current_keyword = liberty_parser_yylval.string;} T_LEFT {idx=0;} list_of_floats T_RIGHT T_SEMICOLON { delete(current_keyword);}
    | T_RISE_CAPACITANCE_RANGE {current_keyword = liberty_parser_yylval.string;} T_LEFT {idx=0;} list_of_floats T_RIGHT T_SEMICOLON { delete(current_keyword);}

statementline_at_cell_level_only:
      T_NAMEMINUS {delete(liberty_parser_yylval.string);} content
    | T_SWITCHING_CONDITION T_SEMICOLON
    | T_PIN T_LEFT T_NAMEMINUS {current_file->add_pin_to_last(liberty_parser_yylval.string);} T_RIGHT ending { current_file->set_end_pin();}

lists_of_numbers:
      lists_of_numbers T_COMMA T_QUOTATION list_of_numbers T_QUOTATION {if (current_table != NULL) current_table->goto_next_row();} | T_QUOTATION list_of_numbers T_QUOTATION {if(current_table != NULL) current_table->goto_next_row();} 
    | 

list_of_numbers:
      list_of_numbers T_COMMA generic_number | generic_number
    | 

list_of_floats:
    list_of_floats T_COMMA {idx++;} generic_float_for_generic | generic_float_for_generic
    |

bracketvals:
      T_LEFT T_RIGHT
    | T_LEFT multiplesimple T_RIGHT

multiplesimple:
    multiplesimple T_COMMA simple | simple

simple:
      generic_number
    | T_NAMEMINUS {delete(liberty_parser_yylval.string);}
    | TOK_STRING {delete(liberty_parser_yylval.string);}

generic_number:
      T_INT {if(current_table != NULL) current_table->add_number((float)liberty_parser_yylval.ival); }
    | T_FLOAT {if(current_table != NULL) current_table->add_number(liberty_parser_yylval.fval);}



generic_float_for_generic:
    T_FLOAT { current_file->set_value_generic(current_keyword, idx, liberty_parser_yylval.fval);}


%%


void liberty_parser_yyerror(const char* s) {
	fprintf(stderr, "Parse error: %s in line %d\n", s, liberty_parser_yyget_lineno());
	liberty_error_reading = 1;
}

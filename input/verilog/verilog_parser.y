%{
#define YYDEBUG 1

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "verilog_design.h"
#include "connection_builder.h"

extern int yylex();
extern int yyparse();
FILE* verilog_parser_mine;

void verilog_parser_yyerror(const char* s);

VerilogDesign *current_verilog;

static std::string *tmp_name;
static int tmp_int;
static ConnectionBuilder connbuilder;
static bool get_tmp_range = 0;
static int tmp_range_start;
static int tmp_range_stop;
static int index_from;
static int index_to;
static bool current_input = 0;
static bool current_output = 0;

int verilog_parser_yyget_lineno();

int verilog_error_reading = 0;
%}

%name-prefix "verilog_parser_yy"

%union {
	int ival;
	float fval;
    std::string *string;
}

%token<ival> T_INT
%token<fval> T_FLOAT
%token <string> TOK_STRING
%token <string> T_NAME
%token <string> T_STRANGE_NAME
%token T_PLUS T_MINUS T_MULTIPLY T_DIVIDE T_LEFT T_RIGHT
%token T_NEWLINE T_QUIT
%token T_MODULE
%token T_ENDMODULE
%token T_SEMICOLON
%token T_INPUT
%token T_OUTPUT
%token T_INOUT
%token T_LOGIC
%token T_REG
%token T_COMMA
%token T_DOT
%token T_LEFTBRACKET
%token T_RIGHTBRACKET
%token T_COLON
%token T_WIRE
%token T_LEFTCURLY
%token T_RIGHTCURLY
%token T_ASSIGN
%token T_EQUAL
%token<string> T_CONSTANT
%left T_PLUS T_MINUS
%left T_MULTIPLY T_DIVIDE

%type <string> both_names

%start calculation

%%

calculation: 
	   | calculation line
;

line: T_NEWLINE
    | T_MODULE T_NAME {current_verilog->add_module(verilog_parser_yylval.string);} module_ports_opt T_SEMICOLON module_body {current_verilog->pop_module();} T_ENDMODULE
;

module_ports_opt:
    T_LEFT T_RIGHT
    |
    | T_LEFT module_ports T_RIGHT;


module_ports:
    module_port {current_verilog->set_end_net();} | module_ports T_COMMA module_port {current_verilog->set_end_net();}

module_port:
    wire_type_opt logic_opt {get_tmp_range=1; tmp_range_start=-1; tmp_range_stop=-1; } range_opt both_names { get_tmp_range=0; current_verilog->add_net_to_last($5); current_verilog->set_range_to_last(tmp_range_start, tmp_range_stop); if(current_input) current_verilog->set_last_net_input(); if(current_output) current_verilog->set_last_net_output(); current_input=0; current_output=0;}

both_names:
    T_NAME { $$ = $1; } | T_STRANGE_NAME { $$ = $1; }

wire_type_opt:
    /* */
    | wire_type

wire_type:  T_INPUT { current_input = 1;}  | T_OUTPUT { current_output = 1;  } | T_INOUT { current_output = 1; current_input = 1; }

logic_opt:
    logic
    |

logic:
      T_LOGIC
    | T_REG
    | T_WIRE
    

module_body:
    module_body module_body_line
    |

module_body_line:
      instantiation
    | inputoutputline
    | wireregline
    | T_ASSIGN both_names {tmp_range_stop = 0; get_tmp_range=1;} range_opt {index_to = tmp_range_stop; get_tmp_range=0;} T_EQUAL both_names {tmp_range_stop=0; get_tmp_range=1;} range_opt {index_from = tmp_range_stop; get_tmp_range=0;} T_SEMICOLON { current_verilog->add_assignment_to_last($7,index_from,$2,index_to);}
    

instantiation:
    T_NAME {tmp_name = verilog_parser_yylval.string;} T_NAME { current_verilog->add_instance_to_last(tmp_name, verilog_parser_yylval.string);} T_LEFT connect_ports T_RIGHT T_SEMICOLON { current_verilog->set_end_instance();}

connect_ports:
    connect_ports T_COMMA connect_port | connect_port
    | 

connect_port:
        {tmp_name = NULL;} connection_from_name {delete(verilog_parser_yylval.string);}  /* FIXME No support for instantiations without specifying which port in sub-module is hooked up */
        range_opt
    |   T_DOT both_names {tmp_name = verilog_parser_yylval.string;} T_LEFT connection_or_connections_from_name T_RIGHT

connection_or_connections_from_name:
      connection_from_name_opt
    | connections_from_name

connections_from_name:
    T_LEFTCURLY {connbuilder.init(); connbuilder.set_to_name(tmp_name); delete verilog_parser_yylval.string;} connections_from_name_int {connbuilder.generate_connections(); for(int i=0;i<connbuilder.connections.size();i++) current_verilog->add_connection_to_last(connbuilder.connections[i]); current_verilog->set_end_connection(); connbuilder.cleanup();} T_RIGHTCURLY

connections_from_name_int:
    connections_from_name_int T_COMMA connection_from_name_opt | connection_from_name_opt
    |
  

connection_from_name_opt:
      connection_from_name {
            if (!(connbuilder.active))
                current_verilog->add_connection_to_last(verilog_parser_yylval.string, tmp_name);
            else {
                connbuilder.add_from(verilog_parser_yylval.string);
                if(current_verilog->last_added_module != NULL) {
                    if(connbuilder.from[connbuilder.from.size()-1] == -1 && connbuilder.to[connbuilder.from.size()-1] == -1) {
                        VerilogNet *n = current_verilog->last_added_module->get_net(verilog_parser_yylval.string);
                        if(n != NULL) {
                            connbuilder.set_range_on_last(n->range_from, n->range_to);
                        }
                    }
                }
            }
      } range_opt {
            if (!(connbuilder.active))
                current_verilog->set_end_connection();
        }
    | constant_connection 
    |

connection_from_name:
    both_names

constant_connection:
      T_CONSTANT{ 
            if (!(connbuilder.active))
                current_verilog->add_connection_to_last_constant(verilog_parser_yylval.string, tmp_name);
            else
                connbuilder.add_from_constant(verilog_parser_yylval.string);
            }
    | T_INT {
            if(!(connbuilder.active))
                current_verilog->add_connection_to_last_constant(verilog_parser_yylval.ival, tmp_name);
            else
                connbuilder.add_from_constant(verilog_parser_yylval.ival);
            } 
    

range_opt:
    range
    |

range:
      T_LEFTBRACKET T_INT {tmp_int = verilog_parser_yylval.ival;} T_COLON T_INT {

            if (get_tmp_range) {
                tmp_range_start = verilog_parser_yylval.ival;
                tmp_range_stop  = tmp_int;
                if(tmp_range_stop < tmp_range_start) {
                    int tmpswap;
                    tmpswap = tmp_range_stop;
                    tmp_range_stop = tmp_range_start;
                    tmp_range_start = tmpswap;
                }
            }
            else {
                if(tmp_range_stop < tmp_range_start) {
                    current_verilog->set_range_to_last(tmp_int, verilog_parser_yylval.ival);
                }
                else {
                    current_verilog->set_range_to_last(verilog_parser_yylval.ival, tmp_int);
                }
            }
            if(connbuilder.active)
                connbuilder.set_range_on_last(verilog_parser_yylval.ival, tmp_int);
        } T_RIGHTBRACKET
    | T_LEFTBRACKET T_INT {
            if (get_tmp_range) {
                tmp_range_start = -1;
                tmp_range_stop = verilog_parser_yylval.ival;
            }
            else
                current_verilog->set_range_to_last(-1, verilog_parser_yylval.ival);
            if(connbuilder.active)
                connbuilder.set_range_on_last(-1, verilog_parser_yylval.ival);
        } T_RIGHTBRACKET

inputoutputline:
    wire_type logic_opt signals {current_input = 0; current_output = 0;} T_SEMICOLON

signals:
    signals T_COMMA signal | signal

signal:
    { get_tmp_range=1; tmp_range_start=-1; tmp_range_stop=-1; } range_opt { get_tmp_range=0; } both_names {current_verilog->add_net_to_last(verilog_parser_yylval.string); current_verilog->set_range_to_last(tmp_range_start, tmp_range_stop); if(current_input) current_verilog->set_last_net_input(); if(current_output) current_verilog->set_last_net_output(); current_verilog->set_end_net();}


wireregline:
    logic signals T_SEMICOLON


%%


void verilog_parser_yyerror(const char* s) {
	fprintf(stderr, "Parse error: %s at line %d\n", s, verilog_parser_yyget_lineno());
    verilog_error_reading = 1;
}

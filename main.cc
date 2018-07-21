/*
 *   Copyright (C) 2017-2018  Ruben Undheim <ruben.undheim@gmail.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <iostream>
#include <cstdio>
#include <stdlib.h>
#include <getopt.h>

#include "program_state.h"
#include "input/liberty/lutable.h"
#include "input/liberty/liberty.h"
#include "input/verilog/verilog_design.h"
#include "tclembed/interpreter.h"

int verilog_parser_yyparse (void);
int liberty_parser_yyparse (void);

extern FILE* verilog_parser_mine;
extern FILE* liberty_parser_mine;

extern int liberty_parser_yydebug;
extern int verilog_parser_yydebug;

extern Liberty *current_file;
extern VerilogDesign *current_verilog;
ProgramState *program_state;

int main(int argc, char **argv)
{
    //liberty_parser_yydebug = 1;
    //verilog_parser_yydebug = 1;
    int s;
    std::string script_file = "";

    while ((s = getopt(argc, argv, "f:")) != -1) {
        switch(s) {
            case 'f':
                script_file = std::string(optarg);
                break;
            default:
                fprintf(stderr, "Wrong arguments on command line\n");
                return -1;
                break;
        }
    }
    if (script_file != "")
        std::cout << "Script file is: " << script_file << std::endl;

    liberty_parser_mine = NULL;
    verilog_parser_mine = NULL;
    program_state = new ProgramState();


    tclembed_interpreter(program_state, script_file);


    delete program_state;
	return 0;

}

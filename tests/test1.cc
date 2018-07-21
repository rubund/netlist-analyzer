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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <vector>

#include <cstdio>
#include "../program_state.h"
#include "../timing/timing_node.h"
#include "../timing/timing_path.h"

extern FILE* verilog_parser_mine;
extern FILE* liberty_parser_mine;
int verilog_parser_yyparse (void);
int liberty_parser_yyparse (void);
extern int verilog_error_reading;
extern int liberty_error_reading;
extern VerilogDesign * current_verilog;
extern Liberty * current_file;

int verilog_parser_yylex_destroy (void );
int liberty_parser_yylex_destroy (void );

std::vector<LibertyPin *> already_visited;
std::vector<TimingNode *> all_timing_nodes;

int main()
{
    ProgramState *program_state;
    program_state = new ProgramState();

    current_verilog = new VerilogDesign();
    program_state->add_design(current_verilog);

    verilog_parser_mine = fopen("../vectors/design2.mapped.v", "r");
    if(verilog_parser_mine == NULL) {
        printf("File does not exist\n");
        return -1;
    }


    do {
        verilog_parser_yyparse();
    } while(!feof(verilog_parser_mine) && verilog_error_reading == 0);

    if (verilog_error_reading) {
        printf("Error reading verilog file\n");
        verilog_error_reading = 0;
    }


    fclose(verilog_parser_mine);
    verilog_parser_yylex_destroy();





    liberty_parser_mine = fopen("../basiclib.lib", "r");
    if(liberty_parser_mine == NULL) {
        printf("File does not exist\n");
        return -1;
    }

    current_file = new Liberty();
    program_state->add_library(current_file);

    do { 
        liberty_parser_yyparse();
    } while(!feof(liberty_parser_mine) && liberty_error_reading == 0);

    if (liberty_error_reading) {
        printf("Error reading liberty file\n");
        liberty_error_reading = 0;
        liberty_parser_yylex_destroy();
    }

    liberty_parser_yylex_destroy();
    fclose(liberty_parser_mine);




    program_state->top_module = current_verilog->modules[4];
    //std::cout << *program_state->top_module << std::endl;
    program_state->connect_modules();
    program_state->link_design();
    program_state->has_linked = 1;

    LibertyPin * current_pin;
    std::vector<TimingPath*> * timing_paths;

    current_pin = program_state->get_pin_liberty(std::string("mult_x_155_U130/I0"));
    //std::cout << *current_pin << std::endl;

    already_visited.clear();
    all_timing_nodes.clear();
    timing_paths = current_pin->get_fanin(0, NULL);
    for(int i=0;i<timing_paths->size();i++) {
        std::cout << *timing_paths->at(i);
    }
    for(int i=0;i<timing_paths->size();i++) {
        delete(timing_paths->at(i));
    }
    for(int i=0;i<all_timing_nodes.size();i++){
        delete(all_timing_nodes[i]);
    }
    delete(timing_paths);

    current_pin = program_state->get_pin_liberty(std::string("mult_x_155_U130/ZN"));
    //std::cout << *current_pin << std::endl;

    already_visited.clear();
    all_timing_nodes.clear();
    timing_paths = current_pin->get_fanout(0, NULL);
    for(int i=0;i<timing_paths->size();i++) {
        std::cout << *timing_paths->at(i);
    }
    for(int i=0;i<timing_paths->size();i++) {
        delete(timing_paths->at(i));
    }
    for(int i=0;i<all_timing_nodes.size();i++){
        delete(all_timing_nodes[i]);
    }
    delete(timing_paths);



    delete program_state;
}

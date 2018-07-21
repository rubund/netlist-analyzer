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
#include <string>
#include <tcl.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "../program_state.h"
#include "../input/liberty/liberty.h"
#include "../input/verilog/verilog_design.h"
#include "../input/verilog/verilog_assignment.h"
#include "../constraints/constraints.h"
#include "../constraints/constraint_clock.h"
#include "../timing/timing_path.h"
#include "../timing/timing_node.h"

static char *buffer;
size_t maxlen = 511;
static int ended = 0;

extern FILE* liberty_parser_mine;
extern FILE* verilog_parser_mine;
extern Liberty *current_file;
extern VerilogDesign *current_verilog;
int liberty_parser_yyparse (void);
int verilog_parser_yyparse (void);

int verilog_parser_yylex_destroy (void );
int liberty_parser_yylex_destroy (void );

extern int liberty_error_reading;
extern int verilog_error_reading;

int nignore = 0;

char const *extra_commands[] = {
    "all_registers",
    "create_clock",
    "exit",
    "get_cell",
    "get_fanin",
    "get_fanout",
    "get_pin",
    "help",
    "link_design",
    "list_libs",
    "report_delay_calculation",
    "report_timing",
    "read_verilog",
    "read_liberty",
    "report_clock",
    "restart",
    "set_design_top",
    "source",
    "traverse_input",
    "quit",
    "-from",
    "-to",
    NULL
};

ProgramState *state;

std::vector<LibertyPin *> already_visited;
std::vector<TimingNode *> all_timing_nodes;

char **extra_command_completion(const char *text, int start, int end);
char *extra_command_generator(const char *text, int state);

void save_history();

void sigint_handler(int v)
{
    std::cout << std::endl;
    save_history();
    exit(-1);
}

int command_report_timing(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    printf("Ran report_timing with these arguments:\n");
    int i;
    for (i=0;i<objc;i++) {
        printf("Et arg %s\n", objv[i]->bytes);
    }
    Tcl_Obj *obj_to_return;
    obj_to_return = Tcl_NewObj();
    Tcl_SetIntObj(obj_to_return, 1);
    Tcl_SetObjResult(interp, obj_to_return);
    return 0;
}

int command_quit(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    ended = 1;
    return 0;
}

int command_read_liberty(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    if(objc >= 2) {
        printf("Reading liberty file %s\n", objv[1]->bytes);
	    liberty_parser_mine = fopen(objv[1]->bytes, "r");
        if(liberty_parser_mine == NULL) {
            printf("File does not exist\n");
            return -1;
        }

        current_file = new Liberty();
        state->add_library(current_file);

        do { 
            liberty_parser_yyparse();
        } while(!feof(liberty_parser_mine) && liberty_error_reading == 0);

        if (liberty_error_reading) {
            printf("Error reading liberty file\n");
            liberty_error_reading = 0;
            liberty_parser_yylex_destroy();
            return -1;
        }
        fclose(liberty_parser_mine);
        liberty_parser_yylex_destroy();

        printf("Done\n");
        return 0;
    }
    else {
        printf("Missing argument. Usage: read_liberty <file>\n");
        return -1;
    }
}

int command_read_verilog(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    if(objc >= 2) {
        printf("Reading verilog design %s\n", objv[1]->bytes);
	    verilog_parser_mine = fopen(objv[1]->bytes, "r");
        if(verilog_parser_mine == NULL) {
            printf("File does not exist\n");
            return -1;
        }

        current_verilog = new VerilogDesign();
        state->add_design(current_verilog);

	    do { 
	    	verilog_parser_yyparse();
	    } while(!feof(verilog_parser_mine) && verilog_error_reading == 0);

        if (verilog_error_reading) {
            printf("Error reading verilog file\n");
            verilog_error_reading = 0;
            verilog_parser_yylex_destroy();
            return -1;
        }
        fclose(verilog_parser_mine);
        verilog_parser_yylex_destroy();

        printf("Done\n");
        return 0;
    }
    else {
        printf("Missing argument. Usage: read_verilog <file>\n");
        return -1;
    }
}

int command_set_design_top(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    if(objc >= 2) {
        int idx = atoi(objv[1]->bytes);
        std::string top_module_name = std::string(objv[1]->bytes);
        if(state->has_linked) {
            printf("Already linked. You need to restart to set top design again\n");
            return -1;
        }
        else if(state->designs.size() == 0) {
            printf("No designs have been read. Use read_verilog\n");
            return -1;
        }
        else if (top_module_name != "") {
            int n = current_verilog->modules.size();
            for(int i =0;i<n;i++) {
                if (*current_verilog->modules[i]->name == top_module_name) {
                    state->top_module = current_verilog->modules[i];
                    std::cout << "The module " << *state->top_module->name << " has been set as top module" << std::endl;
                    return 0;
                }
            }
            std::cout << "Module " <<  top_module_name << " not found." << std::endl;
            return -1;
        }
        else {
            printf("Index out of range\n");
            return -1;
        }
    }
    else {
        printf("Missing argument. Usage: set_design_top <index>\n");
        return -1;
    }
}


int command_link_design(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    int ret;
    if(state->has_linked) {
        printf("Already linked. You need to restart to link again\n");
        return -1;
    }
    else if(state->designs.size() > 0 && state->top_module != NULL) {
        if(state->connect_modules() != 0) return -1;
        if(state->link_design()     != 0) return -1;
        state->has_linked = 1;
        return 0;
    }
    else {
        printf("No module set as top module, or no designs read\n");
        return -1;
    }
}

void get_all_level(VerilogNet *n) {
    for(int i=0;i<n->connections_down.size();i++) {
        if(n->connections_down[i]->net_down != NULL) {
            get_all_level(n->connections_down[i]->net_down);
        }
        else if(n->connections_down[i]->pin_down != NULL) {
            std::cout << n->connections_down[i]->pin_down->cell->get_path() << "/" << *n->connections_down[i]->pin_down->name << " (" << *n->connections_down[i]->pin_down->cell->name << ")";
            if(n->connections_down[i]->pin_down->input)
                std::cout << " (input)";
            else if(n->connections_down[i]->pin_down->output)
                std::cout << " (output)";
            std::cout << std::endl;
            for(int j=0;j<n->connections_down[i]->pin_down->cell->pins.size();j++) {
                if(n->connections_down[i]->pin_down->cell->pins[j]->output) {
                    std::cout << "  Output pin: " << *n->connections_down[i]->pin_down->cell->pins[j] << std::endl;
                    if(n->connections_down[i]->pin_down->cell->pins[j]->name->compare("Q") != 0) // XXX: Do not propagate beyond flip-flops
                        n->connections_down[i]->pin_down->cell->pins[j]->print_receivers(1);
                }
            }
        }
    }
    for(int i=0;i<n->assigned_to.size();i++) {
        if(n->assigned_to[i]->to != NULL) {
            get_all_level(n->assigned_to[i]->to);
        }
    }
}

int command_traverse_input(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    if(!(state->has_linked)) {
        std::cout << "Design must be linked first" << std::endl;
        return -1;
    }
    if(objc < 2) {
        std::cout << "Usage: traverse_input <pin>" << std::endl;
        return -1;
    }
    VerilogNet *start_net = NULL;
    for(int i=0; i< state->top_module->nets.size() ;i++) {
        VerilogNet *n = state->top_module->nets[i];
        if(n->input) {
            if(n->name->compare(objv[1]->bytes) == 0) {
                std::cout << "One input: " << *n << std::endl;
                start_net = n;
                already_visited.clear();
                get_all_level(start_net);
            }
        }
    }

}

int command_get_cell(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    char tmpstr[256];
    if(!(state->has_linked)) {
        std::cout << "Design must be linked first" << std::endl;
        return -1;
    }
    if(objc < 2) {
        std::cout << "Usage: get_cell <cell>" << std::endl;
        return -1;
    }
    const char *cellname = objv[1]->bytes;
    const char *end, *end2;

    VerilogModule *mod = state->top_module;;

    while(cellname != NULL) {
        if(end == NULL) return -1;
        end2 = strchr(cellname,'/');
        if(end2 != NULL) {
            int ncopy = (end2-cellname > 255) ? 255 : end2-cellname;
            strncpy(tmpstr, cellname, ncopy);
            tmpstr[ncopy] = 0;
            cellname = end2+1;
        }
        else {
            strcpy(tmpstr, cellname);
            cellname = NULL;
        }
        //std::cout << tmpstr << std::endl;

        if(tmpstr[0] == '*') {
            for(int i=(mod->instances.size())-1;i>=0; i--) {
                if(mod->instances[i]->associated_cell != NULL) {
                    std::cout << mod->instances[i]->associated_cell->get_path() << " (" << *mod->instances[i]->module_name << ")" << std::endl;
                }
                else if(mod->instances[i]->associated_module != NULL) {
                    std::cout << mod->instances[i]->associated_module->get_path() << " (" << *mod->instances[i]->module_name << ")" << std::endl;
                }
            }
        }
        else {
            bool found = 0;
            for(int i=0;i<mod->instances.size(); i++) {
                if(mod->instances[i]->associated_module != NULL && mod->instances[i]->name->compare(tmpstr) == 0) {
                    mod = mod->instances[i]->associated_module;
                    found = 1;
                    break;
                    //std::cout << *state->top_module->instances[i]->associated_module << std::endl;
                }
            }
            if(!found) break;
        }
    }
    return 0;
}

int command_get_fanin(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    if(objc < 2) {
        std::cout << "Usage: get_fanin <pin>" << std::endl;
        return -1;
    }
    std::string pin_path = std::string(objv[1]->bytes);
    LibertyPin * current_pin = state->get_pin_liberty(pin_path);
    if(current_pin == NULL)
        return -1;
    std::cout << "Found pin: " << *current_pin << " in cell " << current_pin->cell->get_path() << " (" << *current_pin->cell->name << ")" << std::endl;
//    LibertyPin * first_upstream = current_pin->get_upstream();
//    if(first_upstream == NULL)
//        return -1;
//    std::cout << "Driven by pin: " << *first_upstream << " in cell " << first_upstream->cell->get_path() << " (" << *first_upstream->cell->name << ")" << std::endl;
//
    already_visited.clear();
    all_timing_nodes.clear();
    //current_pin->print_fanin(1);
    std::vector<TimingPath*> * timing_paths;
    timing_paths = current_pin->get_fanin(0, NULL);
    std::cout << "Done finding paths" << std::endl << std::endl;
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


    return 0;
}


int command_get_fanout(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    if(objc < 2) {
        std::cout << "Usage: get_fanout <pin>" << std::endl;
        return -1;
    }
    std::string pin_path = std::string(objv[1]->bytes);
    LibertyPin * current_pin = state->get_pin_liberty(pin_path);
    if(current_pin == NULL)
        return -1;
    std::cout << "Found pin: " << *current_pin << " in cell " << current_pin->cell->get_path() << " (" << *current_pin->cell->name << ")" << std::endl;

    already_visited.clear();
    all_timing_nodes.clear();
    //current_pin->print_fanin(1);
    std::vector<TimingPath*> * timing_paths;
    timing_paths = current_pin->get_fanout(0, NULL);
    std::cout << "Done finding paths" << std::endl << std::endl;
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


    return 0;
}

int command_report_delay_calculation(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    if(objc < 2) {
        std::cout << "Usage: report_delay_calculation <pin>" << std::endl;
        return -1;
    }
    std::string pin_path = std::string(objv[1]->bytes);
    LibertyPin * current_pin = state->get_pin_liberty(pin_path);
    if(current_pin == NULL)
        return -1;
    std::cout << "Found pin: " << *current_pin << " in cell " << current_pin->cell->get_path() << " (" << *current_pin->cell->name << ")" << std::endl;

    double transition = current_pin->calculate_transition(8, 0, 0);
    std::cout << "Transition is " << transition << std::endl;

    double celldelay = current_pin->calculate_delay(8, 0, 0);
    std::cout << "Cell delay is " << celldelay << std::endl;

    return 0;
}


int command_get_pin(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    if(objc < 2) {
        std::cout << "Usage: get_fanin <expression>" << std::endl;
        return -1;
    }
    std::string pin_path = std::string(objv[1]->bytes);
    size_t endpos = pin_path.find("/*");
    if(endpos == std::string::npos) {
        std::cout << "Needs a /* expression" << std::endl;
        return -1;
    }
    std::string cell_path = pin_path.substr(0, endpos);
    //std::cout << "cell path: " << cell_path << std::endl;
    LibertyCell *cell = state->get_cell_liberty(cell_path);
    if(cell == NULL) {
        std::cout << "Could not find cell" << std::endl;
        return -1;
    }

    cell->print_pins();
    return 0;

}


int command_create_clock(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    int retval;
    if(objc < 4) {
        std::cout << "Usage: create_clock <name> -net <top_port>" << std::endl;
        return -1;
    }
    if(strcmp(objv[2]->bytes, "-net") != 0) {
        std::cout << "Second argument is not -net" << std::endl;
        std::cout << "Usage: create_clock <name> -net <top_port>" << std::endl;
        return -1;
    }
    float period = 32; // FIXME
    float half_period = 15; // FIXME
    std::string *name = new std::string(objv[1]->bytes);
    std::string *signal = new std::string(objv[3]->bytes);
    retval = state->create_clock(name, signal, period, half_period);
    return retval;
}

int command_report_clock(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    for(int i=0;i<state->constraints.clocks.size();i++) {
        std::cout << *state->constraints.clocks[i] << std::endl;
    }
}

int command_help(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    const char *current;
    int i = 0;
    std::cout << std::endl;
    std::cout << "Available commands:" << std::endl;
    while(current != NULL) {
        current = extra_commands[i];
        if (current != NULL) {
            std::cout << "  " << current << std::endl;
        }
        i++;
    }
    return 0;
}

int command_source_tcl(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    int status;
    if (objc < 2) {
        std::cout << "Missing filename" << std::endl;
        return -1;
    }

    FILE *source_in_fp = fopen(objv[1]->bytes, "r");
    if(source_in_fp != NULL) {
        char *line = NULL;
        size_t len = 0;
        ssize_t read;
        while((read = getline(&line, &len, source_in_fp)) != -1) {
            line[strlen(line)-1] = 0;
            //printf("Running command from source file: %s\n",line);
            status = Tcl_Eval(interp, line);
            if(status != 0) {
                printf("Error running command: %s\n", line);
                printf("Stopped sourcing file\n");
                break;
            }
        }
        fclose(source_in_fp);
        return 0;
    }
    else {
        std::cout << "Could not find file " << objv[1]->bytes << std::endl;
        return -1;
    }
    return -1;
}

int command_restart(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    state->cleanup();
    return 0;
}

int command_exit(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    ended = 1;
    return 0;
}

void save_history()
{
    std::cout << "Saving command history" << std::endl;
    HIST_ENTRY **the_list;
    the_list = history_list();
    if(the_list){
        FILE *histout = fopen(".history_netlist_analyzer", "w");
        for(int i=0; the_list[i] ; i++) {
            if(i>=nignore)
                fprintf(histout, "%s\n", the_list[i]->line);
        }
        fclose(histout);
    }

}

int tclembed_interpreter(ProgramState *program_state, std::string script_file)
{
    int status;
    Tcl_Interp *myInterp;
    //char *action = "set a [expr 5 * 8] ; puts $a";
    signal(SIGINT, sigint_handler); // TODO: Fix later
    //buffer = (char*) malloc(512);
    state = program_state;

    myInterp = Tcl_CreateInterp();
    Tcl_CreateObjCommand(myInterp, "report_timing", (Tcl_ObjCmdProc*)command_report_timing, 0, 0);
    Tcl_CreateObjCommand(myInterp, "quit", (Tcl_ObjCmdProc*)command_quit, 0, 0);
    Tcl_CreateObjCommand(myInterp, "read_liberty", (Tcl_ObjCmdProc*)command_read_liberty, 0, 0);
    Tcl_CreateObjCommand(myInterp, "read_verilog", (Tcl_ObjCmdProc*)command_read_verilog, 0, 0);
    Tcl_CreateObjCommand(myInterp, "set_design_top", (Tcl_ObjCmdProc*)command_set_design_top, 0, 0);
    Tcl_CreateObjCommand(myInterp, "link_design", (Tcl_ObjCmdProc*)command_link_design, 0, 0);
    Tcl_CreateObjCommand(myInterp, "traverse_input", (Tcl_ObjCmdProc*)command_traverse_input, 0, 0);
    Tcl_CreateObjCommand(myInterp, "get_cell", (Tcl_ObjCmdProc*)command_get_cell, 0, 0);
    Tcl_CreateObjCommand(myInterp, "create_clock", (Tcl_ObjCmdProc*)command_create_clock, 0, 0);
    Tcl_CreateObjCommand(myInterp, "report_clock", (Tcl_ObjCmdProc*)command_report_clock, 0, 0);
    Tcl_CreateObjCommand(myInterp, "get_fanin", (Tcl_ObjCmdProc*)command_get_fanin, 0, 0);
    Tcl_CreateObjCommand(myInterp, "get_fanout", (Tcl_ObjCmdProc*)command_get_fanout, 0, 0);
    Tcl_CreateObjCommand(myInterp, "get_pin", (Tcl_ObjCmdProc*)command_get_pin, 0, 0);
    Tcl_CreateObjCommand(myInterp, "report_delay_calculation", (Tcl_ObjCmdProc*)command_report_delay_calculation, 0, 0);
    Tcl_CreateObjCommand(myInterp, "source", (Tcl_ObjCmdProc*)command_source_tcl, 0, 0);
    Tcl_CreateObjCommand(myInterp, "help", (Tcl_ObjCmdProc*)command_help, 0, 0);
    Tcl_CreateObjCommand(myInterp, "restart", (Tcl_ObjCmdProc*)command_restart, 0, 0);
    Tcl_CreateObjCommand(myInterp, "exit", (Tcl_ObjCmdProc*)command_exit, 0, 0);

    rl_attempted_completion_function = extra_command_completion;


    FILE *frc = fopen(".netlist_analyzer.rc", "r");
    if(frc != NULL) {
        char *line = NULL;
        size_t len = 0;
        ssize_t read;
        while((read = getline(&line, &len, frc)) != -1) {
            line[strlen(line)-1] = 0;
            printf("Running command from .netlist_analyzer.rc: %s\n",line);
            status = Tcl_Eval(myInterp, line);
            if(status != 0) {
                printf("Error running command: %s\n", line);
            }
            if (line[0] != 0) {
                if(line[0] == '#')
                    add_history(line+1);
                else
                    add_history(line);
                nignore++;
            }
            }
        fclose(frc);
    }

    FILE *histin = fopen(".history_netlist_analyzer", "r");
    if(histin != NULL) {
        char *line = NULL;
        size_t len = 0;
        ssize_t read;
        while((read = getline(&line, &len, histin)) != -1) {
            line[strlen(line)-1] = 0;
            if (line[0] != 0) {
                add_history(line);
            }
        }
        fclose(histin);
    }

    frc = fopen(".netlist_analyzer.history.after", "r");
    if(frc != NULL) {
        char *line = NULL;
        size_t len = 0;
        ssize_t read;
        while((read = getline(&line, &len, frc)) != -1) {
            line[strlen(line)-1] = 0;
            if (line[0] != 0) {
                add_history(line);
            }
        }
        fclose(frc);
    }

    if(script_file != "") {
        frc = fopen(script_file.c_str(), "r");
        if(frc != NULL) {
            char *line = NULL;
            size_t len = 0;
            ssize_t read;
            while((read = getline(&line, &len, frc)) != -1) {
                line[strlen(line)-1] = 0;
                printf("Running command from %s: %s\n",script_file.c_str(),line);
                status = Tcl_Eval(myInterp, line);
                if(status != 0) {
                    printf("Error running command: %s\n", line);
                }
                }
            fclose(frc);
        }
    }
    while((!ended) && (buffer = readline("netlist_analyzer> ")) != NULL) {
        status = Tcl_Eval(myInterp, buffer);
        if(status != 0) {
            printf("Error running command: %s\n", buffer);
        }
        if (buffer[0] != 0)
            add_history(buffer);
        free(buffer);
        if(ended) break;
    }
    printf("Your program has completed\n");

    save_history();

    Tcl_DeleteInterp(myInterp);
    Tcl_Finalize();
}


char **extra_command_completion(const char *text, int start, int end)
{
    rl_attempted_completion_over = 0;
    return rl_completion_matches(text, extra_command_generator);
}

char *extra_command_generator(const char *text, int state)
{
    static int list_index, len;
    const char *name;

    if (!state) {
        list_index = 0;
        len = strlen(text);
    }

    while ((name = extra_commands[list_index++])) {
        if(strncmp(name, text, len) == 0) {
            return strdup(name);
        }
    }

    return NULL;
}



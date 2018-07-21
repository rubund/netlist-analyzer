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
#include <cstring>
#include "program_state.h"
#include "input/liberty/liberty.h"
#include "constraints/constraints.h"
#include "constraints/constraint_clock.h"

#define DEBUG 0

ProgramState::ProgramState()
{
    top_module = NULL;
    has_linked = 0;
    missing_library = NULL;
}

ProgramState::~ProgramState()
{
    this->cleanup();
}

/* Adds a new Liberty file to the program state
*/
void ProgramState::add_library(Liberty *lib)
{
    libraries.push_back(lib);
}

/* Adds a new Verilog file to the program state
*/
void ProgramState::add_design(VerilogDesign *design)
{
    designs.push_back(design);
}

/* This method goes over all instances in all modules. At first it
   splits any nets wider than 1, and any connections wider than 1. Then
   it checks if either there is module definition matching the instantiation,
   or a cell definition in a Liberty file matchin the instantiation. All pointers
   are then hooked up. If any instantiation does not have a matching definition
   somewhere, the method will fail with status == -1.
*/
int ProgramState::connect_modules()
{
    std::string *instance_name;
    std::string *module_name;

    bool add_missing = 1; // FIXME

    missing_library = new Liberty();
    int done;
    for(int l=0;l < designs.size() ;l++){
        for(int i=0;i < designs[l]->modules.size() ;i++){
            designs[l]->modules[i]->split_nets();
            for(int j=0;j<designs[l]->modules[i]->instances.size(); j++) {
                done = 0;
                instance_name = designs[l]->modules[i]->instances[j]->module_name;
                designs[l]->modules[i]->instances[j]->split_connections();

                // Check if instance is a module in the design:
                for(int m=0;m < designs.size() ;m++){
                    for(int k=0;k < designs[m]->modules.size() ;k++){
                        module_name = designs[m]->modules[k]->name;
                        if(instance_name->compare(*module_name) == 0) {
                            designs[l]->modules[i]->instances[j]->associated_module = designs[m]->modules[k];
                            designs[l]->modules[i]->instances[j]->save_memory();
                            if(DEBUG) std::cout << "Found " << (*designs[l]->modules[i]->instances[j]->module_name) << " is a module instance" << std::endl;
                            done = 1;
                            break;
                        }
                    }
                    if(done) break;
                }
                if(done) continue;

                // Check if instance is a cell in the library:
                for(int m=0;m < libraries.size() ;m++){
                    for(int k=0;k < libraries[m]->cells.size() ;k++){
                        module_name = libraries[m]->cells[k]->name;
                        if(instance_name->compare(*module_name) == 0) {
                            designs[l]->modules[i]->instances[j]->associated_cell = libraries[m]->cells[k];
                            designs[l]->modules[i]->instances[j]->save_memory();
                            if(DEBUG) std::cout << "Found " << (*designs[l]->modules[i]->instances[j]->module_name) << " is a cell instance" << std::endl;
                            done = 1;
                            break;
                        }
                    }
                    if(done) break;
                }
                if(done) continue;

                // FIXME: Temporarily commented out
                if(add_missing) {
                    std::cout << "Warning: Cell or module for " << (*instance_name) << " not found. Have you read all necessary libraries?" << std::endl;
                    for(int k=0;k<missing_library->cells.size() ; k++) {
                        module_name = missing_library->cells[k]->name;
                        if(instance_name->compare(*module_name) == 0) {
                            designs[l]->modules[i]->instances[j]->associated_cell = missing_library->cells[k];
                            designs[l]->modules[i]->instances[j]->save_memory();
                            if(DEBUG) std::cout << "Found " << (*designs[l]->modules[i]->instances[j]->module_name) << " in the automatically generated missing library" << std::endl;
                            done = 1;
                            break;
                        }
                    }
                    if(!done) {
                        LibertyCell *new_missing_cell = new LibertyCell(instance_name);
                        new_missing_cell->is_missing = 1;
                        missing_library->cells.push_back(new_missing_cell);
                        designs[l]->modules[i]->instances[j]->associated_cell = new_missing_cell;
                        //designs[l]->modules[i]->instances[j]->save_memory(); // Cannot do this, since the string is actually taken from this instance..
                        std::cout << "Created missing cell definition " << (*designs[l]->modules[i]->instances[j]->module_name) << std::endl;
                        done = 1;
                    }
                }
                else {
                    std::cout << "Error: Cell or module for " << (*instance_name) << " not found. Have you read all necessary libraries?" << std::endl;
                    return -1;
                }
            }
        }
    }
    return 0;
}

/* After the top module has been set, it is possible to link the design.
   This will clone the module instances for every instance where they are present.
   Afterwards, all connections in all instantiations will be iterated, and pure
   pointers hooked up for all signals in the design.
   If anything goes wrong, it returns -1
*/
int ProgramState::link_design()
{
    bool anyfailing=0;
    VerilogInstance *inst;
    if(top_module != NULL){
        for(int i=0;i<top_module->nets.size();i++) {
            top_module->nets[i]->parent = top_module;
        }
        if(top_module->populate() != 0) anyfailing = 1;
        if(top_module->hook_up_ports() != 0) anyfailing = 1;
        if(anyfailing == 0) {
            std::cout << "Successfully linked design!" << std::endl;
            return 0;
        }
        else {
            std::cout << "Failed linking design" << std::endl;
            return -1;
        }
    }
    else {
        printf("No module set to top module (set_design_top)\n");
        return -1;
    }
}

/* Add a clock constraint to the program state
   Also check if there is a matching net. If not, fail with return value -1. 
*/
int ProgramState::create_clock(std::string *name, std::string *signal, float period, float half_period)
{
    bool found = 0;
    if(has_linked) {
        for(int i=0;i<top_module->nets.size();i++) {
            if(signal->compare(*top_module->nets[i]->name) == 0) {
                ConstraintClock *new_clock = new ConstraintClock(name, signal, period, half_period);
                constraints.clocks.push_back(new_clock);
                new_clock->net = top_module->nets[i];
                std::cout << "Found input " << *top_module->nets[i] << ", and connected the clock to it" << std::endl;
                found = 1;
                return 0;
            }
        }
    }
    if(!found) {
        std::cout << "Could not find specified net" << std::endl;
        return -1;
    }

}



LibertyCell * ProgramState::get_cell_liberty(std::string path)
{
    char tmpstr[256];
    if(!(has_linked)) {
        std::cout << "Design must be linked first" << std::endl;
        return NULL;
    }

    const char *cellname = path.c_str();
    VerilogModule *mod = top_module;;
    LibertyCell *cell = NULL;
    const char *end2;

    while(cellname != NULL) {
        end2 = strchr(cellname, '/');
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

        bool found = 0;
        for(int i=0;i<mod->instances.size() ;i++) {
            if(mod->instances[i]->associated_module != NULL && mod->instances[i]->name->compare(tmpstr) == 0) {
                mod = mod->instances[i]->associated_module;
                found = 1;
                break;
            }
            else if(mod->instances[i]->associated_cell != NULL && mod->instances[i]->name->compare(tmpstr) == 0) {
                cell = mod->instances[i]->associated_cell;
                return cell;
            }
        }
        if(!found) return NULL;
    }
    return NULL;
}


LibertyPin * ProgramState::get_pin_liberty(const std::string &path)
{
    char tmpstr[256];
    if(!(has_linked)) {
        std::cout << "Design must be linked first" << std::endl;
        return NULL;
    }

    const char *cellname = path.c_str();
    VerilogModule *mod = top_module;;
    LibertyCell *cell = NULL;
    const char *end2;
    bool current_is_cell = 0;

    while(cellname != NULL) {
        end2 = strchr(cellname, '/');
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

        if(!current_is_cell) {
            bool found = 0;
            for(int i=0;i<mod->instances.size() ;i++) {
                if(mod->instances[i]->associated_module != NULL && mod->instances[i]->name->compare(tmpstr) == 0) {
                    mod = mod->instances[i]->associated_module;
                    found = 1;
                    break;
                }
                else if(mod->instances[i]->associated_cell != NULL && mod->instances[i]->name->compare(tmpstr) == 0) {
                    cell = mod->instances[i]->associated_cell;
                    found = 1;
                    current_is_cell = 1;
                    break;
                }
            }
            if(!found) return NULL;
        }
        else {
            bool found = 0;
            for(int i=0;i<cell->pins.size() ; i++) {
                if(cell->pins[i]->name->compare(tmpstr) == 0) {
                    return cell->pins[i];
                }
            }
        }
    }
    return NULL;
}

VerilogNet * ProgramState::get_pin_verilog(std::string path)
{
    if(!(has_linked)) {
        std::cout << "Design must be linked first" << std::endl;
        return NULL;
    }

}


VerilogModule * ProgramState::get_cell_verilog(std::string path)
{
    if(!(has_linked)) {
        std::cout << "Design must be linked first" << std::endl;
        return NULL;
    }

}


void ProgramState::cleanup()
{
    for(int i=0;i<designs.size();i++){
        designs[i]->cleanup();
        delete designs[i];
    }
    for(int i=0;i<libraries.size();i++){
        libraries[i]->cleanup();
        delete libraries[i];
    }
    designs.clear();
    libraries.clear();
    top_module = NULL;
    has_linked = 0;
    if(missing_library != NULL) {
        delete missing_library;
        missing_library = NULL;
    }
}

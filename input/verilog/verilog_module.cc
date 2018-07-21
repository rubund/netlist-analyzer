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
#include "verilog_module.h"
#include "verilog_instance.h"
#include "verilog_connection.h"
#include "verilog_assignment.h"
#include "../liberty/liberty_cell.h"

#define DEBUG 0

VerilogModule::VerilogModule(std::string *name)
{
    last_added_net = NULL;
    last_added_submodule = NULL;
    last_added_instance = NULL;
    last_added_assignment = NULL;
    this->name = name;
    this->parent = NULL;
    this->instance = NULL;
}

VerilogModule::~VerilogModule()
{
    this->cleanup();
}

void VerilogModule::cleanup()
{
    for(int i=0;i<nets.size();i++){
        delete nets[i];
    }
    for(int i=0;i<submodules.size();i++){
        delete submodules[i];
    }
    for(int i=0;i<instances.size();i++){
        instances[i]->cleanup();
        delete instances[i];
    }
    for(int i=0;i<assignments.size();i++){
        delete assignments[i];
    }
    for(int i=0;i<cells.size();i++){
        delete cells[i];
    }
    nets.clear();
    submodules.clear();
    instances.clear();
    assignments.clear();
    cells.clear();
}

VerilogNet *VerilogModule::add_net(std::string *name)
{
    last_added_net = new VerilogNet(name);
    nets.push_back(last_added_net);
    return last_added_net;
}

VerilogNet *VerilogModule::add_net()
{
    last_added_net = new VerilogNet();
    nets.push_back(last_added_net);
    return last_added_net;
}

VerilogModule *VerilogModule::add_submodule(std::string *name)
{
    last_added_submodule = new VerilogModule(name);
    submodules.push_back(last_added_submodule);
    return last_added_submodule;
}

VerilogInstance *VerilogModule::add_instance(std::string *module_name, std::string *name)
{
    last_added_instance = new VerilogInstance(module_name, name);
    instances.push_back(last_added_instance);
    return last_added_instance;
}

VerilogAssignment *VerilogModule::add_assignment(std::string *from, int from_index, std::string *to, int to_index)
{
    VerilogAssignment *ass = new VerilogAssignment();
    last_added_assignment = ass;
    ass->from_name = from;
    ass->from_index = from_index;
    ass->to_name = to;
    ass->to_index = to_index;
    assignments.push_back(ass);
    return ass;
}

VerilogNet *VerilogModule::get_net(std::string *name)
{
    for(int i=0;i<nets.size();i++) {
        if(name->compare(*(nets[i]->name)) == 0) {
            return nets[i];
        }
    }
    return NULL;
}

VerilogModule * VerilogModule::clone()
{
    VerilogModule *new_module;
    new_module = new VerilogModule(this->name);
    for(int i=0;i<instances.size();i++) {
        new_module->instances.push_back(instances[i]->clone());
    }
    for(int i=0;i<nets.size();i++) {
        VerilogNet *new_net = nets[i]->clone();
        new_net->parent = new_module;
        new_module->nets.push_back(new_net);
    }
    for(int i=0;i<assignments.size();i++) {
        new_module->assignments.push_back(assignments[i]->clone());
    }

    return new_module;
}


/* This method goes over all instantiations, and clones a VerilogModule
   for each of them if a module instance, or clones a LibertyCell if
   it is a cell instance.
   Then all pointers are updated.

   It is recursive - meant to be run on the top_module only.
*/
int VerilogModule::populate()
{
    bool anyfailing = 0;
    for(int i=0;i<instances.size();i++) {
        if(instances[i]->associated_module != NULL) {
            VerilogModule * newModule = instances[i]->associated_module->clone();
            instances[i]->associated_module = newModule;
            newModule->instance = instances[i];
            newModule->parent = this;
            submodules.push_back(newModule);
            if(newModule->populate() != 0) anyfailing = 1;
        }
        else if(instances[i]->associated_cell != NULL) {
            LibertyCell * cell_instance = new LibertyCell(instances[i]->associated_cell->name);
            for(int j=0;j<instances[i]->associated_cell->pins.size();j++){
                LibertyPin * new_pin = new LibertyPin(instances[i]->associated_cell->pins[j]->name);
                new_pin->parent = instances[i]->associated_cell->pins[j];
                new_pin->cell = cell_instance;
                new_pin->output = instances[i]->associated_cell->pins[j]->output;
                new_pin->input = instances[i]->associated_cell->pins[j]->input;
                cell_instance->pins.push_back(new_pin);
            }
            cell_instance->instance = instances[i];
            cell_instance->parent = this;
            cell_instance->is_missing = instances[i]->associated_cell->is_missing;
            cells.push_back(cell_instance);
            instances[i]->associated_cell = cell_instance;
        }
    }
    if (anyfailing)
        return -1;
    else
        return 0;
}


/* This method goes over all connections in all instances, and makes
   sure all pointers in the C++ objects are updated so that it becomes
   easy to traverse the design afterwards.

   It is recursive - meant to be run on the top_module only.
*/
int VerilogModule::hook_up_ports()
{
    if(DEBUG) std::cout << "\n\nHook_up_ports in " << *name << ":\n\n";
    bool anyfailing=0;
    for(int i=0;i<instances.size();i++) {
        if(instances[i]->associated_module != NULL || instances[i]->associated_cell != NULL) {
            for(int j=0;j<instances[i]->connections.size();j++) {
                bool found = 0;
                if(instances[i]->connections[j]->from != NULL) { // from == NULL means a constant
                    for(int k=0;k<nets.size();k++) {
                        if(nets[k]->name->compare(*(instances[i]->connections[j]->from)) == 0 && nets[k]->range_from == instances[i]->connections[j]->from_range_from && nets[k]->range_to == instances[i]->connections[j]->from_range_to) {
                            found = 1;
                            instances[i]->connections[j]->net_up = nets[k];
                            nets[k]->connections_down.push_back(instances[i]->connections[j]);
                            if(DEBUG) std::cout << "Hooking up connection \"" << *instances[i]->connections[j] << "\" up to net \"" <<  *nets[k] << "\"" << std::endl;
                            break;
                        }
                    }
                    if(!found) {
                        if(instances[i]->connections[j]->from_range_from == 0 && instances[i]->connections[j]->from_range_to == 0) {
                            if(DEBUG) std::cout << "Did not find net called " << *(instances[i]->connections[j]->from) << ", but creating it implicitly" << std::endl;
                            VerilogNet *new_net = new VerilogNet(instances[i]->connections[j]->from);
                            nets.push_back(new_net);
                            instances[i]->connections[j]->net_up = new_net;
                            new_net->connections_down.push_back(instances[i]->connections[j]);
                            if(DEBUG) std::cout << "Hooking up connection \"" << *instances[i]->connections[j] << "\" up to net \"" <<  *new_net << "\"" << std::endl;
                        }
                        else {
                            std::cout << "Warning: Did not find any nets called " << *(instances[i]->connections[j]->from) << "[" << instances[i]->connections[j]->from_range_from << "] in " << *name << std::endl;
                            anyfailing = 1;
                        }
                    }
                }
                found = 0;
                if(instances[i]->associated_cell != NULL) {
                    for(int k=0;k<instances[i]->associated_cell->pins.size();k++) {
                        LibertyPin *p = instances[i]->associated_cell->pins[k];
                        if(1) {
                            if(p->name->compare(*(instances[i]->connections[j]->to)) == 0 && (instances[i]->connections[j]->to_range_from == p->number) && ((instances[i]->connections[j]->to_range_to == p->number)) ) {
                                found = 1;
                                instances[i]->connections[j]->pin_down = p;
                                p->connection_up = instances[i]->connections[j];
                                if(DEBUG) std::cout << "Hooking up connection \"" << *instances[i]->connections[j] << "\" down to pin \"" <<  *p << "\" in cell " << *instances[i]->associated_cell->name << "" << std::endl;
                                break;
                            }
                        }
                    }
                    if(!found) {
                        if(instances[i]->connections[j]->to->compare(std::string("vdd")) != 0 && instances[i]->connections[j]->to->compare(std::string("gnd")) != 0 && instances[i]->connections[j]->to->compare(std::string("VDD")) != 0 && instances[i]->connections[j]->to->compare(std::string("VSS")) != 0) { // FIXME: Rather find in liberty which pins are power-pins

                            if(instances[i]->associated_cell->is_missing) {
                                LibertyPin *new_missing_pin = new LibertyPin(instances[i]->connections[j]->to);
                                new_missing_pin->number = instances[i]->connections[j]->to_range_from;
                                instances[i]->associated_cell->pins.push_back(new_missing_pin);
                                new_missing_pin->cell = instances[i]->associated_cell;
                                new_missing_pin->input = 1;  // XXX: Really wanted?
                                new_missing_pin->output = 1; // XXX: Really wanted?
                                std::cout << "Creating new pin and connecting for " << *instances[i]->connections[j]->to << " number " << new_missing_pin->number << std::endl;
                                instances[i]->connections[j]->pin_down = new_missing_pin;
                                new_missing_pin->connection_up = instances[i]->connections[j];
                            }
                            else {
                                std::cout << "Warning: Did not find any nets called " << *(instances[i]->connections[j]->to) << "[" << instances[i]->connections[j]->to_range_from << "] in cell " << *instances[i]->name << std::endl;
                                anyfailing = 1;
                            }
                        }
                    }
                }
                else if(instances[i]->associated_module != NULL) {
                    for(int k=0;k<instances[i]->associated_module->nets.size();k++) {
                        VerilogNet *n = instances[i]->associated_module->nets[k];
                        if (n->input || n->output) {
                            if(n->name->compare(*(instances[i]->connections[j]->to)) == 0 && (n->range_from - n->range_shift) == instances[i]->connections[j]->to_range_from && (n->range_to - n->range_shift) == instances[i]->connections[j]->to_range_to) {
                                found = 1;
                                instances[i]->connections[j]->net_down = n;
                                n->connection_up = instances[i]->connections[j];
                                if(DEBUG) std::cout << "Hooking up connection \"" << *instances[i]->connections[j] << "\" down to net \"" <<  *n << "\" in instance " << *instances[i]->name << " of module " << *instances[i]->associated_module->name << std::endl;
                                break;
                            }
                        }
                    }
                    if(!found) {
                        std::cout << "Warning: Did not find any nets called " << *(instances[i]->connections[j]->to) << "[" << instances[i]->connections[j]->to_range_from << "] in submodule " << *instances[i]->name << std::endl;
                        anyfailing = 1;
                    }
                }
            }
            if (instances[i]->associated_module != NULL)
                if(instances[i]->associated_module->hook_up_ports() != 0) anyfailing = 1;
        }
    }
    // Hook up assignments
    VerilogNet *from_net;
    VerilogNet *to_net;
    for(int i=0;i<assignments.size();i++) {
        from_net = NULL;
        to_net = NULL;
        for(int j=0;j<nets.size();j++) {
            if(assignments[i]->from_name->compare(*nets[j]->name) == 0 && nets[j]->range_to == assignments[i]->from_index) {
                from_net = nets[j];
                break;
            }
        }
        for(int j=0;j<nets.size();j++) {
            if(assignments[i]->to_name->compare(*nets[j]->name) == 0 && nets[j]->range_to == assignments[i]->to_index) {
                to_net = nets[j];
                break;
            }
        }
        if(from_net != NULL && to_net != NULL) {
            if(DEBUG) std::cout << "Hooking up assigned from \"" << *from_net << "\" to \"" <<  *to_net << "\"" << std::endl;
            from_net->assigned_to.push_back(assignments[i]);
            to_net->assigned_from = assignments[i];
            assignments[i]->from = from_net;
            assignments[i]->to = to_net;
        }
    }
    if(DEBUG) std::cout << "END" << std::endl;

    if (anyfailing)
        return -1;
    else
        return 0;
}


/* This methods splits any nets wider than 1, so that all nets afterwards
   have a width of 1, which is necessary for further operations.
*/
void VerilogModule::split_nets()
{
    int nets_size = nets.size();
    for(int i=0;i<nets_size;i++) {
        if(nets[i]->range_to > 0 && nets[i]->range_from >= 0 && nets[i]->range_to > nets[i]->range_from){
            int from_start = nets[i]->range_from;
            int from_len = nets[i]->range_to - nets[i]->range_from + 1;
            for(int j=1;j<from_len;j++) {
                VerilogNet *n = new VerilogNet(nets[i]->name);
                n->range_from = from_start+j;
                n->range_to   = from_start+j;
                n->input      = nets[i]->input;
                n->output     = nets[i]->output;
                n->range_shift = from_start;
                nets.push_back(n);
            }
            nets[i]->range_from = from_start;
            nets[i]->range_to = from_start; //set_range(from_start, from_start);
        }
    }
}


/* This method returns the hierarchical path up until
   this module instance
*/
std::string VerilogModule::get_path()
{
    std::string name;
    std::string parent_name;
    if(this->parent != NULL) {
        parent_name = this->parent->get_path();
        if(parent_name.compare("") != 0) {
            parent_name = parent_name + "/";
        }
    }
    else {
        parent_name = "";
    }
    if(this->instance != NULL) {
        if(this->instance->name != NULL)
            name = *this->instance->name;
        else
            name = "(unknown)";
    }
    else {
        name = "";
    }
    return parent_name + "" + name;
}

std::ostream & operator<<(std::ostream& os, const VerilogModule& obj) {
    os << (*obj.name) << std::endl;
    os << "Nets:" << std::endl;
    for(int i=0;i<obj.nets.size();i++) {
        os << "  " << (*obj.nets[i]) << std::endl;
    }
    os << "Sub-modules:" << std::endl;
    for(int i=0;i<obj.submodules.size();i++) {
        os << (*obj.submodules[i]) << std::endl;
    }
    return os;
}

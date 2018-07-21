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
#include <cstdlib>
#include "verilog_design.h"

VerilogDesign::VerilogDesign()
{
    last_added_module = NULL;
    last_range_element = NULL;
}

VerilogModule *VerilogDesign::add_module(std::string *name)
{
    if (last_added_module == NULL) {
        last_added_module = new VerilogModule(name);
        modules.push_back(last_added_module);
        module_names.push_back(name);
    }
    else {
        last_added_module = last_added_module->add_submodule(name);
    }
    module_stack.push(last_added_module);
    return last_added_module;
}

VerilogModule *VerilogDesign::pop_module()
{
    VerilogModule *m;

    m = NULL;
    if (!module_stack.empty()) {
        module_stack.pop();
        if (!module_stack.empty()) {
            m = module_stack.top();
            last_added_module = m;
        }
        else {
            last_added_module = NULL;
        }
    }
    else {
        last_added_module = NULL;
    }

    return m;
}


VerilogNet *VerilogDesign::add_net_to_last(std::string *name)
{
    if (last_added_module != NULL) {
        signal_names.push_back(name); // TODO: Also look if already in list to save memory
        VerilogNet *n;
        n = last_added_module->get_net(name);
        if (n == NULL) {
            n = last_added_module->add_net(name);
        }
        else
            last_added_module->last_added_net = n;
        last_range_element = (RangeType*)n;
        return n;
    }
    return NULL;
}

VerilogNet *VerilogDesign::add_net_to_last()
{
    if (last_added_module != NULL) {
        VerilogNet *n = last_added_module->add_net();
        last_range_element = (RangeType*)n;
        return n;
    }
    return NULL;
}

void VerilogDesign::set_net_name_to_last(std::string *name)
{
    if (last_added_module->last_added_net != NULL) {
        signal_names.push_back(name); // TODO: Also look if already in list to save memory
        last_added_module->last_added_net->set_name(name);
    }
}

VerilogInstance *VerilogDesign::add_instance_to_last(std::string *module_name, std::string *name)
{
    if (last_added_module != NULL) {
        instance_names.push_back(module_name); // TODO: Also look if already in list to save memory
        instance_names.push_back(name); // TODO: Also look if already in list to save memory
        return last_added_module->add_instance(module_name, name);
    }
    return NULL;
}

VerilogConnection *VerilogDesign::add_connection_to_last(std::string *from, std::string *to)
{
    if (last_added_module != NULL && last_added_module->last_added_instance != NULL) {
        bool found = 0;
        for(int i=0;i<signal_names.size();i++) {
            if(to->compare(*(this->signal_names[i])) == 0) {
                delete(to);
                to = this->signal_names[i];
                found = 1;
                break;
            }
        }
        if (!found) {
            signal_names.push_back(to);
        }
        found = 0;
        for(int i=0;i<signal_names.size();i++) {
            if(from->compare(*(this->signal_names[i])) == 0) {
                delete(from);
                from = this->signal_names[i];
                found = 1;
                break;
            }
        }
        if (!found) {
            signal_names.push_back(from);
        }
        VerilogNet *n = last_added_module->get_net(from);
        VerilogConnection *p;
        p = last_added_module->last_added_instance->add_connection(from, to);
        last_range_element = (RangeType*)p;
        if (n != NULL) {
            p->set_range(n->range_from, n->range_to, 0, n->range_to-n->range_from);
            if((n->range_to - n->range_from) > 0)
                p->indices = 1;
        }
        return p;
    }
    return NULL;
}

VerilogConnection *VerilogDesign::add_connection_to_last(VerilogConnection *conn)
{
    if (last_added_module != NULL && last_added_module->last_added_instance != NULL) {
        VerilogConnection *p = last_added_module->last_added_instance->add_connection(conn);
        last_range_element = (RangeType*)p;
        return p;
    }
    return NULL;
}

VerilogConnection *VerilogDesign::add_connection_to_last_constant(std::string *constant_desc, std::string *to)
{
    //std::cout << "Adding const: " << *constant_desc << std::endl;
    if (last_added_module != NULL && last_added_module->last_added_instance != NULL) {
        char tmpbeginning [20];
        const char * cstr = constant_desc->c_str();
        const char * lastpart = strchr(cstr, '\'');
        strncpy(tmpbeginning, cstr, lastpart - cstr);
        tmpbeginning[lastpart - cstr] = 0;
        int width = atoi(tmpbeginning);
        char data_type = *(lastpart+1);
        int val;
        if (data_type == 'd') {
            val = atoi(lastpart+2);
        }
        else if (data_type == 'h') {
            val = strtol(lastpart+2, NULL, 16);
        }
        else if (data_type == 'b') {
            val = strtol(lastpart+2, NULL, 2);
        }
        VerilogConnection *p = last_added_module->last_added_instance->add_connection(width, val, to);
        //last_range_element = (RangeType*)p;
        return p;
    }
    return NULL;
}

VerilogConnection *VerilogDesign::add_connection_to_last_constant(int intval, std::string *to)
{
    //std::cout << "Adding const int: " << intval << std::endl;
    if (last_added_module != NULL && last_added_module->last_added_instance != NULL) {
        VerilogConnection *p = last_added_module->last_added_instance->add_connection(1, intval, to);
        //last_range_element = (RangeType*)p;
        return p;
    }
    return NULL;
}

VerilogAssignment * VerilogDesign::add_assignment_to_last(std::string *from, int from_index, std::string *to, int to_index)
{
    if(last_added_module != NULL) {
        last_added_module->add_assignment(from, from_index, to, to_index);
    }
}


void VerilogDesign::set_range_to_last(int from, int to)
{
    if (last_range_element != NULL && to != -1) {
        last_range_element->set_range(from, to);
    }
}

void VerilogDesign::set_end_instance()
{
    if (last_added_module != NULL) {
        last_added_module->last_added_instance = NULL;
    }
}

void VerilogDesign::set_end_net()
{
    if (last_added_module != NULL) {
        last_added_module->last_added_net = NULL;
    }
    last_range_element = NULL;
}

void VerilogDesign::set_end_connection()
{
    if (last_added_module != NULL && last_added_module->last_added_instance != NULL) {
        last_added_module->last_added_instance->last_added_connection = NULL;
    }
    last_range_element = NULL;
}

void VerilogDesign::set_last_net_input()
{
    if (last_added_module != NULL && last_added_module->last_added_net != NULL) {
        last_added_module->last_added_net->input = 1;
    }
}

void VerilogDesign::set_last_net_output()
{
    if (last_added_module != NULL && last_added_module->last_added_net != NULL) {
        last_added_module->last_added_net->output = 1;
    }
}

void VerilogDesign::cleanup()
{
    for(int i=0;i<modules.size();i++) {
        delete modules[i];
    }
    modules.clear();

    for(int i=0;i<signal_names.size();i++) {
        delete signal_names[i];
        signal_names[i] = NULL;
    }
    signal_names.clear();

    for(int i=0;i<instance_names.size();i++) {
        delete instance_names[i];
        instance_names[i] = NULL;
    }
    instance_names.clear();

    for(int i=0;i<module_names.size();i++) {
        delete module_names[i];
        module_names[i] = NULL;
    }
    module_names.clear();
}

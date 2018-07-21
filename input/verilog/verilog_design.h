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

#ifndef INC_VERILOG_DESIGN_H
#define INC_VERILOG_DESIGN_H

#include <string>
#include <vector>
#include <stack>
#include "verilog_module.h"
#include "verilog_net.h"
#include "verilog_instance.h"
#include "verilog_connection.h"
#include "range_type.h"

class LibertyPin;
class LibertyCell;

class VerilogDesign {

  public:
    VerilogDesign();
    VerilogModule * add_module(std::string *name);
    VerilogModule * pop_module();
    VerilogNet * add_net_to_last(std::string *name);
    VerilogNet * add_net_to_last();
    void set_net_name_to_last(std::string *);
    VerilogInstance * add_instance_to_last(std::string *module_name, std::string *name);
    VerilogConnection * add_connection_to_last(std::string *from, std::string *to);
    VerilogConnection * add_connection_to_last(VerilogConnection *conn);
    VerilogConnection * add_connection_to_last_constant(std::string *constant_desc, std::string *to);
    VerilogConnection * add_connection_to_last_constant(int intval, std::string *to);
    VerilogAssignment * add_assignment_to_last(std::string *from, int from_index, std::string *to, int to_index);
    void set_range_to_last(int from, int to);
    void set_end_instance();
    void set_end_net();
    void set_end_connection();

    void set_last_net_input();
    void set_last_net_output();

    void cleanup();


    VerilogModule *last_added_module;

    std::vector<VerilogModule*> modules;

    std::stack<VerilogModule*> module_stack;

    std::vector<std::string*> signal_names;

    std::vector<std::string*> instance_names;
    std::vector<std::string*> module_names;


    RangeType *last_range_element;
};

#endif

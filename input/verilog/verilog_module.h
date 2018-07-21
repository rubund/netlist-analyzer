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

#ifndef INC_VERILOG_MODULE_H
#define INC_VERILOG_MODULE_H

#include <string>
#include <vector>

#include "verilog_net.h"

class VerilogInstance;
class LibertyCell;
class VerilogAssignment;

class VerilogModule {


  public:
    VerilogModule(std::string *name);
    ~VerilogModule();
    VerilogNet *add_net(std::string *name);
    VerilogNet *add_net();
    VerilogNet *get_net(std::string *name);
    VerilogModule *add_submodule(std::string *name);
    VerilogInstance *add_instance(std::string *module_name, std::string *name);
    VerilogAssignment *add_assignment(std::string *from, int from_index, std::string *to, int to_index);
    VerilogModule *clone();
    int populate();
    int hook_up_ports();
    void split_nets();
    std::string get_path();
    void cleanup();

    std::string *name;
    std::vector<VerilogNet*> nets;
    std::vector<VerilogModule*> submodules;
    std::vector<VerilogInstance*> instances;
    std::vector<VerilogAssignment*> assignments;
    std::vector<LibertyCell*> cells;

    VerilogNet  *last_added_net;
    VerilogModule *last_added_submodule;
    VerilogInstance *last_added_instance;
    VerilogAssignment *last_added_assignment;


    VerilogInstance *instance;
    VerilogModule *parent;
};

std::ostream& operator<<(std::ostream&, const VerilogModule&);

#endif

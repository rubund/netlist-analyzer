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

#ifndef INC_VERILOG_INSTANCE_H
#define INC_VERILOG_INSTANCE_H

#include <string>
#include <vector>
#include <iostream>


class VerilogModule;
class VerilogConnection;
class LibertyCell;

/* This class represents one instantiation in one
   module.
*/
class VerilogInstance {

  public:
    VerilogInstance(std::string *module_name, std::string *name);
    VerilogConnection *add_connection(std::string *from, std::string *to);
    VerilogConnection *add_connection(VerilogConnection *conn);
    VerilogConnection *add_connection(int const_width, int const_val, std::string *to);
    void save_memory();
    void split_connections();
    VerilogInstance *clone();
    void cleanup();

    std::string *name;
    std::string *module_name;
    std::vector<VerilogConnection*> connections;
    std::vector<VerilogInstance*> instances;

    VerilogConnection *last_added_connection;

    VerilogModule *associated_module;
    LibertyCell *associated_cell;
};

std::ostream& operator<<(std::ostream&, const VerilogInstance&);

#endif

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


#include "verilog_instance.h"
#include "verilog_connection.h"
#include "verilog_module.h"
#include "../../input/liberty/liberty_cell.h"

VerilogInstance::VerilogInstance(std::string *module_name, std::string *name)
{
    last_added_connection = NULL;
    associated_module = NULL;
    associated_cell = NULL;
    this->name = name;
    this->module_name = module_name;
}

VerilogConnection * VerilogInstance::add_connection(std::string *from, std::string *to)
{
    last_added_connection = new VerilogConnection(from, to);
    connections.push_back(last_added_connection);
    return last_added_connection;
}

VerilogConnection * VerilogInstance::add_connection(VerilogConnection *conn)
{
    last_added_connection = conn;
    connections.push_back(last_added_connection);
    return last_added_connection;
}

VerilogConnection * VerilogInstance::add_connection(int const_width, int const_val, std::string *to)
{
    last_added_connection = new VerilogConnection(const_width, const_val, to);
    connections.push_back(last_added_connection);
    return last_added_connection;
}

void VerilogInstance::save_memory()
{ // This causes problems when cleaning up in the end. FIXME
    //if (associated_cell != NULL){
    //    delete(this->module_name);
    //    this->module_name = associated_cell->name;
    //}
    //else if (associated_module != NULL){
    //    delete(this->module_name);
    //    this->module_name = associated_module->name;
    //}
}

void VerilogInstance::split_connections()
{
    int connections_size = connections.size();
    for(int i=0; i < connections_size ; i++){
        if (connections[i]->is_range()) {
            int from_start = connections[i]->from_range_from;
            int from_len = connections[i]->from_range_to - connections[i]->from_range_from + 1;
            int to_start = connections[i]->to_range_from;
            for(int j=1;j<from_len;j++) {
                VerilogConnection *c = new VerilogConnection(connections[i]->from, connections[i]->to);
                c->set_range(from_start+j, from_start+j, to_start+j, to_start+j);
                c->is_constant = connections[i]->is_constant;
                c->const_val = (connections[i]->const_val >> j-from_start) & 0x01;
                connections.push_back(c);
            }
            connections[i]->const_val = (connections[i]->const_val) & 0x01;
            connections[i]->set_range(from_start, from_start, to_start, to_start);
        }

    }
}

VerilogInstance *VerilogInstance::clone()
{
    VerilogInstance *newInstance = new VerilogInstance(this->module_name, this->name);

    for(int i=0;i<this->connections.size();i++) {
        newInstance->connections.push_back(this->connections[i]->clone());
    }

    newInstance->associated_module = this->associated_module;
    newInstance->associated_cell   = this->associated_cell;

    return newInstance;
}

std::ostream & operator<<(std::ostream& os, const VerilogInstance& obj) {
    os << "  " << (*obj.module_name) << " " << (*obj.name) << std::endl;
    for(int i=0;i<obj.connections.size();i++) {
        os << "    " << (*obj.connections[i]) << std::endl;
        //os << "    " << (*obj.connections[i]->to) << " <-> " << (*obj.connections[i]->from) << std::endl;
    }
    return os;
}

void VerilogInstance::cleanup()
{
    for(int i=0;i<connections.size();i++) {
        delete connections[i];
    }
    connections.clear();
}

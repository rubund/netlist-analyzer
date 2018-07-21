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
#include <cstring>
#include <cstdlib>
#include "verilog_connection.h"
#include "connection_builder.h"

ConnectionBuilder::ConnectionBuilder()
{
    this->init();
    active = 0;
}

void ConnectionBuilder::init()
{
    active = 1;
    this->cleanup();
}

ConnectionBuilder::~ConnectionBuilder()
{
    this->cleanup();
}

void ConnectionBuilder::cleanup()
{
    to_name = NULL;
    connections.clear();
    strings.clear();
    from.clear();
    to.clear();
    constants.clear();
}

void ConnectionBuilder::generate_connections()
{
    int cnt = 0;
    if(connections.size() == 0) {
        for(int i=strings.size()-1 ; i >= 0 ; i--) {
            if(this->strings[i] != NULL) {
                VerilogConnection *conn = new VerilogConnection(this->strings[i], this->to_name);
                int diff = this->to[i] - this->from[i];
                if(1) {
                    conn->set_range(this->from[i], this->to[i], cnt, diff+cnt);
                }
                cnt += diff + 1;
                connections.push_back(conn);
            }
            else {
                VerilogConnection *conn = new VerilogConnection(this->to[i] - this->from[i] + 1, this->constants[i], this->to_name);
                int diff = this->to[i] - this->from[i];
                if(1) {
                    conn->set_range(this->from[i], this->to[i], cnt, diff+cnt);
                }
                cnt += diff + 1;
                connections.push_back(conn);
            }
        }
    }
}

void ConnectionBuilder::set_to_name(std::string *s)
{
    this->to_name = s;
}

void ConnectionBuilder::add_from(std::string *n, int from, int to)
{
    this->strings.push_back(n);
    this->from.push_back(from);
    this->to.push_back(to);
    this->constants.push_back(0);
}

void ConnectionBuilder::add_from(std::string *n)
{
    //if (this->to_name != NULL)
    //    std::cout << "Adding complex connection: " << (*this->to_name) << " " << *n << std::endl;
    //else
    //    std::cout << "Adding complex connection: " << *n << std::endl;
    this->strings.push_back(n);    
    this->from.push_back(-1);
    this->to.push_back(-1);
    this->constants.push_back(0);
}

void ConnectionBuilder::add_from_constant(std::string *constant_desc)
{
    //std::cout << "Adding constant to connection builder " << *constant_desc << std::endl;
    this->strings.push_back(NULL);
    this->from.push_back(0);

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

    this->to.push_back(width-1);
    this->constants.push_back(val);
}

void ConnectionBuilder::add_from_constant(int intval)
{
    //std::cout << "Adding constant int to connection builder " << intval << std::endl;
    this->strings.push_back(NULL);
    this->from.push_back(0);
    this->to.push_back(0);
    this->constants.push_back(intval);
}

void ConnectionBuilder::set_range_on_last(int from, int to)
{
    if(from == -1)
        this->from[this->from.size()-1] = to;
    else
        this->from[this->from.size()-1] = from;
    this->to[this->to.size()-1] = to;
}

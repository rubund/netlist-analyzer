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
#include "verilog_connection.h"

VerilogConnection::VerilogConnection(std::string *from, std::string *to)
{
    this->from_range_from = 0;
    this->from_range_to = 0;
    this->to_range_from = 0;
    this->to_range_to = 0;
    this->from = from;
    this->to = to;
    this->net_up = NULL;
    this->net_down = NULL;
    this->pin_down = NULL;
    this->indices = 0;
    this->is_constant = 0;
}

VerilogConnection::VerilogConnection(int const_width, int const_val, std::string *to)
{
    this->from_range_from = 0;
    this->from_range_to = 0;
    this->to_range_from = const_width-1;
    this->to_range_to = const_width-1;
    this->from = NULL;
    this->to = to;
    this->net_up = NULL;
    this->net_down = NULL;
    this->pin_down = NULL;
    if(const_width > 1)
        this->indices = 1;
    else
        this->indices = 0;
    this->is_constant = 1;
    this->const_val = const_val;
}

VerilogConnection::~VerilogConnection()
{
}

void VerilogConnection::set_range(int from_range_from, int from_range_to)
{
    if(from_range_from != -1) {
        this->from_range_from = from_range_from;
    }
    else if (from_range_to != -1){
        this->from_range_from = from_range_to;
    }
    if(from_range_to != -1) {
        this->from_range_to = from_range_to;
        this->to_range_from = 0;
        this->to_range_to = this->from_range_to - this->from_range_from;
        if(this->to_range_to > 0)
            this->indices = 1;
    }
}

void VerilogConnection::set_range(int from_range_from, int from_range_to, int to_range_from, int to_range_to)
{
    set_range(from_range_from, from_range_to);
    if(to_range_from != -1)
        this->to_range_from = to_range_from;
    else if (to_range_to != -1)
        this->to_range_from = to_range_to;
    if(to_range_to != -1)
        this->to_range_to = to_range_to;
}

bool VerilogConnection::is_range()
{
    bool val;
    val = (bool)((from_range_to - from_range_from) > 0);
    return val;
}

VerilogConnection *VerilogConnection::clone()
{
    VerilogConnection *c;
    c = new VerilogConnection(from, to);
    c->set_range(this->from_range_from, this->from_range_to, this->to_range_from, this->to_range_to);
    c->indices = this->indices;
    c->is_constant = this->is_constant;
    c->const_val = this->const_val;
    return c;
}


std::ostream & operator<<(std::ostream& os, const VerilogConnection& obj) {
    if(obj.to_range_to != 0)
        os << (*obj.to) << "[" << (obj.to_range_to) << ":" << (obj.to_range_from) << "] " << " <-> ";
    else
        os << (*obj.to) << " <-> ";

    if(obj.from != NULL) {
        if(obj.from_range_to != 0 || obj.indices)
            os << (*obj.from) << "[" << (obj.from_range_to) << ":" << (obj.from_range_from) << "]";
        else
            os << (*obj.from);
    }
    else {
        os << "const " << (obj.const_val) << "[" << (obj.from_range_to) << ":" << (obj.from_range_from) << "]";
    }
    return os;
}

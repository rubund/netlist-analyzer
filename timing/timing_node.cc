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

#include <ostream>
#include <sstream>
#include "timing_node.h"

#include "../input/liberty/liberty_pin.h"
#include "../input/liberty/liberty_cell.h"
#include "../input/verilog/verilog_net.h"

TimingNode::TimingNode(EndNode *pin)
{
    this->pin = pin;
    LibertyPin *tmp_pin = NULL;
    VerilogNet *tmp_top_port = NULL;
    if (pin->get_type() == 1) {
        tmp_pin = (LibertyPin*) pin;
    }
    else if (pin->get_type() == 2) {
        tmp_top_port = (VerilogNet*) pin;
    }
    if(tmp_pin != NULL && tmp_pin->cell != NULL) {
        this->path = tmp_pin->cell->get_path() + "/" + *tmp_pin->name;
        this->cell_name = *tmp_pin->cell->name;
    }
    else if(tmp_top_port != NULL) {
        if (tmp_top_port->parent != NULL) {
            std::string parent_path = tmp_top_port->parent->get_path();
            if (parent_path != "")
                this->path = parent_path + "/" + *tmp_top_port->name;
            else
                this->path = *tmp_top_port->name;
        }
        else {
            this->path = *tmp_top_port->name;
        }
        this->cell_name = "<module pin>";
    }
    else {
        this->path = "(unknown)";
        this->cell_name = "UNKNOWN";
    }
    if(tmp_pin != NULL && tmp_pin->connection_up->net_up != NULL) {
        std::ostringstream os;
        os << *tmp_pin->connection_up->net_up;
        this->net_name = os.str();
    }
    else {
        this->net_name = *tmp_top_port->name;
    }
}

TimingNode::TimingNode(std::string path, VerilogNet *net)
{
    std::string nname;
    if(net != NULL) {
        std::ostringstream os;
        os << *net;
        nname = os.str();
    }
    else {
        nname = "";
    }
    if(path.compare("") == 0)
        this->path = nname;
    else
        this->path = path + "/" + nname;
    this->cell_name = "";
    this->net_name = net_name;
}

TimingNode::TimingNode(int special)
{
    if(NOT_UNIQUE) {
        this->path = "  *** Equal to previous ones ***";
        this->cell_name = "";
        this->net_name = "";
    }
}

std::ostream & operator<<(std::ostream& os, const TimingNode& obj) {
    os << obj.path << " (" << obj.cell_name << ") -   " << obj.net_name;
    return os;
}

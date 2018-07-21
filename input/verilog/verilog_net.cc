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
#include "verilog_net.h"
#include "verilog_connection.h"
#include "verilog_assignment.h"
#include "../liberty/liberty_pin.h"
#include "../liberty/liberty_cell.h"

VerilogNet::VerilogNet(std::string *name)
{

    this->name = name;
    this->range_from = 0;
    this->range_to = 0;
    this->input = 0;
    this->output = 0;
    this->connection_up = NULL;
    this->indices = 0;
    this->assigned_from = NULL;
    this->range_shift = 0;
    this->parent = NULL;
}

VerilogNet::VerilogNet()
{
    this->range_from = 0;
    this->range_to = 0;
    this->input = 0;
    this->output = 0;
    this->connection_up = NULL;
    this->indices = 0;
    this->assigned_from = NULL;
    this->range_shift = 0;
}

void VerilogNet::set_name(std::string *name)
{
    this->name = name;
}

void VerilogNet::set_range(int from, int to)
{
    if( (this->range_from == 0 && this->range_to == 0) || (from >= this->range_from && to >= this->range_to)) {
        if(from == -1) {
            this->range_from = 0;
        }
        else {
            this->range_from = from;
        }
        this->range_to = to;
        if (to > from)
            indices = 1;
    }
    else {
        std::cout << "Warning: conflicting width for " << (*this->name) << std::endl;
    }
    this->range_shift = range_from;
}

VerilogNet * VerilogNet::clone()
{
    VerilogNet *net = new VerilogNet(name);
    net->range_from = range_from;
    net->range_to = range_to;
    net->range_shift = range_shift;
    net->input = input;
    net->output = output;
    net->indices = indices;
    return net;
}

std::vector<EndNode*> * VerilogNet::get_consumers_except(VerilogConnection *ex)
{
    std::vector<EndNode*> * consumer_pins = new std::vector<EndNode*>();
    for(int i=0;i<connections_down.size();i++) {
        if(connections_down[i] != ex && connections_down[i]->net_down != NULL && connections_down[i]->net_down->input) {
            std::vector<EndNode*> * child;
            child = connections_down[i]->net_down->get_consumers_except(connections_down[i]);
            for(int j=0;j<child->size();j++){
                consumer_pins->push_back(child->at(j));
            }
            delete(child);
        }
        else if(connections_down[i] != ex && connections_down[i]->pin_down != NULL && connections_down[i]->pin_down->input) {
            //std::cout << "  Found consumer: " << connections_down[i]->pin_down->cell->get_path() << "/" << *connections_down[i]->pin_down->name << std::endl;
            consumer_pins->push_back((EndNode*)(connections_down[i]->pin_down));
        }
    }
    if(this->output && connection_up != NULL && connection_up != ex) {
        std::vector<EndNode*> * child;
        child = connection_up->net_up->get_consumers_except(connection_up);
        if(child->size() > 0) {
            for(int j=0;j<child->size();j++){
                consumer_pins->push_back(child->at(j));
            }
        }
        delete(child);
    }
    if(this->output && connection_up == NULL) {
        consumer_pins->push_back((EndNode*)this);
    }

    // Handle signals assigned
    for(int i=0;i<assigned_to.size();i++) {
        std::vector<EndNode*> * child;
        child = assigned_to[i]->to->get_consumers_except(NULL);
        for(int j=0;j<child->size();j++){
            consumer_pins->push_back(child->at(j));
        }
        delete(child);
    }

    return consumer_pins;
}

std::vector<EndNode*> * VerilogNet::get_producers_except(VerilogConnection *ex)
{
    std::vector<EndNode*> * producer_pins = new std::vector<EndNode*>();
    for(int i=0;i<connections_down.size();i++) {
        if(connections_down[i] != ex && connections_down[i]->net_down != NULL && connections_down[i]->net_down->output) {
            std::vector<EndNode*> * child;
            child = connections_down[i]->net_down->get_producers_except(connections_down[i]);
            for(int j=0;j<child->size();j++){
                producer_pins->push_back(child->at(j));
            }
            delete(child);
        }
        else if(connections_down[i] != ex && connections_down[i]->pin_down != NULL && connections_down[i]->pin_down->output) {
            //std::cout << "  Found consumer: " << connections_down[i]->pin_down->cell->get_path() << "/" << *connections_down[i]->pin_down->name << std::endl;
            producer_pins->push_back((EndNode*)connections_down[i]->pin_down);
        }
    }
    if(this->input && connection_up != NULL && connection_up != ex) {
        std::vector<EndNode*> * child;
        child = connection_up->net_up->get_producers_except(connection_up);
        for(int j=0;j<child->size();j++){
            producer_pins->push_back(child->at(j));
        }
        delete(child);
    }
    if(this->input && connection_up == NULL) {
        producer_pins->push_back((EndNode*)this);
    }

    // Handle signals assigned
    if(assigned_from != NULL) {
        std::vector<EndNode*> * child;
        child = assigned_from->from->get_producers_except(NULL);
        for(int j=0;j<child->size();j++){
            producer_pins->push_back(child->at(j));
        }
        delete(child);
    }

    if(producer_pins->size() > 1) {
        std::cout << "Warning: More than one driver of the net." << *this << std::endl;
    }


    return producer_pins;
}

double VerilogNet::get_load(char min)
{
    return 0.08; // FIXME: Should return real load
}

int VerilogNet::get_type()
{
    return 2;
}

std::ostream & operator<<(std::ostream& os, const VerilogNet& obj) {
    if(obj.range_from != 0 || obj.range_to != 0 || obj.indices)
        os << (*obj.name) << "[" << (obj.range_to) << ":" << (obj.range_from) << "]";
    else
        os << (*obj.name);
    if(obj.input)
        os << " input";
    if(obj.output)
        os << " output";
    return os;
}

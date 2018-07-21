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
#include <sstream>
#include <string>
#include "liberty_cell.h"
#include "../verilog/verilog_instance.h"
#include "../verilog/verilog_module.h"

LibertyCell::LibertyCell(std::string *name)
{
    last_added_pin = NULL;
    this->name = name;
    this->instance = NULL;
    this->parent = NULL;
    this->is_missing = 0;
}

LibertyCell::~LibertyCell()
{
    this->cleanup();
}

void LibertyCell::add_pin(std::string *name)
{
   last_added_pin = new LibertyPin(name);
   pins.push_back(last_added_pin);
}

std::string LibertyCell::get_path()
{
    std::string parent_name;
    if(this->parent != NULL) {
        parent_name = this->parent->get_path();
        if(parent_name.compare("") != 0)
            return parent_name + "/" + *instance->name;
        else
            return *instance->name;
    }
    else
        return "";
}

void LibertyCell::print_pins()
{
    std::string cell_path = this->get_path();
    for(std::vector<LibertyPin*>::const_iterator it = pins.begin() ; it != pins.end(); ++it) {
        std::cout << cell_path << "/" << (*it)->get_name() << std::endl;
    }

}


std::ostream & operator<<(std::ostream& os, const LibertyCell& obj) {
    os << (*obj.name) << std::endl;
    for(int i=0;i<obj.pins.size();i++) {
        os << "  " << (*obj.pins[i]->name) << std::endl;
    }
    return os;
}


void LibertyCell::cleanup()
{
    for(int i =0;i<pins.size();i++) {
        delete pins[i];
    }
}

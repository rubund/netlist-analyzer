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

#ifndef INC_LIBERTY_CELL_H
#define INC_LIBERTY_CELL_H

#include <string>
#include <vector>
#include <ostream>
#include "liberty_pin.h"
#include "../verilog/verilog_module.h"

class VerilogModule;
class VerilogInstance;

class LibertyCell {


  public:

    LibertyCell(std::string *name);
    ~LibertyCell();

    void add_pin(std::string *name);
    std::string get_path();
    void print_pins();
    void cleanup();

    LibertyPin *last_added_pin;
    std::string *name;
    std::vector<LibertyPin*> pins;
    bool is_missing;

    VerilogModule *parent;
    VerilogInstance *instance;
};

std::ostream& operator<<(std::ostream&, const LibertyCell&);

#endif

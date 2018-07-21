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

#ifndef INC_PROG_STATE_H
#define INC_PROG_STATE_H

#include <vector>
#include "input/verilog/verilog_design.h"
#include "input/liberty/liberty.h"
#include "constraints/constraints.h"

class LibertyPin;
class LibertyCell;
class VerilogNet;
class VerilogModule;

/* ProgramState contains the entire "state" of the program run
*/
class ProgramState {

  public:
    ProgramState();
    ~ProgramState();
    std::vector<VerilogDesign*> designs;

    std::vector<Liberty*> libraries;

    Liberty* missing_library;

    Constraints constraints;
    int create_clock(std::string *name, std::string *signal, float period, float half_period);

    void add_library(Liberty *);
    void add_design(VerilogDesign *);
    int link_design();
    void cleanup();


    LibertyPin * get_pin_liberty(const std::string&);
    LibertyCell * get_cell_liberty(std::string);
    VerilogNet * get_pin_verilog(std::string);
    VerilogModule * get_cell_verilog(std::string);

    int connect_modules();

    VerilogModule *top_module;

    bool has_linked;
};

#endif


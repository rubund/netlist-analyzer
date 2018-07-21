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

#ifndef INC_VERILOG_CONNECTION_H
#define INC_VERILOG_CONNECTION_H

#include <iostream>
#include <string>
#include "range_type.h"
#include "verilog_net.h"

class LibertyPin;


/* VerilogConnection represents ONE hook-up in an
   instantiation of a verilog module or a library cell
*/
class VerilogConnection : RangeType{

  public:
    VerilogConnection(std::string *from, std::string *to);
    VerilogConnection(int const_width, int const_val, std::string *to);
    ~VerilogConnection();
    void set_range(int, int);
    void set_range(int, int, int, int);
    bool is_range();
    VerilogConnection *clone();

    std::string *from;
    std::string *to;

    bool is_constant;
    int const_val;
    int from_range_from;
    int from_range_to;
    int to_range_from;
    int to_range_to;
    bool indices;

    VerilogNet *net_up;
    VerilogNet *net_down;
    LibertyPin *pin_down;
};

std::ostream& operator<<(std::ostream&, const VerilogConnection&);

#endif

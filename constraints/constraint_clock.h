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

#ifndef INC_CONSTRAINT_CLOCK_H
#define INC_CONSTRAINT_CLOCK_H

#include <string>
#include "../input/verilog/verilog_net.h"

class ConstraintClock {

  public:
    ConstraintClock(std::string *name, std::string *signal, float period, float half_period);

    std::string *name;
    std::string *signal;
    float period;
    float half_period;

    VerilogNet *net;

};

std::ostream & operator<<(std::ostream& os, const ConstraintClock& obj);

#endif


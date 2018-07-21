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

#ifndef INC_LUTABLE_H
#define INC_LUTABLE_H

#include <string>
#include <iostream>
#include <vector>
#include "lutable_numbers.h"


#define LUT_SPECIAL_LENGTH 4

#define LUT_CELL_RISE           0
#define LUT_RISE_TRANSITION     1
#define LUT_CELL_FALL           2
#define LUT_FALL_TRANSITION     3

class VerilogNet;

class LutTable {



  public:
    LutTable(std::string *name);
    ~LutTable();
    void add_table();
    void add_table(char, int);

    double get_value(double, double);
    void cleanup();

    std::string *name;

    std::vector<LutTableNumbers*> tables;
    LutTableNumbers *last_added_table;
};

std::ostream& operator<<(std::ostream&, const LutTable&);

#endif 

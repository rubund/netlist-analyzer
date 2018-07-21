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

#ifndef INC_LUTABLE_NUMBERS_H
#define INC_LUTABLE_NUMBERS_H

#include <iostream>

class LutTableNumbers {



  public:
    LutTableNumbers();
    LutTableNumbers(char, int);
    ~LutTableNumbers();

    void goto_next_row();
    void add_number(float);
    void print_table();
    void cleanup();

    float *vals;
    int rowlength;
    int count;
    char index_or_value;
    int index;
};

std::ostream& operator<<(std::ostream&, const LutTableNumbers&);

#endif 

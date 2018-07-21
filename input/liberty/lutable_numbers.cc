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

#include <cstdio>
#include "lutable_numbers.h"
#include <iostream>


LutTableNumbers::LutTableNumbers()
{
    //std::cout << "Heisann" << std::endl;
    index_or_value = -1;
    index = -1;
    vals = new float[200];
    rowlength = -1;
    count = 0;
}

LutTableNumbers::LutTableNumbers(char index_or_value, int index)
{
    //std::cout << "Heisann" << std::endl;
    index_or_value = -1;
    index = -1;
    vals = new float[200];
    rowlength = -1;
    count = 0;
    this->index_or_value = index_or_value;
    this->index = index;
}

LutTableNumbers::~LutTableNumbers()
{
    this->cleanup();
}

void LutTableNumbers::cleanup()
{
    delete vals;
}


void LutTableNumbers::goto_next_row()
{
    if(rowlength == -1) {
        rowlength = count; 
    }
}

void LutTableNumbers::add_number(float val)
{
    vals[count] = val;
    count++;
}

void LutTableNumbers::print_table()
{
    int columns = count/rowlength;

    printf("Table:\n");
    for(int i=0;i<columns;i++){
        for(int j=0;j<rowlength;j++) {
            printf("%f, ", vals[i*rowlength+j]);
        }
        printf("\n");
    }
}

std::ostream & operator<<(std::ostream& os, const LutTableNumbers& obj)
{
    int columns = obj.count/obj.rowlength;
    for(int i=0;i<columns;i++){
        for(int j=0;j<obj.rowlength;j++) {
            os << obj.vals[i*obj.rowlength+j];
            if((j < (obj.rowlength-1) ) || (i < (columns - 1))) os << ", ";
        }
        if (i < columns - 1)
            os << std::endl;
    }
    return os;
}

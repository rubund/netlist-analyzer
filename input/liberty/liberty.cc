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
#include <iostream>
#include "liberty.h"
#include "liberty_cell.h"

Liberty::Liberty()
{
    last_added_cell = NULL;
    to_find_direction = 0;
}

void Liberty::add_cell(std::string *name)
{
    last_added_cell = new LibertyCell(name);
    cells.push_back(last_added_cell);
    cell_names.push_back(name);
}


void Liberty::list_cells()
{
    for(int i=0;i<cells.size();i++) {
        printf("Cell:");
        std::cout << (*cells[i]);
    }    
}

void Liberty::add_pin_to_last(std::string *name)
{
    bool found = 0;
    if (last_added_cell != NULL) {
        for(int i = 0; i < this->pin_names.size() ; i++) {
            if(name->compare(*this->pin_names[i]) == 0) {
                delete (name);
                name = this->pin_names[i];
                found = 1;
                break;
            }
        }
        if(!found) {
            this->pin_names.push_back(name);
        }
        last_added_cell->add_pin(name);
    }
}

void Liberty::add_lut_to_last(std::string *name)
{
    bool found = 0;
    if (last_added_cell != NULL && last_added_cell->last_added_pin != NULL) {
        for(int i = 0; i < this->table_names.size() ; i++) {
            if(name->compare(*this->table_names[i]) == 0) {
                delete(name);
                name = this->table_names[i];
                found = 1;
                break;
            }
        }
        if(!found) {
            this->table_names.push_back(name);
        }
        last_added_cell->last_added_pin->add_lut(name);
    }
    else {
        delete(name);
    }
}

void Liberty::add_table_to_last(char index_or_value)
{
    if (last_added_cell != NULL && last_added_cell->last_added_pin != NULL && last_added_cell->last_added_pin->last_added_lut != NULL) {
        last_added_cell->last_added_pin->last_added_lut->add_table(index_or_value, 0);
    }
}

void Liberty::set_direction_to_last(std::string *val)
{
    if (to_find_direction) {
        if(last_added_cell != NULL && last_added_cell->last_added_pin != NULL) {
            if(val->compare("input") == 0) {
                last_added_cell->last_added_pin->input = 1;
            }
            else if(val->compare("output") == 0) {
                last_added_cell->last_added_pin->output = 1;
            }
        }
    }
}

void Liberty::set_end_lut()
{
    if (last_added_cell != NULL && last_added_cell->last_added_pin != NULL) {
        last_added_cell->last_added_pin->last_added_lut = NULL;
    }
}

void Liberty::set_end_pin()
{
    if (last_added_cell != NULL) {
        last_added_cell->last_added_pin = NULL;
    }
}

void Liberty::set_end_table()
{
    if (last_added_cell != NULL && last_added_cell->last_added_pin != NULL && last_added_cell->last_added_pin->last_added_lut != NULL) {
        last_added_cell->last_added_pin->last_added_lut->last_added_table = NULL;
    }
}

LutTableNumbers * Liberty::get_current_table()
{
    if (last_added_cell != NULL && last_added_cell->last_added_pin != NULL && last_added_cell->last_added_pin->last_added_lut != NULL && last_added_cell->last_added_pin->last_added_lut->last_added_table)
        return last_added_cell->last_added_pin->last_added_lut->last_added_table;
    else
        return NULL;
}

void Liberty::set_value_generic(std::string *keyword, int index, float value)
{
    if(keyword != NULL) {
        if(keyword->compare("rise_capacitance") == 0) {
            if(last_added_cell != NULL && last_added_cell->last_added_pin != NULL) {
                last_added_cell->last_added_pin->rise_capacitance = value;
            }
        }
        else if(keyword->compare("fall_capacitance") == 0) {
            if(last_added_cell != NULL && last_added_cell->last_added_pin != NULL) {
                last_added_cell->last_added_pin->fall_capacitance = value;
            }
        }
        else if(keyword->compare("rise_capacitance_range") == 0) {
            if(last_added_cell != NULL && last_added_cell->last_added_pin != NULL) {
                if(index < 2)
                    last_added_cell->last_added_pin->rise_capacitance_range[index] = value;
            }
        }
        else if(keyword->compare("fall_capacitance_range") == 0) {
            if(last_added_cell != NULL && last_added_cell->last_added_pin != NULL) {
                if(index < 2)
                    last_added_cell->last_added_pin->fall_capacitance_range[index] = value;
            }
        }
    }
}

void Liberty::cleanup()
{
    for(int i=0; i<cells.size();i++){
        delete cells[i];
    }
    for(int i=0; i<pin_names.size();i++){
        delete pin_names[i];
    }
    for(int i=0; i<cell_names.size();i++){
        delete cell_names[i];
    }
    cells.clear();
    pin_names.clear();
    cell_names.clear();

}

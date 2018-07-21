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

#ifndef INC_LIBERTY_H
#define INC_LIBERTY_H

#include <string>
#include <vector>
#include "liberty_cell.h"

/* This is a class representing one Liberty file
*/
class Liberty {

  public:
    std::vector<LibertyCell*> cells;

    Liberty();
    LibertyCell *last_added_cell;
    std::vector<std::string*> table_names;
    std::vector<std::string*> pin_names;
    std::vector<std::string*> cell_names;

    void add_cell(std::string *name);
    void list_cells();

    void add_pin_to_last(std::string *name);
    void add_lut_to_last(std::string *name);
    void add_table_to_last(char index_or_value);
    void set_direction_to_last(std::string *val);
    void set_value_generic(std::string *keyword, int index, float value);
    void set_end_lut();
    void set_end_pin();
    void set_end_table();
    LutTableNumbers *get_current_table();

    void cleanup();

    bool to_find_direction;
};

#endif

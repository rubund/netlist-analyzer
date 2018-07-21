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

#ifndef INC_CONNECTION_BUILDER_H
#define INC_CONNECTION_BUILDER_H

#include <string>
#include <vector>

class VerilogConnection;

class ConnectionBuilder {

  public:
    ConnectionBuilder();
    ~ConnectionBuilder();
    void init();
    void generate_connections();
    void cleanup();


    void set_to_name(std::string *name);
    void add_from(std::string *name, int from, int to);
    void add_from(std::string *name);
    void add_from_constant(std::string *const_desc);
    void add_from_constant(int ival);
    void set_range_on_last(int from, int to);


    int active;
    std::vector<VerilogConnection*> connections;
    std::string *to_name;

    std::vector<std::string *> strings;
    std::vector<int> constants;
    std::vector<int> from;
    std::vector<int> to;
};

#endif

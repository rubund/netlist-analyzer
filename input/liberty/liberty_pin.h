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

#ifndef INC_LIBERTY_PIN_H
#define INC_LIBERTY_PIN_H

#include <iostream>
#include <vector>
#include <string>

#include "lutable.h"
#include "../verilog/verilog_connection.h"
#include "../../endnode.h"

class VerilogConnection;
class LibertyCell;
class TimingPath;
class VerilogNet;

class LibertyPin : EndNode {

  public:
    LibertyPin(std::string *name);
    ~LibertyPin();
    void add_lut(std::string *name);

    LibertyPin * get_associated_output();
    std::vector<LibertyPin*> * get_associated_inputs();
    void get_receivers();
    void print_receivers(int);
    void print_fanin(int);
    std::vector<TimingPath*> * get_fanin(int level, TimingPath * in_path);
    std::vector<TimingPath*> * get_fanout(int level, TimingPath * in_path);
    std::vector<EndNode*> * get_downstream();
    EndNode* get_upstream();
    std::string get_name();
    VerilogNet * get_net();
    int get_type();
    void cleanup();

    double calculate_transition(double intrans, char sense, char min);
    double calculate_delay(double intrans, char sense, char min);

    std::string *name;
    int number;
    std::vector<LutTable*> luts;
    LutTable **special_luts;
    bool input;
    bool output;

    float rise_capacitance;
    float rise_capacitance_range[2];
    float fall_capacitance;
    float fall_capacitance_range[2];

    LutTable *last_added_lut;

    LibertyPin *parent;

    VerilogConnection *connection_up;

    LibertyCell *cell;



};

std::ostream& operator<<(std::ostream&, const LibertyPin&);

#endif

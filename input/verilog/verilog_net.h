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

#ifndef INC_VERILOG_NET_H
#define INC_VERILOG_NET_H

#include <vector>
#include <string>
#include "range_type.h"
#include "../../endnode.h"

class VerilogConnection;
class LibertyPin;
class VerilogAssignment;
class VerilogModule;


/* VerilogNet represents one net (or word) in one
   module. Input and output ports are also represented
   as VerilogNet with either the boolean variables input
   or output set
*/
class VerilogNet : RangeType, EndNode{

  public:
    VerilogNet(std::string *name);
    VerilogNet();
    void set_name(std::string *name);
    void set_range(int from, int to);
    VerilogNet *clone();
    std::vector<EndNode*> * get_consumers_except(VerilogConnection *ex);
    std::vector<EndNode*> * get_producers_except(VerilogConnection *ex);
    int get_type();

    double get_load(char min);

    std::string *name;
    int range_from;
    int range_to;
    int range_shift;
    bool input;
    bool output;
    bool indices;
    VerilogModule *parent;

    VerilogConnection *connection_up;
    std::vector<VerilogConnection*> connections_down;

    VerilogAssignment *assigned_from;
    std::vector<VerilogAssignment*> assigned_to;
};

std::ostream& operator<<(std::ostream&, const VerilogNet&);

#endif

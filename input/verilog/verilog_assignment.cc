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

#include <iostream>
#include "verilog_assignment.h"



VerilogAssignment::VerilogAssignment()
{
    this->from = NULL;
    this->to   = NULL;
    this->from_name  = NULL;
    this->to_name    = NULL;
    this->from_index = 0;
    this->to_index = 0;
}

VerilogAssignment * VerilogAssignment::clone()
{
    VerilogAssignment *ass = new VerilogAssignment();
    ass->from = this->from;
    ass->to   = this->to;
    ass->from_name = this->from_name;
    ass->to_name   = this->to_name;
    ass->from_index  = this->from_index;
    ass->to_index    = this->to_index;
}

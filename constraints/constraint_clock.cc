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
#include "constraint_clock.h"


ConstraintClock::ConstraintClock(std::string *name, std::string *signal, float period, float half_period)
{
    net = NULL;
    this->name = name;
    this->signal = signal;
    this->period = period;
    this->half_period = half_period;
}


std::ostream & operator<<(std::ostream& os, const ConstraintClock& obj)
{
    os << "Clock: " << *obj.name << " net " << *obj.signal << " with period " << obj.period << " (half period: " << obj.half_period << ")";
    return os;
}

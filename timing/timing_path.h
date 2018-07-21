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

#ifndef INC_TIMING_PATH_H
#define INC_TIMING_PATH_H


#include <list>

class TimingNode;

class TimingPath {


  public:
    TimingPath();
    ~TimingPath();
    void add_node_front(TimingNode *);
    void add_node_back(TimingNode *);
    TimingPath *clone();
    TimingPath *clone_from_end_n(int n);
    TimingPath *clone_from_begin_n(int n);
    std::list<TimingNode*> nodes;

    int length;

};

std::ostream& operator<<(std::ostream&, const TimingPath&);

#endif


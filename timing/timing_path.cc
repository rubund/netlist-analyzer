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

#include <ostream>
#include "timing_path.h"

#include "timing_node.h"

TimingPath::TimingPath()
{
    length = 0;
}

TimingPath * TimingPath::clone()
{
    TimingPath *new_path = new TimingPath();

    std::list<TimingNode*>::const_iterator iterator;
    for(iterator = nodes.begin() ; iterator != nodes.end() ; ++iterator) {
        new_path->add_node_back(*iterator);
    }
    new_path->length = this->length;

    return new_path;
}

TimingPath * TimingPath::clone_from_begin_n(int n)
{
    TimingPath *new_path = new TimingPath();

    std::list<TimingNode*>::const_iterator iterator;
    int i = 0;
    for(iterator = nodes.begin() ; iterator != nodes.end() ; ++iterator) {
        if (i == n) break;
        new_path->add_node_back(*iterator);
        i++;
    }
    new_path->length = n;

    return new_path;
}

TimingPath * TimingPath::clone_from_end_n(int n)
{
    TimingPath *new_path = new TimingPath();

    std::list<TimingNode*>::reverse_iterator iterator;
    int i = 0;
    for(iterator = nodes.rbegin() ; iterator != nodes.rend() ; ++iterator) {
        if (i == n) break;
        new_path->add_node_front(*iterator);
        i++;
    }
    new_path->length = n;

    return new_path;
}

TimingPath::~TimingPath()
{
    //std::list<TimingNode*>::iterator iterator;
    //for(iterator = nodes.begin() ; iterator != nodes.end() ; ++iterator) {
    //    if(*iterator != NULL)
    //        delete(*iterator);
    //    *iterator = NULL;
    //}

}

void TimingPath::add_node_front(TimingNode *node)
{
    nodes.push_front(node);
    this->length++;
}

void TimingPath::add_node_back(TimingNode *node)
{
    nodes.push_back(node);
    this->length++;
}

std::ostream& operator<<(std::ostream& os, const TimingPath& obj)
{
    os << "Timing path:" << std::endl;
    std::list<TimingNode*>::const_iterator iterator;
    for(iterator = obj.nodes.begin() ; iterator != obj.nodes.end() ; ++iterator) {
        os << "   " << **iterator << std::endl;
    }
    os << std::endl << std::endl;
    return os;
}



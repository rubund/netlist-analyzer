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

#include "lutable.h"
#include <iostream>


LutTable::LutTable(std::string *name)
{
    //std::cout << "Heisann" << std::endl;

    last_added_table = NULL;
    this->name = name;
}

LutTable::~LutTable()
{
    this->cleanup();
}

void LutTable::add_table()
{
    last_added_table = new LutTableNumbers();
    tables.push_back(last_added_table);
}

void LutTable::add_table(char index_or_value, int index)
{
    last_added_table = new LutTableNumbers(index_or_value, index);
    tables.push_back(last_added_table);
}

double LutTable::get_value(double trans, double net_load)
{
    float *index_trans;
    int len_index_trans;
    float *index_net_load;
    int len_index_net_load;

    if(tables.size() < 3) {
        std::cout << "Less than 3 rows in this LUT. Aborting.." << std::endl;
        return -1;
    }

    if(tables[0]->index_or_value == 1) {
        std::cout << "Erroneous table" << std::endl;
        return 0;
    }
    index_trans = tables[0]->vals;
    len_index_trans = tables[0]->count;
    if(tables[1]->index_or_value == 1) {
        std::cout << "Erroneous table" << std::endl;
        return 0;
    }
    index_net_load = tables[1]->vals;
    len_index_net_load = tables[1]->count;

    int i,j;

    if (trans < index_trans[0])
        i = 1;
    else if(trans >= index_trans[len_index_trans -1])
        i = len_index_trans -1;
    else {
        for (i =0 ; i < len_index_trans ; i++) {
            if (index_trans[i] > trans) break;
        }
    }

    double transition_fraction;
    transition_fraction = (trans - index_trans[i-1]) / (index_trans[i] - index_trans[i-1]);

    if (net_load < index_net_load[0])
        j = 1;
    else if(net_load >= index_net_load[len_index_net_load -1])
        j = len_index_net_load -1;
    else {
        for(j = 0; j < len_index_net_load ; j++) {
            if(index_net_load[j] > net_load) break;
        }
    }

    double net_load_fraction;
    net_load_fraction = (net_load - index_net_load[j-1]) / (index_net_load[j] - index_net_load[j-1]);
    double *tmp_table;
    double vlow, vhigh;

#ifdef DEBUG
    std::cout << "Net load fraction " << net_load_fraction << std::endl;
#endif


    tmp_table = new double[len_index_net_load];
    for (int k=0;k<len_index_trans;k++) {
        vlow  = tables[2]->vals[k*len_index_net_load+j-1];
#ifdef DEBUG
        std::cout << "vlow:  " << vlow << std::endl;
#endif
        vhigh = tables[2]->vals[k*len_index_net_load+j];
#ifdef DEBUG
        std::cout << "vhigh:  " << vlow << std::endl;
#endif
        tmp_table[k] = vlow + (vhigh - vlow) * net_load_fraction;
    }

#ifdef DEBUG
    std::cout << "Tmp table" << std::endl;
    for (int k=0;k<len_index_trans;k++) {
        std::cout << tmp_table[k] << ", " << std::endl;
    }
#endif

    double vlow_trans, vhigh_trans, value;

    vlow_trans  = *(tmp_table + (i-1));
    vhigh_trans = *(tmp_table + (i));
    value = vlow_trans + (vhigh_trans - vlow_trans) * transition_fraction;

    delete tmp_table;

    return value;
}

std::ostream & operator<<(std::ostream& os, const LutTable& obj)
{
    os << *obj.name << std::endl;
    for(int i=0;i<obj.tables.size();i++) {
        os << "[" << *obj.tables[i] << "]" << std::endl;
    }
    os << std::endl;
    return os;
}

void LutTable::cleanup()
{
    for(int i=0;i<tables.size();i++) {
        delete tables[i];
    }
    tables.clear();

}


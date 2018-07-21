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
#include <vector>
#include "liberty_pin.h"
#include "liberty_cell.h"
#include "../../timing/timing_path.h"
#include "../../timing/timing_node.h"

//#define DEBUG

extern std::vector<LibertyPin *> already_visited;
extern std::vector<TimingNode*> all_timing_nodes;

LibertyPin::LibertyPin(std::string *name)
{
    last_added_lut = NULL;
    this->name = name;
    this->number = 0;
    this->parent = NULL;
    this->cell = NULL;
    this->special_luts      = NULL;
    this->input = 0;
    this->output = 0;
    this->rise_capacitance = 0;
    this->rise_capacitance_range[0] = 0;
    this->rise_capacitance_range[1] = 0;
    this->fall_capacitance = 0;
    this->fall_capacitance_range[0] = 0;
    this->fall_capacitance_range[1] = 0;
    this->has_downstream = 0;
    this->upstream = NULL;
}

LibertyPin::~LibertyPin()
{
    this->cleanup();
}

void LibertyPin::add_lut(std::string *str)
{
    last_added_lut = new LutTable(str);
    luts.push_back(last_added_lut);   
    if(special_luts == NULL) {
        special_luts = new LutTable*[LUT_SPECIAL_LENGTH];
        for(int i=0;i<LUT_SPECIAL_LENGTH;i++) {
            special_luts[i] = NULL;
        }
    }
    if(str->compare(std::string("cell_rise")) == 0) {
        special_luts[LUT_CELL_RISE] = last_added_lut;
    }
    else if(str->compare(std::string("fall_transition")) == 0) {
        special_luts[LUT_FALL_TRANSITION] = last_added_lut;
    }
    else if(str->compare(std::string("cell_fall")) == 0) {
        special_luts[LUT_CELL_FALL] = last_added_lut;
    }
    else if(str->compare(std::string("rise_transition")) == 0) {
        special_luts[LUT_RISE_TRANSITION] = last_added_lut;
    }
}

//std::vector<LibertyPin*> * LibertyPin::get_receivers()
void LibertyPin::get_receivers()
{
    VerilogNet *n;
    std::vector<EndNode*> *consumers;

    if(this->connection_up != NULL) {
        n = this->connection_up->net_up;
        if(n != NULL) {
            consumers = n->get_consumers_except(this->connection_up);
            for(int i=0;i<consumers->size() ; i++) {
                LibertyPin *tmp_pin;
                if (consumers->at(i)->get_type() == 1) {
                    tmp_pin == (LibertyPin*)consumers->at(i);
                    VerilogNet *l = tmp_pin->connection_up->net_up;
                    if (l != NULL) {
                        l->get_consumers_except(tmp_pin->connection_up);
                        //std::cout << "  EndNode level 2: " << *consumers->at(i) << std::endl;
                    }
                }
            }
            delete(consumers);
        }
    }
}

LibertyPin * LibertyPin::get_associated_output()
{
    if(this->cell != NULL) {
        for(int i=0; i < this->cell->pins.size() ; i++) {
            if (this->cell->pins[i]->output) {
                return this->cell->pins[i];
            }
        }
    }
    return NULL;
}

std::vector<LibertyPin*> * LibertyPin::get_associated_inputs()
{
    std::vector<LibertyPin*> *inputs;
    inputs = new std::vector<LibertyPin*>();
    if(this->cell != NULL) {
        for(int i=0; i < this->cell->pins.size() ; i++) {
            if (this->cell->pins[i]->input) {
                inputs->push_back(this->cell->pins[i]);
            }
        }
    }
    return inputs;

}

std::vector<EndNode*> * LibertyPin::get_downstream() {
    VerilogNet *n;
    std::vector<EndNode*> *consumers;

    if(!has_downstream) {
        if(this->connection_up != NULL) {
            n = this->connection_up->net_up;
            if(n != NULL) {
                consumers = n->get_consumers_except(this->connection_up);
                for(int i=0;i<consumers->size();i++) {
                    EndNode *tmp_pin = consumers->at(i);
                    downstream.push_back(tmp_pin);
                    tmp_pin->upstream = this;
                }
                delete(consumers);
            }
        }
    }
    has_downstream = 1;

    return &downstream;
}

EndNode* LibertyPin::get_upstream() {
    VerilogNet *n;
    std::vector<EndNode*> *producers;

    if(upstream == NULL) {
        if(this->connection_up != NULL) {
            n = this->connection_up->net_up;
            if(n != NULL) {
                producers = n->get_producers_except(this->connection_up);
                for(int i=0;i<producers->size();i++) {
                    if(i==0) {
                        upstream = producers->at(i);
                    }
                    else {
                        std::cout << "More than one driver for pin: " << this->cell->get_path() << "/" << *this->name << " - Using only the first" << std::endl;
                    }
                }
                delete(producers);
            }
        }
    }

    return upstream;
}

void LibertyPin::print_receivers(int indent)
{
    VerilogNet *n;
    std::vector<EndNode*> *consumers;

    if(this->connection_up != NULL) {
        n = this->connection_up->net_up;
        if(n != NULL) {
            consumers = this->get_downstream();;
            for(int i =0;i<consumers->size() ; i++) {
                if (consumers->at(i)->get_type() == 1) {
                    LibertyPin *tmp_pin;
                    tmp_pin = (LibertyPin*)(consumers->at(i));
                    LibertyPin *associated_output = tmp_pin->get_associated_output();
                    for(int j=0;j<indent;j++) printf("  ");
                    std::cout << tmp_pin->cell->get_path() << "/" << *tmp_pin->name << " (" << *tmp_pin->cell->name << ") Output: " << *associated_output->name;
                    if(associated_output->connection_up != NULL && associated_output->connection_up->net_up != NULL)
                        std::cout << " Net: " << *associated_output->connection_up->net_up;
                    std::cout << std::endl;
                    if(indent < 50 && associated_output->name->compare("Q") != 0) {
                        bool found = 0;
                        for(int j=0;j<already_visited.size();j++) {
                            if (already_visited[j] == associated_output) {
                                found = 1;
                                break;
                            }
                        }
                        if(found) {
                            for(int j=0;j<indent+1;j++) printf("  ");
                                std::cout << "** Has already traversed this pin -- stopping " << *associated_output<< "**" << std::endl;
                        }
                        else {
                            already_visited.push_back(associated_output);
                            associated_output->print_receivers(indent + 1);
                        }
                    }
                }
            }
        }
    }
}

std::vector<TimingPath*> * LibertyPin::get_fanin(int level, TimingPath * in_path)
{
    VerilogNet *n;
    EndNode *producer;
    std::vector<LibertyPin*> *associated_inputs;
    std::vector<TimingPath*> *timing_paths;
    TimingPath *previous_path = NULL;
    TimingPath *pass_path;
    bool add_current_path = 1;
    bool already_added_common = 0;  // FIXME: implement in a cleaner way

    timing_paths = new std::vector<TimingPath*>();

    if (in_path == NULL) {
        in_path = new TimingPath();
    }
    int length_path_here = in_path->length;

    if(this->connection_up != NULL) {
        n = this->connection_up->net_up;
        if(n != NULL) {
            producer = this->get_upstream();;
            if(producer != NULL) {
                bool do_not_continue = 0;
                if (producer->get_type() == 1) {
                    do_not_continue = (((LibertyPin*)producer)->name->compare("Q") == 0);
                    do_not_continue = do_not_continue | (((LibertyPin*)producer)->name->compare("QN") == 0);
                }
                TimingNode *tnthis = new TimingNode(this);
                in_path->add_node_front(tnthis);
                all_timing_nodes.push_back(tnthis);
                TimingNode *tn = new TimingNode(producer);
                in_path->add_node_front(tn);
                all_timing_nodes.push_back(tn);
                if(producer->get_type() == 1)
                    associated_inputs = ((LibertyPin*)producer)->get_associated_inputs();
                else if (producer->get_type() == 2){
                    associated_inputs = new std::vector<LibertyPin*>();
                }
                else {
                    associated_inputs = new std::vector<LibertyPin*>();
                }
                length_path_here = in_path->length;
                for(int m=0;m<associated_inputs->size();m++) {
                    //for(int j=0;j<level;j++) printf("  ");
                    //std::cout << producer->cell->get_path() << "/" << *producer->name << " (" << *producer->cell->name << ") Input: " << *associated_inputs->at(m)->name;
                    //if(associated_inputs->at(m)->connection_up != NULL && associated_inputs->at(m)->connection_up->net_up != NULL)
                    //    std::cout << " Net: " << *associated_inputs->at(m)->connection_up->net_up;
                    //std::cout << std::endl;
                    if(level < 50) {
                        bool found = 0;
                        //for(int j=0;j<already_visited.size();j++) {
                        //    if (already_visited[j] == associated_inputs->at(m)) {
                        //        found = 1;
                        //        break;
                        //    }
                        //}
                        if(!do_not_continue) {
                            if(found) {
                                if(!already_added_common) {
                                    TimingNode *tn2 = new TimingNode(NOT_UNIQUE);
                                    in_path->add_node_front(tn2);
                                    all_timing_nodes.push_back(tn2);
                                    already_added_common = 1;
                                }
                            }
                            else {
                                if (previous_path == NULL)
                                    pass_path = in_path;
                                else
                                    pass_path = previous_path->clone_from_end_n(length_path_here);
                                previous_path = pass_path;
                                already_visited.push_back(associated_inputs->at(m));
                                std::vector<TimingPath*> *child_timing_paths = associated_inputs->at(m)->get_fanin(level + 1, pass_path);
                                for(int l=0;l<child_timing_paths->size();l++) {
                                    timing_paths->push_back(child_timing_paths->at(l));
                                }
                                delete(child_timing_paths);
                                //associated_inputs->at(m)->print_fanin(level + 1);
                                add_current_path = 0;
                            }
                        }
                    }
                }
                delete(associated_inputs);
            }
            //else {
            //    TimingNode *tn =  new TimingNode(this->cell->parent->get_path(), n);
            //    in_path->add_node_front(tn);
            //    all_timing_nodes.push_back(tn);
            //}
        }
    }
    if(add_current_path) {
        if (previous_path != NULL)
            in_path = previous_path->clone_from_end_n(length_path_here);
        timing_paths->push_back(in_path);
    }
    return timing_paths;
}


std::vector<TimingPath*> * LibertyPin::get_fanout(int level, TimingPath * in_path)
{
    std::vector<TimingPath*> *timing_paths;
    std::vector<EndNode*> *consumers;
    TimingPath *previous_path = NULL;
    TimingPath *pass_path;
    bool add_current_path = 1;

    timing_paths = new std::vector<TimingPath*>();

    if (in_path == NULL) {
        in_path = new TimingPath();
    }

    int length_path_here = in_path->length;

    consumers = this->get_downstream();;
    for(int i=0 ; i < consumers->size(); i++) {
        EndNode *current = consumers->at(i);
        LibertyPin *associated_output = NULL;
        if (current->get_type() == 1)
            associated_output = ((LibertyPin*)current)->get_associated_output();
        else if (current->get_type() == 2) {
            TimingNode *tn = new TimingNode(current);
            all_timing_nodes.push_back(tn);
            if (previous_path == NULL)
                pass_path = in_path;
            else
                pass_path = previous_path->clone_from_begin_n(length_path_here);
            previous_path = pass_path;
            pass_path->add_node_back(tn);
            timing_paths->push_back(pass_path);
            add_current_path = 0;
        }
        if(associated_output != NULL) {
            bool do_not_continue = (associated_output->name->compare("Q") == 0);
            TimingNode *tnthis = new TimingNode((LibertyPin*)current);
            all_timing_nodes.push_back(tnthis);
            TimingNode *tn = new TimingNode(associated_output);
            all_timing_nodes.push_back(tn);
            if (previous_path == NULL)
                pass_path = in_path;
            else
                pass_path = previous_path->clone_from_begin_n(length_path_here);
            previous_path = pass_path;
            pass_path->add_node_back(tnthis);
            pass_path->add_node_back(tn);
            if (level < 50) {
                if(!do_not_continue) {
                    bool found = 0;
                    //for(int j=0;j<already_visited.size();j++) {
                    //    if (already_visited[j] == associated_output) {
                    //        found = 1;
                    //        break;
                    //    }
                    //}
                    if(found) {
                        TimingNode *tn2 = new TimingNode(NOT_UNIQUE);
                        pass_path->add_node_back(tn2);
                        all_timing_nodes.push_back(tn2);
                        timing_paths->push_back(pass_path);
                        add_current_path = 0;
                    }
                    else {
                        already_visited.push_back(associated_output);
                        std::vector<TimingPath*> *child_timing_paths = associated_output->get_fanout(level + 1, pass_path);
                        for(int l=0;l<child_timing_paths->size();l++) {
                            timing_paths->push_back(child_timing_paths->at(l));
                        }
                        delete(child_timing_paths);
                        add_current_path = 0;
                    }
                }
                else {
                    timing_paths->push_back(pass_path);
                    add_current_path = 0;
                }
            }
        }
    }
    if(add_current_path) {
        //VerilogNet *n;
        //if(connection_up != NULL && connection_up->net_up != NULL) {
        //    n = connection_up->net_up;
        //    TimingNode *tn =  new TimingNode(this->cell->parent->get_path(), n);
        //    in_path->add_node_back(tn);
        //    all_timing_nodes.push_back(tn);
        //}
        if (previous_path != NULL)
            in_path = previous_path->clone_from_begin_n(length_path_here);
        timing_paths->push_back(in_path);
    }
    return timing_paths;


}

void LibertyPin::print_fanin(int indent)
{
    VerilogNet *n;
    EndNode *producer;
    std::vector<LibertyPin*> *associated_inputs;

    if(this->connection_up != NULL) {
        n = this->connection_up->net_up;
        if(n != NULL) {
            producer = this->get_upstream();;
            if(producer != NULL && producer->get_type() == 1) { // FIXME
                LibertyPin *tmp_pin = (LibertyPin*) producer;
                bool do_not_continue = (tmp_pin->name->compare("Q") == 0);
                associated_inputs = tmp_pin->get_associated_inputs();
                for(int m=0;m<associated_inputs->size();m++) {
                    for(int j=0;j<indent;j++) printf("  ");
                    std::cout << tmp_pin->cell->get_path() << "/" << *tmp_pin->name << " (" << *tmp_pin->cell->name << ") Input: " << *associated_inputs->at(m)->name;
                    if(associated_inputs->at(m)->connection_up != NULL && associated_inputs->at(m)->connection_up->net_up != NULL)
                        std::cout << " Net: " << *associated_inputs->at(m)->connection_up->net_up;
                    std::cout << std::endl;
                    if(indent < 50) {
                        bool found = 0;
                        for(int j=0;j<already_visited.size();j++) {
                            if (already_visited[j] == associated_inputs->at(m)) {
                                found = 1;
                                break;
                            }
                        }
                        if(!do_not_continue) {
                            if(found) {
                                for(int j=0;j<indent+1;j++) printf("  ");
                                    std::cout << "** Has already traversed this pin -- stopping " << *associated_inputs->at(m) << "**" << std::endl;
                            }
                            else {
                                already_visited.push_back(associated_inputs->at(m));
                                associated_inputs->at(m)->print_fanin(indent + 1);
                            }
                        }
                    }
                }
                delete(associated_inputs);
            }
        }
    }
}

std::string LibertyPin::get_name()
{
    return std::string(*this->name);
}

VerilogNet *LibertyPin::get_net()
{
    if(this->connection_up != NULL)
        return this->connection_up->net_up;
}

double LibertyPin::calculate_transition(double intrans, char sense, char min)
{
    LibertyPin *output_pin = this->get_associated_output();
    if (output_pin == NULL) {
        std::cout << "No output pin found for input pin" << std::endl;
        return 0;
    }
    VerilogNet *output_net = output_pin->get_net();
    if (output_net == NULL) {
        std::cout << "Output not connected to net, so no transition delay can be calculated" << std::endl;
        return 0;
    }
    double net_load = output_net->get_load(min);
    std::cout << "Intrans: " << intrans << "\nOutput_pin: " << *output_pin << "\nNet_name: " << *output_net << "\nNet load: " << net_load << std::endl;;
#ifdef DEBUG
    if(parent == NULL) std::cout << "Parent is null" << std::endl;
    else if (parent->special_luts == NULL) std::cout << "special_luts is null" << std::endl;
    std::cout << "all LUTs:" << std::endl;
    for (int i=0;i<output_pin->parent->luts.size();i++) {
        std::cout << *output_pin->parent->luts[i];
    }
#endif

    if(sense == 0) {
        if(output_pin->parent != NULL && output_pin->parent->special_luts != NULL && output_pin->parent->special_luts[LUT_FALL_TRANSITION] != NULL) {
#ifdef DEBUG
            std::cout << "Found table for fall transition" << std::endl;
#endif
            return output_pin->parent->special_luts[LUT_FALL_TRANSITION]->get_value(intrans, net_load);
        }
    }
    if(sense == 1) {
        if(output_pin->parent != NULL && output_pin->parent->special_luts != NULL && output_pin->parent->special_luts[LUT_RISE_TRANSITION] != NULL) {
#ifdef DEBUG
            std::cout << "Found table for rise transition" << std::endl;
#endif
            return output_pin->parent->special_luts[LUT_RISE_TRANSITION]->get_value(intrans, net_load);
        }
    }
    return 0;
}

double LibertyPin::calculate_delay(double intrans, char sense, char min)
{
    LibertyPin *output_pin = this->get_associated_output();
    if (output_pin == NULL) {
        std::cout << "No output pin found for input pin" << std::endl;
        return 0;
    }
    VerilogNet *output_net = output_pin->get_net();
    if (output_net == NULL) {
        std::cout << "Output not connected to net, so no delay can be calculated" << std::endl;
        return 0;
    }
    double net_load = output_net->get_load(min);
    std::cout << "Intrans: " << intrans << "\nOutput_pin: " << *output_pin << "\nNet_name: " << *output_net << "\nNet load: " << net_load << std::endl;;

    if(sense == 0) {
        if(output_pin->parent != NULL && output_pin->parent->special_luts != NULL && output_pin->parent->special_luts[LUT_CELL_FALL] != NULL) {
#ifdef DEBUG
            std::cout << "Found table for cell fall delay" << std::endl;
#endif
            return output_pin->parent->special_luts[LUT_CELL_FALL]->get_value(intrans, net_load);
        }
    }
    if(sense == 1) {
        if(output_pin->parent != NULL && output_pin->parent->special_luts != NULL && output_pin->parent->special_luts[LUT_CELL_RISE] != NULL) {
#ifdef DEBUG
            std::cout << "Found table for cell rise delay" << std::endl;
#endif
            return output_pin->parent->special_luts[LUT_CELL_RISE]->get_value(intrans, net_load);
        }
    }
    return 0;
}

int LibertyPin::get_type()
{
    return 1;
}

std::ostream & operator<<(std::ostream& os, const LibertyPin& obj) {
    os << "Pin: " << *obj.name;
    if(obj.input)
        os << " (input)";
    else if(obj.output)
        os << " (output)";
    return os;
}


void LibertyPin::cleanup()
{
    for(int i=0;i<luts.size();i++)  {
        delete luts[i];
    }
    luts.clear();
}

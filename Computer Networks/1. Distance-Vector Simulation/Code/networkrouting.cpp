//
// Created by abc on 9/26/21.
// Modified by Hu Yang on 12/14/2021
//

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <cctype>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <chrono>
#include "networkrouting.h"

using namespace std;
using namespace std::chrono;

Network_Info::Network_Info() {

}

Network_Info::~Network_Info() {

}
int Network_Info::set_node_id(std::string self_node_id) {
    node_id = self_node_id;
    return 1;
}

int Network_Info::init_node_addr(std::string neigh_nodes_file_in) {

    std::ifstream neigh_nodes_in {neigh_nodes_file_in};

    if(!neigh_nodes_in) {
        std::cerr << neigh_nodes_file_in << " won't open." << std::endl;
        exit(1);
    }

    string key;
    string value;
    while (neigh_nodes_in >> key && getline(neigh_nodes_in, value)) {
        node_addr[key] = stoi(value);
    }

    return 1;
}

int Network_Info::display_node_addr() {
    // to display port number used by nodes, including this nodes itself
    std::cout << "Ports of nodes:" << std::endl;
    for (const auto& item : node_addr) {
        std::cout << item.first << "'s port: " << item.second << std::endl;
    }
}

int Network_Info::init_cost_to_neigh(std::string cost_to_neigh_file_in){
    std::ifstream neigh_cost_in { cost_to_neigh_file_in };
    if (!neigh_cost_in) {
        std::cerr << cost_to_neigh_file_in << " won't open." << std::endl;
        exit(1);
    }

    string from_node, to_node, value;
    int16_t val;
    while (neigh_cost_in >> to_node &&  neigh_cost_in >> from_node && getline(neigh_cost_in, value)) {
        val = stoi(value);
        thres += val;
        if (to_node == node_id) neigh_cost[from_node] = val;
        if (from_node == node_id) neigh_cost[to_node] = val;

        if  (to_node == node_id) {
            // In the initialization phase, the neigh_connected is set to 0, indicating the link to the neighbor is not established.
            // Only after the initialization phase, and having received a PING_MSG from a neighbor,
            // the element of neigh_connected corresponding to this neighbor is set to 1
            neigh_connected[from_node] = 0;

            neighbour[from_node] = 1;
        }
        if (from_node == node_id) {
            // In the initialization phase, the neigh_connected is set to 0, indicating the link to the neighbor is not established.
            // Only after the initialization phase, and having received a PING_MSG from a neighbor,
            // the element of neigh_connected corresponding to this neighbor is set to 1
            neigh_connected[to_node] = 0;
            neighbour[to_node] = 1;
        }
    }
}

int Network_Info::display_cost_to_neigh() {
    // to display cost of link to the neighbors
    std::cout << "Distance to neighbors:" << std::endl;
    std::cout << "Neighbor Cost" << std::endl;
    for (const auto& item : neigh_cost)
        std::cout << item.first << " \t " << item.second << std::endl;
}


int Network_Info::init_routing_table(){
    // to initialize distance table

    map<string, int16_t>::iterator iter;
    iter = neigh_cost.begin();
    while (iter != neigh_cost.end()){
        auto pr = std::make_pair(iter->first, std::numeric_limits<int16_t>::max());
        auto table_pr = std::make_pair(iter->first, pr);
        auto ret_table_pr = distance_table.insert(table_pr);
        iter++;
    }

    return 1;
}

int Network_Info::display_routing_table() {
    cout << endl;
    cout << "Routing Table (at " << Network_Info::get_time_string() << "): " << endl;
    cout << "From \t To \t Cost \t Via" << endl;

    map<string, pair<string, int16_t>>::iterator distance_table_iter;
    distance_table_iter = distance_table.begin();
    while (distance_table_iter != distance_table.end()) {
        auto table_pr = distance_table_iter->second;
        cout <<  node_id << "\t" << distance_table_iter->first << "\t" << table_pr.second << "\t" << table_pr.first << endl;
        distance_table_iter++;
    }
    cout << endl;
}

std::string Network_Info::get_time_string() {
    std::time_t date_time = system_clock::to_time_t(system_clock::now());
    std::string time_string = std::ctime(&date_time);
    return time_string.substr(11, 8); // extract "hh:mm:ss"
}

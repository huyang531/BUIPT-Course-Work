//
// Created by abc on 9/25/21.
// Modified by Hu Yang on 12/14/21.
//

#include <sys/time.h>
#include <iostream>
#include <signal.h>
#include <limits>

#include "dv_algorithm.h"

#define INF std::numeric_limits<int16_t>::max()
#define VERBOSE // whether to display status messages

extern class Network_Info net_info; // a object of Network_info, which is used to nodes and link costs of the network
extern class TransportLayer *p_t_l;
extern class DV_Routing dv_routing;

extern std::string src_IP_address;
extern std::string dest_IP_address;

static volatile sig_atomic_t gotAlarm = 0;

int counter = 0;

static void sigalrmHandler(int sig) { // flood ping message at every interval
    gotAlarm = 1;
    counter++;

    // increase ping counter for connected nodes at every interval, see networkrouting.h for more
    if (dv_routing.increase_counter(&net_info) == -1) {
        // flood new routing table because a neighbor is down
        DV_Msg *p_distance_dv_msg = new DV_Msg;
        dv_routing.construct_dv_msg(&net_info, PATH_DISTANCE_MSG, p_distance_dv_msg);
        dv_routing.flood_dv_msg(p_t_l, &net_info, p_distance_dv_msg);
        delete p_distance_dv_msg;

#ifdef VERBOSE
        std::cout << "\nFound unconnected neighbor(s)!" << std::endl;
#endif

        net_info.display_routing_table();
    }

    // construct and flood ping message
    DV_Msg dv_msg;
    dv_routing.construct_dv_msg(&net_info, PING_MSG, &dv_msg);
    dv_routing.flood_ping_dv_msg(p_t_l, &net_info, & dv_msg);

    if (counter == 5) { // flood routing table to neighbors every 5 intervals
        counter = 0;
        
        DV_Msg *p_distance_dv_msg = new DV_Msg;
        dv_routing.construct_dv_msg(&net_info, PATH_DISTANCE_MSG, p_distance_dv_msg);
        dv_routing.flood_dv_msg(p_t_l, &net_info, p_distance_dv_msg);
        delete p_distance_dv_msg;

#ifdef VERBOSE
        std::cout << "[[ Flooded RT to neighbors! ]]" << std::endl;
#endif
    }
}

DV_Routing::DV_Routing(int interval, int thres) : thres(thres) {

    // to set timer interval of ping operation
    struct sigaction sa;
    struct itimerval itv;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = sigalrmHandler;
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        std::cout << "sigaction(SIGALRM, &sa, NULL) " << std::endl;
        exit(1);
    }

    itv.it_interval.tv_sec = interval;
    itv.it_interval.tv_usec = 0;
    itv.it_value.tv_sec = interval;
    itv.it_value.tv_usec = 0;

    if (setitimer(ITIMER_REAL, &itv, 0) == -1) {
        std::cout << "setitimer()" << std::endl;
        exit(1);
    }
}

DV_Routing::~DV_Routing() {}

int DV_Routing::construct_dv_msg(Network_Info * p_net_info, int msg_type, DV_Msg *p_dv_msg) {
    switch(msg_type) {
        case PING_MSG:
            // to construct ping message
            p_dv_msg->insert_from_node_id(PING_MSG, p_net_info->node_id);
            break;
        case PATH_DISTANCE_MSG:
            Path_Cost path_cost;
            p_dv_msg->insert_from_node_id(PATH_DISTANCE_MSG, p_net_info->node_id);
            for (const auto &element : p_net_info->distance_table) {
                path_cost.to_node_id = element.first;
                path_cost.cost = element.second.second;
                p_dv_msg->insert_a_path_cost(&path_cost);
            }
            // add a path to myself with cost of 0
            path_cost.to_node_id = p_net_info->node_id;
            path_cost.cost = 0;
            p_dv_msg->insert_a_path_cost(&path_cost);
            break;
    }

}

int DV_Routing::flood_ping_dv_msg(TransportLayer *p_t_l, Network_Info * p_net_info, DV_Msg * p_dv_msg) {
    // encode ping message
    std::string encoded_msg_str;
    p_dv_msg->encode(encoded_msg_str);

    // send dv_msg to neighbors, reset neigh_resp
    for (const auto& element : p_net_info->neighbour) {
        if (element.second != 1) continue;
        if (element.first == p_net_info->node_id) continue;

        auto peer_port_no = p_net_info->node_addr[element.first];  // to find neighbor's port no from neigh_addr;

        p_t_l->send_msg(dest_IP_address, peer_port_no, (char *)encoded_msg_str.c_str(), encoded_msg_str.length());
    }
}

int DV_Routing::flood_dv_msg(TransportLayer *p_t_l, Network_Info * p_net_info, DV_Msg *p_dv_msg) {

    // to encode the ping message
    std::string encoded_msg_str;

    p_dv_msg->encode(encoded_msg_str);

    // to send the dv_msg to its neighbors.
    for (const auto& element : p_net_info->neighbour) {
        if (element.second != 1) continue;

        if (element.first == p_net_info->node_id) continue;

        auto peer_port_no = p_net_info->node_addr[element.first];  // to find neighbor's port number from neigh_addr;

        p_t_l->send_msg(dest_IP_address, peer_port_no, (char *) encoded_msg_str.c_str(), encoded_msg_str.length());
    }
}

int DV_Routing::send_dv_msg(std::string to_node_id, TransportLayer *p_t_l, Network_Info * p_net_info, DV_Msg * p_dv_msg) {

    // to encode the ping message
    std::string encoded_msg_str;
    p_dv_msg->encode(encoded_msg_str);

    auto peer_port_no = p_net_info->node_addr[to_node_id];  // to find neighbor's port no from neigh_addr;

    p_t_l->send_msg(dest_IP_address, peer_port_no, (char *) encoded_msg_str.c_str(), encoded_msg_str.length());

    return 1;
}

int DV_Routing::decode_dv_msg(std::string received_msg, DV_Msg *p_dv_msg) {
    p_dv_msg->decode(received_msg);
}

int DV_Routing::increase_counter(Network_Info * p_net_info) {
    bool node_down = false; // node_down determines if a new flooding is needed
    // increase counter
    for (auto it1 = p_net_info->neigh_count.begin(); it1 != p_net_info->neigh_count.end();) {
        it1->second++;
        if (it1->second > thres) {
            node_down = true;
            // get node_id of the downed node
            std::string down_node_id = it1->first;

            // update neigh_count
            it1 = p_net_info->neigh_count.erase(it1);
            
            // update neigh_connected (topology)
            p_net_info->neigh_connected[down_node_id] = 0;

            // update routing table
            for (auto & it2 : p_net_info->distance_table) {
                // mark distance of node infinity to indicate it's down
                if (it2.first == down_node_id || it2.second.first == down_node_id) {
                    it2.second.second = INF;
                }
            }
        } else {
            it1++;
        }
    }
    // tell node to flood new routing table by returning -1
    if (node_down) {
        start_timer();
        return -1;
    }
    return 0;
}

void DV_Routing::start_timer() {
    inf_timer_enabled = true;
    inf_timer_start = clock();
    std::cout << "<INF_TIMER started @ " << Network_Info::get_time_string() << ">" << std::endl;
}

void DV_Routing::stop_timer() {
    inf_timer_enabled = false;
    inf_timer_start = 0;
    std::cout << "<INF_TIMER expired @ " << Network_Info::get_time_string() << ">" << std::endl;
}

int DV_Routing::reset_counter(Network_Info * p_net_info, std::string node_id) {

#ifdef VERBOSE
    std::cout << "Received Ping from " << node_id << "." << std::endl;
#endif
    
    p_net_info->neigh_count[node_id] = 0; // insert if it doesn't exist
    return 0;
}



#include "dv_algorithm.h"
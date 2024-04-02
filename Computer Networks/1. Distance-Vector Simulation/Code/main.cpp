// To compile, run "g++ *.cpp -std=c++1z -o dv-routing -lrt".
// To run nodeX, run "clear && ./dv-routing nodeX ./config/nodeaddr.txt ./config/topology.txt".

// You are recommended to run nodes in the order of (A, C, D), (B, F), and then E, to observe results.
// Then, shut down F, D, and E once at a time, and observe the behavior. Wait for convergence to be
// achieved before you shut down the next node - It might take a while, so please be patient!

// To better understand how INF_TIMER helps with the count-to-infinity problem, start all nodes, wait for
// convergence, and then shut down three of them (say A, B, and C) consecutively, each BEFORE the previous
// convergence was achieved, then observe the behavior.

// For more fun, toggle ACTIVE_MODE in this file, and VERBOSE in dv_algorithm.cpp.
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <string>
#include <cctype>
#include <stdlib.h>
#include <stdio.h>
#include <thread>
#include <limits>

#include <sys/time.h>
#include <signal.h>

#include "transport.h"
#include "dv_msg.h"
#include "networkrouting.h"
#include "user_cmd.h"
#include "dv_algorithm.h"

#define MAX_CMD_LEN 100
/* INF stands for Infinity. Remark: "INF messages" are messages of infinite cost that indicate a node is down. */
#define INF std::numeric_limits<int16_t>::max()
#define INTRVL 2
/**
 * This is the amount of time a node will reject deceitful information after it thinks a node is down. Theoretically, if it is
 * set to the time it takes for an INF message to reach every node in a network, then the count-to-infinity problem can be solved.
 * However, since it is impossible to project network delay in exact numbers, we still need a safety measure - A threshold
 * above which the cost of a route will be considered INF. In this way, the count-to-infinity problem becomes count-to-threshold.
 * In this program, the threshold is the sum of all edges in the topology, as seen in Network_Info::init_cost_to_neigh().
 */
#define INF_TIMER_THRES INTRVL * 5 * 1000 /* This could be 10 - 32 seconds, depending on how busy is the Ethernet. */
#define DOWN_THRESHOLD 3
/* In ACTIVE_MODE, the updated routes are flooded immediately when ready, instead of waiting for the periodic flood. */
// #define ACTIVE_MODE

static volatile sig_atomic_t gotAlarm = 0;
class Network_Info net_info; // an object of class Network_Info, net_info object is used to store nodes, link and distance table information of the network
class TransportLayer *p_t_l; // a pointer to a TransportLayer object, which is used to communicate with neighboring nodes of this node
class DV_Routing dv_routing(INTRVL, DOWN_THRESHOLD);  // an object instance of class DV_Routing, INTRVL is interval, and DOWN_THRESHOLD is the threshold to determine a node is down
/**
 * WARNING: Under ACTIVE_MODE, set INTRVL to 3 unless your computer has an extremely powerful Ethernet kernel.
 * 3 is already an aggressive number that could lead to Ethernet congestion - A threshold of 1 would simply 
 * cripple the entire program because of failed connection validation due to ping delays.
 */

std::string src_IP_address { "127.0.0.1" };
std::string dest_IP_address { "127.0.0.1" };

int main(int argc, char* argv[]) {

    char user_cmd[MAX_CMD_LEN]; // user_cmd is used to store user arguments
    DV_Msg *p_dv_msg;
    DV_Msg *p_distance_dv_msg;
    DV_Msg *p_received_dv_msg;

    // set the node ID of this process, the node ID is inputted as the first parameter of the command line parameters
    net_info.set_node_id(argv[1]);
    std::cout << "Router ID: " << net_info.node_id << std::endl;

    // init UDP ports of all nodes
    net_info.init_node_addr(argv[2]);
    net_info.display_node_addr();

    // init cost to neighbors
    net_info.init_cost_to_neigh(argv[3]);
    net_info.display_cost_to_neigh();

    // init routing table
    net_info.init_routing_table();

    // display the distance table
    net_info.display_routing_table();

    // initialize the transport layer
    int16_t self_port_no, peer_port_no;

    // find port number first
    self_port_no = net_info.node_addr[net_info.node_id];
    p_t_l = new TransportLayer(src_IP_address, self_port_no);

    // these are needed to receive a message
    char recv_buf[1000];
    int byte_received;

    // these are needed to send a DV_Msg
    Path_Cost path_cost;
    std::string encoded_msg_str;

    // for soloving count-to-inf problem
    int16_t cost_of_old_old_path;

    // init neigh_count
    for (const auto & it : net_info.neighbour) {
        if (it.second == 1) {
            net_info.neigh_count[it.first] = 0;
        }
    }

    std::cout << "INF: " << INF << std::endl;
    std::cout << "INF_TIMER: " << INF_TIMER_THRES << std::endl;
    std::cout << "DOWN_THRESHOLD: " << DOWN_THRESHOLD << std::endl;
    std::cout << "INTRVL: " << INTRVL << std::endl;

    while (true) {
        // check user command from the console
        if (read_user_cmd(user_cmd, MAX_CMD_LEN) == 1) {
            if (strcmp(user_cmd, "exit") == 0) {
                exit(0); // terminate the process
            }
            if (strcmp(user_cmd, "disp_rt") == 0) {
                net_info.display_routing_table(); // print routing table
            }
        }

        // check inf timer
        if (dv_routing.inf_timer_enabled && clock() - dv_routing.inf_timer_start >= INF_TIMER_THRES) {
            dv_routing.stop_timer();
        }

        // get message from neighbors (a PING_MSG is flooded at every interval)
        // recv_buf's size is 1000 bytes, and timeout is 1000ms
        byte_received = 0;
        byte_received = p_t_l->get_msg(recv_buf, 1000, 1000);

        // continue the loop if no message is received
        if (byte_received <= 0) continue;

        std::string received_string = recv_buf;

        // decode the message into a DV_Msg object
        p_received_dv_msg = new DV_Msg();
        p_received_dv_msg->decode(received_string);

        // process message according to its type
        switch (p_received_dv_msg->msg_type) {
            case PING_MSG: { // found a neighbor node
                dv_routing.reset_counter(&net_info, p_received_dv_msg->from_node_id); /* decrease counter for the node */
                // if the neighbor node is not connected to this node before, send the distance_table to this new connected neighbor
                if (net_info.neigh_connected[p_received_dv_msg->from_node_id] == 0) {
                    // update routing table
                    net_info.distance_table[p_received_dv_msg->from_node_id].first = p_received_dv_msg->from_node_id;
                    net_info.distance_table[p_received_dv_msg->from_node_id].second = net_info.neigh_cost[p_received_dv_msg->from_node_id];
                    
                    // init new route table message 
                    p_distance_dv_msg = new DV_Msg;
                    dv_routing.construct_dv_msg(&net_info, PATH_DISTANCE_MSG, p_distance_dv_msg);

                    // send message
                    dv_routing.send_dv_msg(p_received_dv_msg->from_node_id, p_t_l, &net_info, p_distance_dv_msg);

                    delete p_distance_dv_msg;

                    // mark this new node connected
                    net_info.neigh_connected[p_received_dv_msg->from_node_id] = 1;

#ifdef ACTIVE_MODE
                    // flood this good news to neighbors
                    p_distance_dv_msg = new DV_Msg;
                    p_distance_dv_msg->insert_from_node_id(PATH_DISTANCE_MSG, net_info.node_id);

                    path_cost.to_node_id = p_received_dv_msg->from_node_id;
                    path_cost.cost = net_info.neigh_cost[p_received_dv_msg->from_node_id];
                    p_distance_dv_msg->insert_a_path_cost(&path_cost);

                    dv_routing.flood_dv_msg(p_t_l, &net_info, p_distance_dv_msg);

                    delete p_distance_dv_msg;
#endif

                    // print new routing table
                    std::cout << "[NOTE]A neighbor is connected!" << std::endl;
                    net_info.display_routing_table();
                }
                break;
            }
            case PATH_DISTANCE_MSG: {
                bool has_modification = false;
                for (const auto& element : p_received_dv_msg->to_nodes) { // for each reachable node, check and update its cost
                    if (element.to_node_id == net_info.node_id) { // ignore myself
                        continue;
                    }

                    if (net_info.distance_table.count(element.to_node_id) != 1 && element.cost < INF) { // if the node was not in routing table
                        // add new entry
                        has_modification = true;
                        auto pr = std::make_pair(p_received_dv_msg->from_node_id, element.cost + net_info.neigh_cost[p_received_dv_msg->from_node_id]);
                        auto table_pr = std::make_pair(element.to_node_id, pr);
                        auto ret_table_pr = net_info.distance_table.insert(table_pr);

#ifdef ACTIVE_MODE
                        // generate a dv_msg containing ONLY the updated entry
                        p_distance_dv_msg = new DV_Msg;
                        p_distance_dv_msg->insert_from_node_id(PATH_DISTANCE_MSG, net_info.node_id);

                        path_cost.to_node_id = element.to_node_id;
                        path_cost.cost = pr.second;
                        p_distance_dv_msg->insert_a_path_cost(&path_cost);

                        // send message to neighbors
                        dv_routing.flood_dv_msg(p_t_l, &net_info, p_distance_dv_msg);

                        delete p_distance_dv_msg;
#endif

                        // print new routing table
                        net_info.display_routing_table();
                    } else if (net_info.distance_table.count(element.to_node_id) != 0) { // if an entry exists, check if a shorter path is discoverd
                        int16_t cost_of_new_path = element.cost + net_info.neigh_cost[p_received_dv_msg->from_node_id];
                        int16_t cost_of_old_path = net_info.distance_table[element.to_node_id].second;

                        if (cost_of_new_path < 0 && // node down
                            net_info.distance_table[element.to_node_id].first == p_received_dv_msg->from_node_id &&
                            cost_of_old_path < INF) { // an infinity distance will result in overflow
                            std::cout << "[ERROR]A node is down!" << std::endl;
                            /**
                             * Note that we are not able to find out exactly which node is down - We just know that a previously
                             * reachable node is no longer reachable. If this node is a neighbor, then great! We know that a
                             * neighbor is down. But also not-so-great, because we would have already noticed that from the timed-out
                             * pings long before we receive an updated routing table from a neighbor telling us another neighbor
                             * is down. However, in some extreme cases (i.e., it is faster to reach neighbor A via neighbor B than
                             * directly accessing A), that might be the case. So, we include the following "redundant" code.
                             * 
                             * EDIT: As it turns out, deprecated INF messages would also result in the following [WARNING] situation.
                             * This is more of a bonus effect because then the observer could tell that the network is congested.
                             */
                            
                            // update neigh_connected if it's a neighbor AND somehow it's not yet updated
                            if (net_info.neighbour[element.to_node_id] == 1) {
                                std::cout << "[WARNING]Neighbor " << element.to_node_id << " is down!" << std::endl;
                                net_info.neigh_connected[element.to_node_id] = 0;
                            }

                            // Start INF_TIMER to avoid count-to-inf and circular routing problem
                            cost_of_old_old_path = net_info.distance_table[element.to_node_id].second;
                            dv_routing.start_timer();
                            
                            /**
                             * Update routing table to infinity. Note that you can't just remove this entry altogether
                             * because then you wouldn't be able to propagate this "node down" information to your neighbors.
                             * Also, using infinity can allow later updates on the path to the destination node via another
                             * node that is NOT the downed node.
                             */
                            has_modification = true;
                            net_info.distance_table[element.to_node_id].second = INF;


// #ifdef ACTIVE_MODE
                            /**
                             * Unlike other news, for the timer mechanism to work, INF news are flooded immediately (regardless
                             * of whether ACTIVE_MODE is set) when they are ready.
                             */

                            // flood an infinity route
                            // generate a dv_msg containing ONLY the updated entry
                            p_distance_dv_msg = new DV_Msg;
                            p_distance_dv_msg->insert_from_node_id(PATH_DISTANCE_MSG, net_info.node_id);

                            path_cost.to_node_id = element.to_node_id;
                            path_cost.cost = INF;
                            p_distance_dv_msg->insert_a_path_cost(&path_cost);

                            // send message to neighbors
                            dv_routing.flood_dv_msg(p_t_l, &net_info, p_distance_dv_msg);

                            delete p_distance_dv_msg;
// #endif

                        } else if (cost_of_new_path < cost_of_old_path &&
                                   cost_of_new_path > 0 && // a shorter path is found
                                   (dv_routing.inf_timer_enabled == false || // if not in an inf_timer period, or,
                                    cost_of_new_path < cost_of_old_old_path) &&
                                   cost_of_new_path < net_info.thres) { // an actually better path is found
                            {
                                // The following code is WRONG! A node may never know if its neighbor is down! It only knows that
                                // a neighbor (say A) is no longer reachable via the original link between them. The cause could
                                // be either 1) A is down, or 2) A is still up, but the link broke, i.e., the topology changed and
                                // A is no longer a neighbor.
                                // if (net_info.neighbour[element.to_node_id] && !net_info.neigh_connected[element.to_node_id]) {
                                //     // do not update but back propagate
                                //     // init new route table message 
                                //     p_distance_dv_msg = new DV_Msg;
                                //     dv_routing.construct_dv_msg(&net_info, PATH_DISTANCE_MSG, p_distance_dv_msg);

                                //     // send message
                                //     dv_routing.send_dv_msg(p_received_dv_msg->from_node_id, p_t_l, &net_info, p_distance_dv_msg);

                                //     delete p_distance_dv_msg;
                                //     break;
                                // }
                            }

                            // update table
                            has_modification = true;
                            net_info.distance_table[element.to_node_id].first = p_received_dv_msg->from_node_id;
                            net_info.distance_table[element.to_node_id].second = cost_of_new_path;

#ifdef ACTIVE_MODE
                            // generate a dv_msg containing ONLY the updated entry
                            p_distance_dv_msg = new DV_Msg;
                            p_distance_dv_msg->insert_from_node_id(PATH_DISTANCE_MSG, net_info.node_id);

                            path_cost.to_node_id = element.to_node_id;
                            path_cost.cost = cost_of_new_path;
                            p_distance_dv_msg->insert_a_path_cost(&path_cost);

                            // flood message to neighbors
                            dv_routing.flood_dv_msg(p_t_l, &net_info, p_distance_dv_msg);
                            delete p_distance_dv_msg;
#endif

                        }
                    }
                }
                if (has_modification) {
                    // print new routing table
                    net_info.display_routing_table();
                }
                break;
            }
            default: break;
        }
        delete p_received_dv_msg;
    }

    return 0;
}

// This is a stop-and-wait server/client. See readme.txt.

// To compile, run "g++ -std=c++1z *.cpp -o rudp"
// To run, run "./rudp client|server"
// To toggle drop, enter "TOGGLE_DROP"
// To toggle down, enter "TOGGLE_DOWN"

// To test stop-and-wait:
// 1) Start the server, wait, and then start the client
// 2) Wait, and then toggle drop on the client
// 3) Wait, and then toggle drop on the server

// To test re-connection:
// 1) Start both ends
// 2) Toggle drop on the server
// 3) Toggle drop on the client, and then IMMEDIATELY toggle down. Now the client is down.
// 4) Wait for the server to realize the client is down (this will take around 18 seconds).
//    The server will stop re-sending messages to the client but will still periodically ping it
// 5) Toggle down on the client. The server now continues to send what's left.
// You may also switch the role (i.e. toggle drop on the client in step 2, etc.), then you can
// simulate when the server stops sending in the middle but later re-connects.

// @remark: Just as with TCP in real life, the server and client both receives and
// sends messages, so, they run the same program. However, only when running with
// the argument "server" will the messages be initialized. Running with argument
// "client" will initialize a client that has nothing to send to the server.

// @remark: To avoid confusion, the term "server" and "client" used in the following
// comments may not always imply the way in which the program is run, i.e., when the
// client has an ACK message to send to the server, it may be seen as a "server" as
// well. So whether server == sender and/or client == receiver depends on the context. 😂

// @remark: A regular message is a message that contains three fields and a content,
// whereas a ping message only contains the source address. See rudp_msg.h for more.
#include <sys/time.h>

#include <bitset>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "rudp_msg.h"
#include "transport.h"
#include "user_cmd.h"

#define MAX_CMD_LEN 100
#define MAX_MSG_LEN 1000
#define NUM_MSG 10
#define DOWN_THRESHOLD 9  // # of missed pings that indicates the client is down
#define TIMEOUT 3         // if ACK isn't received within TIMEOUT seconds, resend message
#define PING_INTERVAL 2   // ping interval
#define DONT_GIVE_TIPS    // whether to give tips when user command is illegal

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using namespace std::chrono;

string src_IP_address{"127.0.0.1"};
string dest_IP_address{"127.0.0.1"};
int16_t src_port{8011};
int16_t dest_port{8012};

// some messages
const string messages[NUM_MSG] = {
    "This is the 1st message! 哈哈哈！",
    "This is the second message...",
    "这是第三条消息",
    "你好！这是第4条信息。",
    "Hello world! 5",
    "6666666666",
    "Message No.7",
    "What's up? This is the 8th message.",
    "这是第九条，也就是倒数第二条消息",
    "Message No.10. 最后一条消息。"};

// message buffer
vector<string> messages_to_send;

TransportLayer *p_t_l;

// some variables to implement stop-and-wait
bool client_is_up = false;
bool clear_to_send_next = true;          // clear to send the first message
bool drop_incoming_reg_messages = true;  // this is to simulate packet lost
bool node_down = false;                  // this is to simulate node down

// timer related stuff
bool timer_enabled = false;
time_point<system_clock> timer_start;

// This is a safety measure because ping_counter overflows way slower than
// TIMEOUT. If the client is down, there's no need to keep re-sending messages
// to it. (This is implemented.) However, if the client is downed but then
// quickly re-connected (or has been repeatedly doing so), this might indicate
// that the network delay is too high, so the server can then double TIMEOUT and
// DOWN_THRESHOLD to accommodate this fact. However, to implement the logic for
// that requires more time than I have. :-( So it's not implemented here.
int ping_counter = 0;

// The server will periodically ping the client when it wants to send something to it.
bool ping_enabled = false;

static void sigalrmHandler(int sig) {
    ping_counter++;
    if (ping_enabled && !node_down) {
        // send ping to dest address
        auto *p_ping_msg = new RUDP_Msg();
        p_ping_msg->insert_header(PING_MSG, src_IP_address, src_port);
        string encoded_string;
        p_ping_msg->encode(encoded_string);
        p_t_l->send_msg(dest_IP_address, dest_port, (char *)encoded_string.c_str(), encoded_string.length());
        delete p_ping_msg;
        cout << "[INFO] Pinged destination." << endl;
    }
}

// send a regular message to client
void send_reg_message(int index, int seq, int ack, int fin) {
    auto *p_reg_msg = new RUDP_Msg();
    p_reg_msg->insert_header(REG_MSG, src_IP_address, src_port);
    if (index >= 0) p_reg_msg->insert_message_content(messages_to_send[index]);
    p_reg_msg->insert_flag_field(seq, ack, fin);
    string encoded_string;
    p_reg_msg->encode(encoded_string);
    p_t_l->send_msg(dest_IP_address, dest_port, (char *)encoded_string.c_str(), encoded_string.length());
    delete p_reg_msg;
}

void start_timer() {
    timer_enabled = true;
    timer_start = system_clock::now();
}

bool timeout() {
    // count milliseconds for better accuracy
    if (timer_enabled && duration_cast<milliseconds>(system_clock::now() - timer_start).count() > TIMEOUT * 1000) {
        return true;
    }
    return false;
}

void stop_timer() {
    timer_enabled = false;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "[ERROR] usage: ./rudp server|client" << endl;
        exit(1);
    } else {
        if (strcmp(argv[1], "server") == 0) {
            // add messages to buffer
            for (const auto &msg : messages) {
                messages_to_send.emplace_back(msg);
            }
            // swap source and destination addresses
            std::swap(src_IP_address, dest_IP_address);
            std::swap(src_port, dest_port);
        } else if (strcmp(argv[1], "client") != 0) {  // if argv[1] is neither "server" nor "client"
            cout << "[ERROR] usage: ./rudp server|client" << endl;
            exit(1);
        }
    }

    // init buffer and stuff
    char user_cmd[MAX_CMD_LEN];
    RUDP_Msg *p_recv_msg;

    p_t_l = new TransportLayer(src_IP_address, src_port);

    char recv_buf[MAX_MSG_LEN];
    int byte_received;

    string encoded_string;

    int index = 0;                   // index of next message to send
    int ack = -1, seq = 1, fin = 1;  // fin = 1 because we have finished the last transmission

    // init periodic ping
    struct sigaction sa;
    struct itimerval itv;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = sigalrmHandler;

    if (sigaction(SIGALRM, &sa, nullptr) == -1) {
        cerr << "[ERROR] in sigaction()" << endl;
        exit(-2);
    }

    itv.it_interval.tv_sec = PING_INTERVAL;
    itv.it_interval.tv_usec = 0;
    itv.it_value.tv_sec = PING_INTERVAL;
    itv.it_value.tv_usec = 0;

    if (setitimer(ITIMER_REAL, &itv, nullptr) == -1) {
        cerr << "[ERROR] in setitimer()" << endl;
        exit(-3);
    }

    // If I have stuff to send to the client, start pinging it.
    if (!messages_to_send.empty()) ping_enabled = true;

    system("clear");
    // print init info
    cout << "[INFOLIST_BEGIN]" << endl
         << "Initialization successful!" << endl
         << "My name: " << argv[1] << endl
         << "Ping interval: " << PING_INTERVAL << 's' << endl
         << "Source Address: " << src_IP_address << ':' << src_port << endl
         << "Destination Address: " << dest_IP_address << ':' << dest_port << endl
         << "Waiting for client to connect..." << endl
         << "[INFOLIST_END]" << endl;

    // main loop
    while (true) {
        // if I have more messages to send, set fin = 0
        if (!messages_to_send.empty() && index < messages_to_send.size()) fin = 0;

        // check user command
        if (read_user_cmd(user_cmd, MAX_CMD_LEN) == 1) {
            if (strcmp(user_cmd, "EXIT") == 0) {
                cout << "[WARNING] User terminated server program!" << endl;
                exit(-1);
            } else if (strcmp(user_cmd, "TOGGLE_DROP") == 0) {
                drop_incoming_reg_messages = !drop_incoming_reg_messages;
                if (drop_incoming_reg_messages) {
                    cout << "[WARNING] Incoming regular messages will be dropped!" << endl;
                } else {
                    cout << "[WARNING] Incoming regular messages will be received!" << endl;
                }
            } else if (strcmp(user_cmd, "TOGGLE_DOWN") == 0) {
                node_down = !node_down;
                if (node_down) {
                    stop_timer();
                    cout << "[WARNING] Node down. Nothing will be sent or received now." << endl;
                } else {
                    cout << "[WARNING] Node is up. Messages will be sent and received." << endl;
                }
            } else {
#ifndef DONT_GIVE_TIPS
                cout << "[ERRORLIST_BEGIN] \"" << user_cmd << "\" cannot be resolved. Available commands are: " << endl
                     << "EXIT: terminate the program" << endl
                     << "TOGGLE_DROP: toggle whether to drop incoming regular messages" << endl
                     << "TOGGLE_DOWN: toggle whether the node is down" << endl
                     << "[ERRORLIST_END]" << endl;
#endif
            }
        }

        if (client_is_up && !node_down) {  // if client is connected
            // send next message
            if (clear_to_send_next && index < messages_to_send.size()) {
                if (index == 0) {
                    seq = 1;
                } else {
                    seq += messages_to_send[index - 1].length() * 8;
                }
                if (index == messages_to_send.size() - 1) fin = 1;  // piggyback FIN in the last message
                send_reg_message(index, seq, ack, fin);
                start_timer();
                cout << "[INFO] Message " << index << " is sent. Waiting for ACK..." << endl;
                index++;
                clear_to_send_next = false;
            }

            if (ping_counter > DOWN_THRESHOLD &&                               // if client has't pinged in a long time &&,
                (fin != 1 ||                                                   // I haven't finished sending my messages ||,
                 (p_recv_msg != nullptr && (p_recv_msg->flag & FIN) == 0))) {  // my server hasn't finished sending its messages
                client_is_up = false;
                cout << "[WARNING] Client is down! Waiting for reconnection..." << endl;
            }

            if (timeout()) {                                           // timeout() == true implies that server is waiting for ACK
                                                                       // resend last message
                if (index <= messages_to_send.size() && index != 0) {  // if last message is a message with content
                    cout << "[WARNING] Did not receive ACK for message " << index - 1 << ". Re-sending..." << endl;
                    send_reg_message(index - 1, seq, ack, fin);
                } else {  // or it's a message without content
                    cout << "[WARNING] Did not receive ACK for message with seq=" << seq << ". Re-sending..." << endl;
                    send_reg_message(-1, seq, ack, fin);
                }
                stop_timer();
                start_timer();
            }
        }

        // receive new message
        byte_received = p_t_l->get_msg(recv_buf, 1000, 1000);
        if (byte_received <= 0) continue;

        p_recv_msg = new RUDP_Msg();
        p_recv_msg->decode(recv_buf);

        // In our simulation, messages are received but not handled when the node is down.
        // This is because if we don't receive the messages, they will pile up in the Ethernet
        // queue, so, when the client is turned back up, it will have to handle deprecated and
        // repeated messages in the queue, which is not what will happen in real life.
        if (node_down) continue;

        // switch case
        switch (p_recv_msg->msg_type) {
            case PING_MSG: {  // ping from client
                ping_counter = 0;
                if (!client_is_up) {
                    client_is_up = true;
                    cout << "[INFO] Client is connected!" << endl;
                }
                // When the client receives a ping from the server, it starts to ping the
                // server as well to let the server know that it's up.
                if (!ping_enabled) {
                    ping_enabled = true;
                    fin = 0;  // connection start, set fin = 0
                }
                break;
            }
            case REG_MSG: {
                if (drop_incoming_reg_messages) break;
                // print received message
                cout << "[INFOLIST_BEGIN]" << endl
                     << "A regular message is received!" << endl
                     << "Flag: " << std::bitset<3>(p_recv_msg->flag) << endl
                     << "\tSEQ: " << p_recv_msg->seq << endl
                     << "\tACK: " << p_recv_msg->ack << endl
                     << "\tFIN: " << ((p_recv_msg->flag & FIN) > 0) << endl
                     << "Content: " << p_recv_msg->content << endl
                     << "[INFOLIST_END]" << endl;
                // check FIN first to allow piggybacking
                if (p_recv_msg->flag & FIN) {
                    // set ACK
                    ack = p_recv_msg->seq + 1;
                    // if I want to end as well, set FIN
                    if (messages_to_send.empty() || index >= messages_to_send.size()) fin = 1;
                    // send message
                    send_reg_message(-1, p_recv_msg->ack, ack, fin);  // Note that we can't reset variable seq because it is needed later.
                    start_timer();
                }
                if (p_recv_msg->flag & ACK) {
                    if (fin != 1 && messages_to_send.empty()) {  // for client
                        if (p_recv_msg->ack == seq) {
                            stop_timer();
                            clear_to_send_next = true;
                            ack = -1;
                            cout << "[INFO] ACK for the last message is received!" << endl;
                        }
                    } else if (!messages_to_send.empty() && p_recv_msg->ack == seq + messages_to_send[index - 1].length() * 8) {
                        // for the server, if the last message is ACKed
                        // (i.e., ack == seq of last message + size of last message)
                        stop_timer();
                        clear_to_send_next = true;
                        ack = -1;  // server's ack is ACKed, so reset it
                        cout << "[INFO] ACK for message " << index - 1 << " is received! Clear to send next message." << endl;
                    } else if (fin == 1 && p_recv_msg->ack == seq + 1) {  // if ACK for FIN message is received
                        seq = seq + 1;
                        ack = p_recv_msg->seq + 1;
                        send_reg_message(-1, seq, ack, fin);
                        cout << "[INFO] Transmission finished! Waiting for the next transmission..." << endl;
                        messages_to_send.clear();  // delete all messages
                        ping_enabled = false;
                        exit(0);  // If we start properly as in TCP, we can just break here not worry about deprecated pings... :-(
                    }
                }
                if (fin != 1 && p_recv_msg->flag & SEQ) {  // during normal transmission
                    // set ACK that is going to be sent piggybackedly
                    ack = p_recv_msg->seq + p_recv_msg->content.length() * 8;
                    if (messages_to_send.size() <= index) {  // if there aren't more messages to send
                                                             // send ACK immediately
                        send_reg_message(-1, seq, ack, fin);
                        // start_timer();  // However, do not start timer for this plain ACK message because it is unnecessary!
                    }
                }

                break;
            }
            default:
                break;
        }
        delete p_recv_msg;
    }

    delete p_t_l;
    return 1;
}

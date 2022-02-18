#include "rudp_msg.h"

#include <cstdio>
#include <cstdlib>
#include <regex>
#include <string>
#include <utility>
using namespace std;

RUDP_Msg::RUDP_Msg() = default;
RUDP_Msg::~RUDP_Msg() = default;

int RUDP_Msg::insert_header(int msg_kind, std::string ip, int16_t port) {
    msg_type = msg_kind;
    src_IP_address = std::move(ip);
    src_port = port;
    return 0;
}

int RUDP_Msg::insert_flag_field(int s, int a, int fin) {
    flag = ((s >= 0) << 2) | ((a >= 0) << 1) | (fin == 1);
    this->seq = s;
    this->ack = a;
    return 0;
}

int RUDP_Msg::insert_message_content(std::string c) {
    if (msg_type == PING_MSG) {
        perror("[ERROR] Ping message must not have content! Consider setting message header first!\n");
        return 1;
    }
    this->content = std::move(c);
    return 0;
}

int RUDP_Msg::encode(std::string &encoded_string) const {
    // add message type and source address
    char tmp_str[100];
    sprintf(tmp_str, "%d#%s#%d", msg_type, src_IP_address.c_str(), src_port);
    encoded_string = (std::string)tmp_str;

    // add other content
    switch (msg_type) {
        case PING_MSG: {
            encoded_string = encoded_string + '#';
            break;
        }
        case REG_MSG: {
            char tmp_str[100];
            sprintf(tmp_str, "#%d#", flag);
            encoded_string += tmp_str;
            if (flag & SEQ) {
                char tmp_str[100];
                sprintf(tmp_str, "%d", seq);
                encoded_string += (std::string)tmp_str + '#';
            }
            if (flag & ACK) {
                char tmp_str[100];
                sprintf(tmp_str, "%d", ack);
                encoded_string += (std::string)tmp_str + '#';
            }
            encoded_string += content + '#';
            break;
        }
        default:
            break;
    }

    return 0;  // indicate success
}

int RUDP_Msg::decode(std::string string_to_be_decoded) {
    std::string to_decode_string;

    regex ex_field("#");

    to_decode_string = std::move(string_to_be_decoded);

    std::sregex_token_iterator pos(to_decode_string.begin(), to_decode_string.end(), ex_field, -1);

    // extract message type
    msg_type = std::stoi(pos->str(), nullptr, 10);
    pos++;

    // extract source address
    src_IP_address = pos->str();
    pos++;
    src_port = (int16_t)std::stoi(pos->str(), nullptr, 10);
    pos++;

    // extract other fields
    if (msg_type == REG_MSG) {
        // extract flag
        flag = std::stoi(pos->str(), nullptr, 10);
        pos++;
        // extract SEQ
        if (flag & SEQ) {
            seq = std::stoi(pos->str(), nullptr, 10);
            pos++;
        }
        // extract ACK
        if (flag & ACK) {
            ack = std::stoi(pos->str(), nullptr, 10);
            pos++;
        }
        // extract content
        content = pos->str();
    }

    return 0;
}

// The RUDP_Msg class is used to represent a reliable UDP message, which contains
// the following fields:
//    1) message type - ping or regular message,
//    2) source IP address,
//    3) source port number,
//    4) (regular message only) flag field - SEQ, ACK, or FIN,
//    5) (flagged reg message only) flag content - SEQ and/or ACK, separated by '#',
//    6) (regular message only) message string - a string terminated by '#'.
// insert_header() inserts message type and the source address (IP + port)
// insert_flag_field inserts the flag field and the flag content
// insert_message_content() adds content to the message string

// The member function encode() is to encode a RUDP_Msg object into a std::string variable,
// the seperator between the fields of the encoded message is '#', for example,
// "0#127.0.0.1#8011" translates to a ping message from 192.0.0.1:8011
// "1#127.0.0.1#8012#7#1#210#This is a test message#" translates to a regular message from 127.0.0.1:8012,
// with flags: (7d == 111b)
// SEQ: set, 1
// ACK: set, 210
// FIN: set
// and content: This is a test message

// WARNING: '#' and '/' are reserved characters with no escaping!

// The member function decode() is to decode a std::string variable into a RUDP_Msg object.
// The decode() function uses regular expression library regex to extract each field encoded in the std::string object.

#ifndef RUDP_MSG_H
#define RUDP_MSG_H

#define PING_MSG 0
#define REG_MSG 1  // regular message

#include <cctype>
#include <string>

// RUDP_Msg flags
const unsigned int SEQ = 0x4;
const unsigned int ACK = 0x2;
const unsigned int FIN = 0x1;

class RUDP_Msg {
   public:
    int msg_type{};
    std::string src_IP_address;
    int src_port{};
    int flag = 0;
    int seq = -1;
    int ack = -1;
    std::string content;

    RUDP_Msg();
    ~RUDP_Msg();

    // This function inserts its three parameters into the message.
    int insert_header(int msg_kind, std::string ip, int16_t port);

    // s/a = -1 indicates flag not set, fin = 1 indicates flag set, 0 not set.
    int insert_flag_field(int s, int a, int fin);

    int insert_message_content(std::string content);

    int encode(std::string &encoded_string) const;
    int decode(std::string string_to_be_decoded);
};

#endif

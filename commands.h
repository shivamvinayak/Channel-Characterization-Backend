#include<stdio.h>
#include<string.h>


#define MAX_NUM_NODES 100
#define MAX_NUM_NEIGHBOURS 34
#define TOSH_DATA_LENGTH 99
#define HEADER_SIZE 8
#define FOOTER_SIZE 0
#define METADATA_SIZE 16
#define myoffsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#define MAX_NUM_HOPS 15

// COMMANDS BETWEEN GUI AND SERVER
const unsigned short INIT_INFO = 1; // initial handshake
const unsigned short RESET = 4; // reset network
const unsigned short LINK_LEARN = 5; // start link learning
const unsigned short SOURCE = 7; //sources sent
const unsigned short REFRESH = 11; //to clear some files so new case can be run
const unsigned short CONFIGURE = 12; //network parameters
const unsigned short SEND_LL_LIST =15; // list for link learning
const unsigned short SEND_PKT_PART =16;
const unsigned short CLOSE_SERVER =21; // to close the server
const unsigned short HOP_INFO =23;// to send hop info
const unsigned short STOP_DATA_COLLECTION =24;// stop pseudo data collection 
const unsigned short OPTIMIZE_POWER = 25;//start link wise power optimization
const unsigned short PATH_ARRAY_ROUTING =26;// path array for trace route
const unsigned short PATH_ARRAY_POWER_OPTIMIZATION = 27;//path array for power optimization
const unsigned short LEARNT_LINK_INFO = 28; //learnt links info


//COMMANDS BETWEEN SERVER AND BS DEVICE
const unsigned short RESET_PKT = 59; // reset 
const unsigned short NODES_ARRAY_PKT = 55; // link learning nodes array
const unsigned short START_LINK_LEARNING_PKT =63; // power and num of TX pkts with start LL command
const unsigned short STATUS_PKT = 66; // which node is broadcasting hello pkts
const unsigned short DONE_PKT = 52; // link learning done
const unsigned short NODE_ID_PKT = 56; // per list
const unsigned short REBOOT_PKT = 58; // to reboot a mote
const unsigned short RSSI_LIST_PKT = 60; // rssi list
const unsigned short CONFIG_PKT = 51; // power and no. of pkts info for routing
const unsigned short ROUTING_INFO_PKT =61; //node next hop rank and slot timings 
const unsigned short START_ROUTING_PKT =64; //start sending pseudo sensor pkts
const unsigned short STOP_ROUTING_PKT = 65;// stop pseudo data collection
const unsigned short SENSOR_PKT = 62;// to identify sensor pkts from the mote
const unsigned short PATH_INFO_PKT = 73;//to send the path info to the BS for trace routing
const unsigned short START_TRACE_ROUTE_PKT = 74; //to indicate start of trace routing
const unsigned short PATH_BROKEN_PKT = 72;//sent by BS when a path is detected to be broken
		// NOT YET INCLUDED IN THE NESC CODE
const unsigned short ERROR_LL = 88; // node didnot respond to link learning command
const unsigned short I_AM_ALIVE = 89; //
const unsigned short CHECK_BS_CONNECTION =27;// to check if any BS mote is connected to the server
const unsigned short MOTE_PING =90;
const unsigned short MOTE_PING_RESPONSE = 91;
const unsigned short FLASH_ERASE = 92;
const unsigned short CLEAR_PKT = 105;
const unsigned short HELLO_PKT = 200;  // Being used only by motes, not by SmartConnect-Server

//PACKET STRUCTURE DEFINITIONS


typedef struct learnt_link_info{
	int type;
	int nof_elements;
	int nodes[2000];
	int neighbors[2000];
	int bad_pkts[2000];
}__attribute ((packed)) learnt_link_info;

typedef struct path_broken_info {
	int type;
	int unresponsive_node_id;
}__attribute ((packed)) path_broken_info;



//used
//COMMAND PACKET 
typedef struct mycommand {
        uint8_t type;
        uint8_t value;
	uint8_t value2;
	uint32_t global_start_time;
}__attribute ((packed)) mycommand;

//used 
//THE DELAY AND SEQ NO. INFORMATION PKT SENT TO GUI AFTER RECEIVING SENSOR PKT  
struct delaypkt{
//	int type;
        int moteno;
        int seqno;
        int delay;
};

//used
//PACKET TO SEND ARRAYS FROM CLIEBT TO SERVER
typedef struct source_pkt{
        int nof_sources;
        int source[50];
}source_pkt;


//used
//TINYOS DEFINED MESSAGE_T PKT
typedef struct message_t {
        uint8_t header[HEADER_SIZE];
        uint8_t data[TOSH_DATA_LENGTH];
        uint8_t footer[FOOTER_SIZE];
        uint8_t metadata[METADATA_SIZE];
} __attribute((packed)) message_t;


//used
// SENSOR DATA PKT
typedef struct sensordata
{
        uint8_t type;
        uint8_t source;
	uint8_t sensor_data_type; //analog or digital
        uint8_t sensor_data_from_channels[8];
        uint16_t seqnum;
        uint32_t path_txtmstp_array;
        uint32_t path_rxtmstp_array;
	uint8_t path[MAX_NUM_HOPS];
}  __attribute((packed)) sensordata;


//used
//TO SEND ARRAYS 
typedef struct initcommand{
     uint8_t type;
     uint8_t sources[MAX_NUM_NODES];
}__attribute ((packed)) initcommand;

//used
//LINK LEARNING CONFIGURATION
typedef struct config_pkt{
	uint8_t type;
	uint8_t node_id;
	uint16_t numofpkts;
} __attribute ((packed)) config_pkt;

// not used 
//TO REBOOT THE STSTEM FULLY 
typedef struct rebootpkt{
	uint8_t type;
	uint8_t img_num;
} rebootpkt;

//used
//PER LIST
typedef struct per_list{
        uint8_t type;	
	uint8_t src;
	uint8_t neighbour_array[MAX_NUM_NEIGHBOURS];
	uint16_t numpkt_array[MAX_NUM_NEIGHBOURS];
} per_list;

//used
//RSSI LIST 
typedef struct rssi_list {
        uint8_t type;
        uint8_t src;
	uint8_t neighbour_array[MAX_NUM_NODES];
        int8_t avg_rssi_array[MAX_NUM_NODES];
}rssi_list;

//used
//ROUTING INFORMATION 
typedef struct routinginfo
{
  uint8_t type;
  uint8_t nodeid;
  uint8_t state;
  uint8_t rank;
  uint8_t nexthop;
  uint8_t power_level;
  uint16_t gap_11;
  uint16_t gap_12;
  uint16_t routepkts;
  uint32_t SR_BS_Globalstart_time;
} __attribute ((packed)) routinginfo;

typedef struct path_info{
        uint8_t type;
        uint8_t source_id;
        uint8_t pathToBS[50];               //Paths seperated by 0; 3 2 1 0 3 4 1 0
}path_info;

typedef struct path_broken_pkt {
        uint8_t type;
        uint8_t generating_node_id;
        uint8_t path[MAX_NUM_HOPS];
} __attribute ((packed)) path_broken_pkt;

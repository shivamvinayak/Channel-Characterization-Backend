/* This program has the java Client to C server tcp/ip socket and C server (acting as client)to iwase(or any other tcp/ip device) socket programming  */
/*This program has been edited by Shivam Vinayak to make it compatible with 6LoWPAN. The base station code is being implemented at the server itself */

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <error.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <poll.h>
#include <errno.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/signal.h>
#include <pthread.h>
#include <stdarg.h>
#include <errno.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "inet.h"
#include "commands.h"

typedef message_t *msgptr;

int sockfd;
int GUIclient_sockfd, BS_sock, routing_socketfd, path_broken_info_socketfd;
int link_learning_done = 0;
int relays[100];
int ll_node_array[200];
int nofllnodes = 0;
int stop_data_collection_flag = 0;
FILE *per_list_ptr;
FILE *rssi_list_ptr;
FILE *sendataFp;
FILE *pbp_file_ptr;
FILE *log_ptr;
int power_optimization_bool = 0;
long globaltime_at_basestation = 0;
char file_name_str[200];
int SlotTime;
int FrameTime;
int NoPktsToRoute;
int retrans;
int sources[MAX_NUM_NODES + 1];

struct sockaddr_in iwase_serv_addr, iwase_client_addr;

struct neighbor_list {
//	unsigned char ip_addr[16];
	short node_id;
	short avg_rssi;
	short rxd_pkts;
} neighbor_in_list[32], struct_ele;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//FUNCTIONS//


#define PORT 3000
#define BUFFSIZE 2048
#define MAX_RETRANS 10

void mote_reset_with_id()
{
	message_t buffer;
	int i;
	struct pollfd ufd;
	unsigned int buf[BUFFSIZE];     /* receive buffer */
	char buf_1[INET6_ADDRSTRLEN];
	int pollres, reclen;

	mycommand *commandinfo;

	commandinfo = (mycommand*)buffer.data;
	commandinfo->type = RESET_PKT;
	commandinfo->value = 1;
	//READING THE FILE THAT CONTAINS THE LIST OF NODES ON FIELD
	FILE *link_learningNodesFp;
	link_learningNodesFp = fopen("textfiles/source_array.txt", "r");
	if (link_learningNodesFp == NULL)
		printf("error reading the link learning nodes list from file textfiles/source_array.txt\n");

	int sources[MAX_NUM_NODES + 1];
	for (i = 0; i < MAX_NUM_NODES + 1; i++)
		sources[i] = 0;
	int k = 0;
	int node;
	int num_nodes = 0;
	while (!feof(link_learningNodesFp)) {
		fscanf(link_learningNodesFp, "%d", &node);
		sources[k] = node;
		k++;
		num_nodes++;
	}
	fclose(link_learningNodesFp);
	initcommand *nodes_ptr;
	nodes_ptr = (initcommand*)buffer.data;
	nodes_ptr->type = NODES_ARRAY_PKT;
	printf("\nNodes that are going to get RESET are:\t");
	for (i = 0; i < MAX_NUM_NODES + 1; i++) {
		nodes_ptr->sources[i] = sources[i];
		if (sources[i] != 0)
			printf("%d\t", nodes_ptr->sources[i]);
			//	printf("Nodes :%d\t", nodes_ptr->sources[i]);
	}
	printf("\n");

	config_pkt *ptr = (config_pkt*)buffer.data;
	printf("Reading IP addresses from the list \n");

// Reading IP addresses from the file
	FILE *link_learningNodesFp_ipaddr;
	link_learningNodesFp_ipaddr =
		fopen("textfiles/source_ip_addr.txt", "r");
	if (link_learningNodesFp_ipaddr == NULL)
		printf("error reading the link learning nodes list from file textfiles/source_array.txt\n");

	char sources_ip[MAX_NUM_NODES + 1][32];
	k = 0;
	int num_nodes_ip = 0;

// This piece of code prints the IP addresses of all the nodes
    //printf("Printing the IP addresses of the nodes.... \n ");
	while (!feof(link_learningNodesFp_ipaddr)) {
		fscanf(link_learningNodesFp_ipaddr, "%s",
		       sources_ip[num_nodes_ip]);
		//      printf("%s \n", sources_ip[num_nodes_ip]);
		k++;
		num_nodes_ip++;
	}
	fclose(link_learningNodesFp_ipaddr);
	printf("\n");
// Printing IP addresses of nodes that are going to get Reset..........
	printf("Printing the IP addresses of nodes that are going to get Reset... \n");
	for (i = 0; i < MAX_NUM_NODES + 1; i++) {

		if (sources[i] != 0)
			printf("%s.. Node id is %d:\n", sources_ip[sources[i] - 1], sources[i]);
	}
/* Resetting all the nodes that are going to perform link learning......  */
	printf("Clearing the prior data on the nodes before beginning channel characterization exercise... \n");
	ptr->type = RESET_PKT;

	int j = 0;
	for (j = 0; j < MAX_NUM_NODES; j++) {

		if (sources[j] != 0) {

			printf("\nSending Reset command to the node with IP address- %s whose id is %d \n",
				sources_ip[sources[j] - 1], sources[j]);
			
			ptr->node_id = sources[j];          /*Configure each node with its Node id by Piggybacking the Reset Packet */
			
			int sock;
			socklen_t clilen;
			struct sockaddr_in6 server_addr, client_addr;
			char addrbuf[INET6_ADDRSTRLEN];

			sock = socket(PF_INET6, SOCK_DGRAM, 0);         /* create a DGRAM (UDP) socket in the INET6 (IPv6) protocol */

			if (sock < 0) {
				perror("creating socket");
				exit(1);
			}
// initialising client address
			memset(&client_addr, 0,sizeof(struct sockaddr_in6));
			client_addr.sin6_port = htons(3001);
			client_addr.sin6_family = AF_INET6;
			client_addr.sin6_addr = in6addr_any;

// binding to the socket
			int sin6len;
			sin6len = sizeof(struct sockaddr_in6);
			int status = bind(sock, (struct sockaddr*)&client_addr,sin6len);

			if (-1 == status)
				perror("bind"), exit(1);

			/* create server address: where we want to send to */
			/* clear it out */
			memset(&server_addr, 0, sizeof(server_addr));
			server_addr.sin6_family = AF_INET6;                                             /* it is an INET address */
			inet_pton(AF_INET6, sources_ip[sources[j] - 1], &server_addr.sin6_addr);        /* the server IP address, in network byte order */
			server_addr.sin6_port = htons(PORT);                                            /* the port we are going to send to, in network byte order */
			/* now send a datagram */


			for (retrans = 0; retrans < MAX_RETRANS; retrans++) {

				if (sendto(sock, ptr, sizeof(config_pkt), 0,
					   (struct sockaddr*)&server_addr,
					   sizeof(server_addr)) < 0) {
					perror("sendto failed");
					exit(4);
				}

				ufd.fd = sock;
				ufd.events = POLLIN | POLLPRI;  // check for normal or out-of-band

				pollres = poll(&ufd, 1, 5000);
				reclen = 0;

				if (pollres == -1)
					perror("poll"); // error occurred in poll()
				else if (pollres == 0) {
					printf
						("Timeout occurred! No data till 5 seconds.\n");
					continue;
				} else {
					if (ufd.revents & POLLIN) {
						reclen =
							recvfrom(sock, buf,
								 BUFFSIZE, 0,
								 (struct sockaddr*)
								 &server_addr,
								 &sin6len);
					}
					break;
				}
			}

			sleep(2);
			close(sock);
			printf("moving on to the next node \n ");
		}
	}



}

///////////////////////////////////////////////////////////////////MAIN//////////////////////////////////////////////////////////////////////////////////////////
void main()
{

	char time_str[256];

	//struct sockaddr_in iwase_serv_addr, iwase_client_addr;

	struct sockaddr_in serv_addr, cli_addr;
	int n, m, fd, size, nof_elements = 0;
	FILE *coordfp, *gntfp;
	FILE *act_coord_fp;
	FILE *plink_fp;
	FILE *src_fp;
	int buf;
	float *floatbuf;
	message_t buffer;
	char message[256];
	int i, j = 0, num = 0;
	float P_out_max;
	char recv_buf[1024];
	int source_array[50];
	int number_of_sources;
	int k_value;
	struct pollfd poll_read;

	//BS_sock = connect_to_BSinterface(iwase_serv_addr,iwase_client_addr);//CONNNECT TO IWASE OR BS MACHINE

	mote_reset_with_id();
	exit(0);
}

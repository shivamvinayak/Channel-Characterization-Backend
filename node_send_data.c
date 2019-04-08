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
FILE *rxd_strm_list_ptr;
FILE *sendataFp;
FILE *pbp_file_ptr;
FILE *log_ptr;

char file_name_str[200];
int retrans;
int sources[MAX_NUM_NODES + 1];

struct sockaddr_in iwase_serv_addr, iwase_client_addr;


struct neighbor_rx_str{
	uint8_t type;
	char frag;
	short tx_id;
	short mote_id;
	char rx_strm[101]
}__attribute((packed)) neighbor_rx_strm;

struct neighbor_list {
//	unsigned char ip_addr[16];
	uint8_t tx_id;
	short avg_rssi;
	short rxd_pkts;
	short otg_pkts;//char rxd_strm[50];
} __attribute((packed)) neighbor_in_list[32], struct_ele;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//FUNCTIONS//
#define PORT 3000
#define BUFFSIZE 2048
#define MAX_RETRANS 5
#define IP_ADDR "aaaa::200:0:0:1"


void get_ll_list(){
	message_t buffer;

	char fment;
	int node_id;
	float per;
	int i;
	char sources_ip[MAX_NUM_NODES + 1][32];
	int num_nodes_ip = 0;
	int sources[MAX_NUM_NODES + 1];
	int k = 0;
	int j = 0;
	int node;
	int num_nodes = 0;
	int power = 0;
	//unsigned short numofpkts = 0;
	int numofpkts = 0;
	struct pollfd ufd;
	unsigned int buf[BUFFSIZE];     /* receive buffer */
	char buf_1[INET6_ADDRSTRLEN];
	int reclen,pollres;

	time_t rawtime;
	struct tm * timeinfo;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    printf ( "Current local time and date: %s", asctime (timeinfo));


	per_list_ptr = fopen("textfiles/DF/per_info.txt", "w");
	rssi_list_ptr = fopen("textfiles/DF/rssi_info.txt", "w");
	rxd_strm_list_ptr = fopen("textfiles/DF/rxd_strm_info.txt", "w");	
	
	mycommand *commandinfo;
	commandinfo = (mycommand*)buffer.data;
	commandinfo->type = RESET_PKT;
	commandinfo->value = 1;
	
	//READING THE FILE THAT CONTAINS THE LIST OF NODES ON FIELD
	FILE *link_learningNodesFp;
	link_learningNodesFp = fopen("textfiles/source_array.txt", "r");
	if (link_learningNodesFp == NULL)
		printf
			("error reading the link learning nodes list from file textfiles/source_array.txt\n");

	for (i = 0; i < MAX_NUM_NODES + 1; i++)
		sources[i] = 0;
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
	
	printf("\n");

	config_pkt *ptr = (config_pkt*)buffer.data;
	FILE *InputFp;
	InputFp = fopen("textfiles/link_learning_config.txt", "r");
	if (InputFp == NULL)
		printf("input error\n");
	i = 0;
	while (!feof(InputFp))
		fscanf(InputFp, "%d\t%d", &power, &numofpkts);
	fclose(InputFp);
	ptr->type = SEND_LL_LIST;

	FILE *link_learningNodesFp_ipaddr;
	link_learningNodesFp_ipaddr =
		fopen("textfiles/source_ip_addr.txt", "r");
	if (link_learningNodesFp_ipaddr == NULL)
		printf
			("error reading the link learning nodes list from file textfiles/source_array.txt\n");
	while (!feof(link_learningNodesFp_ipaddr)) {
		fscanf(link_learningNodesFp_ipaddr, "%s",
		       sources_ip[num_nodes_ip]);
		//      printf("%s \n", sources_ip[num_nodes_ip]);
		k++;
		num_nodes_ip++;
	}
	fclose(link_learningNodesFp_ipaddr);
	
// Printing IP addresses of nodes that  are going to perform Link Learning..........
	printf("Printing the IP addresses of Sink Node : ");

	for (i = 0; i < MAX_NUM_NODES + 1; i++)
		if (sources[i] != 0)
			printf("%s\n", sources_ip[sources[i] - 1]);
	 

	printf("\n");
	for (j = 0; j < MAX_NUM_NODES; j++) {
		if (sources[j] == 0)
			break;

		if (sources[j] != 0) {
			node_id = sources[j];
			//printf("Node id is %d \n", node_id);
			int sock;
			socklen_t clilen;
			struct sockaddr_in6 server_addr, client_addr;
			//  char buffer_1[1024];
			char addrbuf[INET6_ADDRSTRLEN];

			sock = socket(PF_INET6, SOCK_DGRAM, 0); /* create a DGRAM (UDP) socket in the INET6 (IPv6) protocol */

			if (sock < 0) {
				perror("creating socket");
				exit(1);
			}
// initialising client address
			memset(&client_addr, 0, sizeof(struct sockaddr_in6));
			client_addr.sin6_port = htons(3001);
			client_addr.sin6_family = AF_INET6;
			client_addr.sin6_addr = in6addr_any;

// binding to the socket
			int sin6len;
			sin6len = sizeof(struct sockaddr_in6);
			int status = bind(sock, (struct sockaddr*)&client_addr,
					  sin6len);

			if (-1 == status)
				perror("bind"), exit(1);

			/* create server address: where we want to send to */
			/* clear it out */
			memset(&server_addr, 0, sizeof(server_addr));
			server_addr.sin6_family = AF_INET6;     /* it is an INET address */
			//  inet_pton(AF_INET6, SERVADDR, &server_addr.sin6_addr);  /* the server IP address, in network byte order */
			
//  Previous line works perferctly, commenting for generalisations

			inet_pton(AF_INET6, sources_ip[sources[j] - 1], &server_addr.sin6_addr);        /* the server IP address, in network byte order */
			server_addr.sin6_port = htons(PORT);                                            /* the port we are going to send to, in network byte order */
			/* now send a datagram */

			for (retrans = 0; retrans < 1; retrans++) {

				if (sendto(sock, ptr, sizeof(config_pkt), 0,
					   (struct sockaddr*)&server_addr,
					   sizeof(server_addr)) < 0) {
					perror("sendto failed");
					exit(4);
				}
				//int recv_count=0;
				ufd.fd = sock;
				ufd.events = POLLIN | POLLPRI; // check for normal or out-of-band

				pollres = poll(&ufd, 1, 5000);
				reclen = 0;

				if (pollres == -1)
					perror("poll"); // error occurred in poll()
				else if (pollres == 0) {
					printf("Timeout occurred! No ack till 5 seconds.\n");
					continue;
				} else {
					if (ufd.revents & POLLIN) {
						reclen =
							recvfrom(sock, buf,
								 BUFFSIZE, 0, (struct sockaddr*)
								 &server_addr, &sin6len);
						
						printf("\nReady to Receive Data \n");
					}
					break;
				}
			}

			while (1) {
				if (ufd.revents) {
					reclen = recvfrom(sock, buf, BUFFSIZE, 0, (struct sockaddr*)&server_addr, &sin6len);
					//recv_pkt_count++;
					printf("Received some data-part\n");
					if (reclen > 100){
						char rxd_strm_data[101] = {0};
						int neb_id;
						printf("Received data : ");
						struct sockaddr_in6 sa;

						struct neighbor_rx_str *neb = (struct neighbor_rx_str*)buf;
						//printf("Received data : ");
						printf("Length of received buffer is: %d, nei: %d \n", reclen, sizeof(struct neighbor_rx_str));
						for (i = 0; i < reclen; i++)
							printf("%02X ", buf[i]);
						printf("\n");

						for (i = 0; i < reclen; i = i + sizeof(struct neighbor_rx_str)) {
							memcpy(rxd_strm_data,&neb->rx_strm,100);
							printf("\nPrinting Received Data...\n");
								
							printf("RX Node : %d Tx Node: %d Fragment %d RX_STRM ",neb->mote_id,neb->tx_id,neb->frag);
							static int ndx;
							for(ndx=0;ndx<101;ndx++)
								printf(" %d,",rxd_strm_data[ndx]);
							printf("\n");
								
							if (strlen(rxd_strm_data) != 0) {
								//printf("Neighbor's node ID: %d \n", neb->tx_id);
								fment = neb->frag;
								neb_id = neb->tx_id;
								node_id = neb->mote_id;
								time ( &rawtime );
        							timeinfo = localtime ( &rawtime );
							
								//printf("Pkt Receive Stream from the neighbour: %s\n",neb->rx_strm);
								FILE *rxd_strm_info_fp;
								rxd_strm_info_fp = fopen("textfiles/DF/rxd_strm_info.txt","a");
								//fprintf(rxd_strm_info_fp,"%d<-%d %s @%s\n", node_id,neb_id,rxd_strm_data,asctime (timeinfo));
								fprintf(rxd_strm_info_fp,"%d %d %d ",node_id,neb_id,fment);
								
								for(ndx=0;ndx<101;ndx++)
									fprintf(rxd_strm_info_fp," %d,",rxd_strm_data[ndx]);
								fprintf(rxd_strm_info_fp," @%s\n",asctime (timeinfo));
								fclose(rxd_strm_info_fp);
								neb = neb + 1;
								}
							}							
						}else if (reclen > 0 && reclen <100 ) {
							int i;
							float nei_per = 0;
							uint8_t nei_id;
							int old_nei_id = 1000; //garbage value
							int no_of_bad_pkts;
							int avg_rssi_data;
							int pkts_in_otg;
							
							struct sockaddr_in6 sa;

							struct neighbor_list *nei = (struct neighbor_list*)buf;
							printf("Received data : ");
							printf("Length of the received buffer is:%d, nei:%d \n",reclen, sizeof(struct neighbor_list));
							for (i = 0; i < reclen; i++)
								printf("%02X ", buf[i]);
							printf("\n");

							for (i = 0; i < reclen;i = i + sizeof(struct neighbor_list)) {
								no_of_bad_pkts = numofpkts - nei->rxd_pkts;
								nei_per = (numofpkts - nei->rxd_pkts)/numofpkts;
								avg_rssi_data = nei->avg_rssi;
								pkts_in_otg = nei->otg_pkts;
								//strcpy(rxd_strm_data,nei->rxd_strm);
			              				// printf("Trying to find the id of the neighbour\n");
								// If a neighbour is found(which updates nei_id, make an entry into the PER file)
								if (nei->tx_id != 0) {
									
									printf("Neighbor's node ID: %d \n", nei->tx_id);
									nei_id = nei->tx_id;
									
									printf("Avg RSSI of the link : %d \n",nei->avg_rssi);
									
									FILE *rssi_info_fp;time ( &rawtime );
        								timeinfo = localtime ( &rawtime );
									
									rssi_info_fp = fopen("textfiles/DF/rssi_info.txt","a");
									fprintf(rssi_info_fp,"%d %d %d @%s\n", node_id,nei->tx_id,avg_rssi_data,asctime (timeinfo));
									fclose(rssi_info_fp);


									printf("No. of LL packets rxd by the neighbour: %d \n",nei->rxd_pkts);

									FILE *per_info_fp;
									per_info_fp = fopen("textfiles/DF/per_info.txt","a");
									fprintf(per_info_fp,"%d %d %d @%s\n", node_id,nei->tx_id, no_of_bad_pkts,asctime (timeinfo));
									fclose(per_info_fp);
									
									FILE *otg_info_fp;
                                    					otg_info_fp = fopen("textfiles/DF/otg_info.txt","a");
									fprintf(otg_info_fp,"%d %d %d @%s\n", node_id,nei->tx_id, pkts_in_otg,asctime (timeinfo));
									fclose(otg_info_fp);
									
									nei = nei + 1;
								}

							}
						}

						continue;
					}
				
			}
			sleep(5);
			close(sock);
			printf("moving on to the next node \n");
			printf("\n");
		}
	}

}



///////////////////////////////////////////////////////////////////MAIN//////////////////////////////////////////////////////////////////////////////////////////
void main()
{

	char time_str[256];

//      struct sockaddr_in iwase_serv_addr, iwase_client_addr;

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

//      BS_sock = connect_to_BSinterface(iwase_serv_addr,iwase_client_addr);//CONNNECT TO IWASE OR BS MACHINE

	get_ll_list();
	exit(0);
}


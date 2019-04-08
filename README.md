# Channel-Characterization-Backend
This repository contains the backend code to control the channel characterization experiment 

node_reset.c : The node reset all the variables on receiving this command and ack the command station.

node_broadcast.c : The node broadcast a large number of packets on receiving this command. All the other nodes logs these packets with rssi and sender's info.

node_send_data.c : The node send the logged information (about all the broadcasts) to the base station.

//server.cpp is what sends the packets to the client
//This file performs the Go-Back-N keeping track of the window
//Usage: ./server port cwnd (./server 10000 4)
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <string>
#include <iostream>
#include <fstream>

#include "packet.h"

using namespace std;

void error(string msg)
{
    cerr << msg << endl;
    exit(1);
}

int main(int argc, char **argv)
{
    int sockfd, portno, cwnd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen = sizeof(serv_addr);
    
    
    portno = atoi(argv[1]);
    if(argc == 3)
        cwnd = atoi(argv[2]);
    
    
    //Setup socket connection - SOCK_DGRAM is for UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0)
        error("ERROR creating socket");
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    
    cout << "Server bound correctly on port "<< portno << endl;
    
    int response_length;
    int seq_num = 0;
    int bytes_loaded = 0;
    int total_bytes = 0;
    Packet packet, ack;
    memset(&packet, 0, sizeof(packet));
    memset(&ack, 0, sizeof(ack));
    ofstream file;

    while(1)
    {
				//recvfrom dumps the message into packet
        if((response_length = recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *) &cli_addr, &clilen)) < 0)
            error("ERROR receiving message");
        
        if(packet.type == INIT)
        {
            file.open(packet.data);
            total_bytes = packet.size;
            bytes_loaded = 0;
        }
        
        //PSEUDO CODE:
        //if packet is received
        //for seq = 0 start new file and write to it
        //for packet in order write to end of file
        //for out of order packet send ACK back with old seq_num
        //if end is received end write to file
        if(packet.type == DATA && seq_num == packet.seq_num)
        {
            
            bytes_loaded = (bytes_loaded >= total_bytes) ? total_bytes : bytes_loaded + packet.size;
            
            seq_num++;
            
            file << packet.data;
            
            ack.seq_num = seq_num;
            
        }
        
        if(total_bytes == bytes_loaded)
        {
            file.close();
        }

        
    }
    
}
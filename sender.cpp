//server.cpp is what sends the packets to the client
//This file performs the Go-Back-N keeping track of the window
//Usage: ./sender port cwnd (./sender 10000 4)
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <string>
#include <iostream>
#include <fstream>

#include <ctime>
#include <cstdlib>

#include "packet.h"

using namespace std;

const int MAX_PKTS = 100;

clock_t startTime;
const int TIMEOUT = 5;

void error(string msg)
{
    cerr << msg << endl;
    exit(1);
}

void start_timer()
{
		startTime = clock();
}

void stop_timer()
{
		startTime = 0;
}

int main(int argc, char **argv)
{
    int sockfd, portno, cwnd;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    
    
    if (argc < 3) {
        fprintf(stderr,"usage: port cwnd\n");
        exit(0);
    }
    
    portno = atoi(argv[1]);
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
    
    Packet packet, ack;
    memset(&packet, 0, sizeof(packet));
    memset(&ack, 0, sizeof(ack));
    
    Packet packets[MAX_PKTS];
    memset(&packets, 0, sizeof(packets));
    
    ifstream file;
    char c;
    int curr_pkt = 0;
    int curr_byte = 0;
    
    int response_length;
    int seq_num = 0;
    int base = 0;
    int total_bytes = 0;
    
    double secondsPassed;
    
    while(1)
    {
        //recvfrom dumps the message into packet
        if((response_length = recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *) &cli_addr, &clilen)) < 0)
            error("ERROR receiving message");
        
        //Got a request from client for a file
        if(packet.type == REQ)
        {
        		
        		//Open file and break into packets
            file.open(packet.data);
            total_bytes = 0;
            while(file.get(c))
            {
            		//Current packet full, move to next one
            		if(packets[curr_pkt].size > PACKET_SIZE)
            		{
            				curr_pkt++;
            		}
            		
            		//Initialize other packet data
            		if(packets[curr_pkt].size == 0)
            		{
            				packets[curr_pkt].type = DATA;
										packets[curr_pkt].server_portno = serv_addr.sin_port;
										packets[curr_pkt].client_portno = sockfd;
										packets[curr_pkt].seq_num = total_bytes;
            		}
            		
            		//Add byte to packet data
            		packets[curr_pkt].data[ packets[curr_pkt].size++ ] = c;
            		total_bytes++;
            				
            }
            
            file.close();
            
            //Send initial packets to client
            for(int i=0; i < cwnd; i++)
            {
            		if (sendto(sockfd, &packets[seq_num], sizeof(packets[seq_num]), 0, 
            		(struct sockaddr *)&cli_addr, clilen) < 0)
                	error("ERROR sending DATA");
            		if(base == seq_num)
            				start_timer();
            		seq_num++;
            } 
            cout << "Sending initial packets to client" << endl;
        				
        }
        
        //Respond to ACK
        if(packet.type == ACK)
        {
        		//upon ACK, shift the window and send new packet
        		base = packet.seq_num + 1;
        		cout << "Sender received ack " << packet.seq_num << endl;
        }
        
        //Check for timeout
        secondsPassed = (clock() - startTime) / CLOCKS_PER_SEC;
        if (secondsPassed >= TIMEOUT)
        {
        		//Resend all packets up to current sequence number
            for(int i=base; i < seq_num-1; i++)
            {
            		if (sendto(sockfd, &packets[i], sizeof(packets[i]), 0, 
            		(struct sockaddr *)&cli_addr, clilen) < 0)
                	error("ERROR sending DATA");

            } 
        }
        
        //Try to send another packet
        if(seq_num < base + cwnd)
        {
            if (sendto(sockfd, &packets[seq_num], sizeof(packets[seq_num]), 0, 
            (struct sockaddr *)&cli_addr, clilen) < 0)
                error("ERROR sending DATA");
            if(base == seq_num)
            				start_timer();
            		seq_num++;
        }
        
    } //end of while
    return 0;
}
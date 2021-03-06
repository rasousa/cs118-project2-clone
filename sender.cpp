//server.cpp is what sends the packets to the client
//This file performs the Go-Back-N keeping track of the window
//Usage: ./sender port cwnd (./sender 10000 4) 
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include <ctime>
#include <cstdlib>
#include <time.h>

#include "packet.h"

using namespace std;

const int MAX_PKTS = 10000;

clock_t startTime;
const double TIMEOUT = 2; //times out after 3 seconds

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
		startTime = -1;
}

int main(int argc, char **argv)
{
    int sockfd, portno, cwnd;
    float pl, pc;
    struct sockaddr_in serv_addr, cli_addr;
    struct timeval tv;
    
    
    time_t timer = 0;
    socklen_t clilen = sizeof(serv_addr);
    
    if (argc < 3) {
        fprintf(stderr,"usage: port cwnd\n");
        exit(0);
    }
    
    portno = atoi(argv[1]);
    cwnd = atoi(argv[2]);
    pl = 100*atof(argv[3]);
    pc = 100*atof(argv[4]);
    
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
        tv.tv_sec = 30000;
        tv.tv_usec = 0;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
        if((response_length = recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *) &cli_addr, &clilen)) < 0)
            cout << "No message received yet" << endl;
        
        //Got a request from client for a file
        
        //print_packet(&packet);
        if(packet.type == REQ)
        {
        		//Open file and break into packets
            file.open(packet.data);
            total_bytes = 0;
            while(file.get(c))
            {
            		//Current packet full, move to next one
            		if(packets[curr_pkt].size > PACKET_SIZE - 1)
            		{
            				curr_pkt++;
            		}
            		
            		//Initialize other packet data
            		if(packets[curr_pkt].size == 0)
            		{
            				packets[curr_pkt].type = DATA;
                            packets[curr_pkt].server_portno = serv_addr.sin_port;
                            packets[curr_pkt].client_portno = sockfd;
                            packets[curr_pkt].seq_num = curr_pkt;
            		}
            		
            		//Add byte to packet data
            		packets[curr_pkt].data[ packets[curr_pkt].size++ ] = c;
            		total_bytes++;

            }
            
            file.close();
            
            Packet init;
            memset(&init, 0, sizeof(init));
            init.type = INIT;
            init.size = total_bytes;
            
            if (sendto(sockfd, &init, sizeof(init), 0, (struct sockaddr *)&cli_addr, clilen) < 0) {
                error("ERROR sending INIT");
            }
            
            //Send initial packets to client
            for(int i=0; i < cwnd; i++)
            {
                Packet temp = packets[seq_num];
                cout << seq_num << ":" << temp.seq_num << endl;
                
                if (sendto(sockfd, &temp, sizeof(temp), 0, (struct sockaddr *)&cli_addr, clilen) < 0) {
                    error("ERROR sending DATA");
                }
                if(base == seq_num)
                    time(&timer);
                seq_num++;
            } 
            cout << "Sending initial packets to client" << endl;
        				
        }
        
        while (base < curr_pkt)
        {
            tv.tv_sec = TIMEOUT;
            tv.tv_usec = 0;
            setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
            //cout << "Hi! Inside second while loop!" << endl;
						//Check for timeout
            //secondsPassed = (clock() - startTime) / CLOCKS_PER_SEC;
            //cout << "seconds passed = " << secondsPassed << endl;
            //cout << "start time = " << startTime << endl;
            //cout << "Timer " << timer << " Current Time " << time(NULL) << endl;
//            cout << "[Base1,Curr] [" << base << "," << curr_pkt << "]" << endl;
            if (timer + TIMEOUT < time(NULL))
            {
                    cout << "Timer " << timer << " Current Time " << time(NULL) << endl;
                    //cout << "Sender timed out, resending packets starting from " << base << endl;
                    //start_timer();
                    //Resend all packets up to current sequence number
                resend:
                    time(&timer);
                    for(int i=base; i < seq_num; i++)
                    {
                        Packet temp = packets[i];
                        //cout << seq_num << ":" << temp.seq_num << "=" << endl;
                        if (sendto(sockfd, &packets[i], sizeof(packets[i]), 0, (struct sockaddr *)&cli_addr, clilen) < 0)
                            error("ERROR sending DATA");

                    }
            }
//            cout << "[Base2,Curr] [" << base << "," << curr_pkt << "]" << endl;
//            cout << "Request ACK" << endl;
            if((recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *) &cli_addr, &clilen)) > 0)
            {
        
                //print_packet(&packet);

                int plrand = rand()%100;
                int pcrand = rand()%100;

                if (pl > plrand)
                {
                        cout << "Packet number: " << packet.seq_num << " lost." << endl;
                        continue;
                }
                else if (pc > pcrand)
                {
                        cout << "Packet number: " << packet.seq_num << " corrupted." << endl;
                        continue;
                }						

                //Respond to ACK
                if(packet.type == ACK_CORRUPT)
                {
                    goto resend;
                }
                if(packet.type == ACK)
                {
                        //upon ACK, shift the window and send new packet
                    base = packet.seq_num + 1;
                    if(base == seq_num)
                        time(&timer);
                        //cout << "Sender received ack " << packet.seq_num << endl;
                }



                //Try to send another packet
                if(seq_num < base + cwnd)
                {
                        if (sendto(sockfd, &packets[seq_num], sizeof(packets[seq_num]), 0, 
                        (struct sockaddr *)&cli_addr, clilen) < 0)
                            error("ERROR sending DATA");
                        if(base == seq_num)
                            time(&timer);
                        seq_num++;
                        //cout << "Sent packet with sequence number " << seq_num-1 << endl;
                }
            }
//            cout << "[Base3,Curr] [" << base << "," << curr_pkt << "]" << endl;
        }
    }
        
     //end of while
    return 0;
}

#define FILENAME_SIZE 256
#define PACKET_SIZE 512

using namespace std;

enum PType { DATA, ACK, INIT, REQ }; //Might need a few more types

struct Packet {
    PType type;
    int server_portno;
    int client_portno;
    int seq_num;
    int size;
    char data[PACKET_SIZE];
};

void print_packet(Packet *packet)
{
    cout << "Type: " << packet->type << endl;
    cout << "Server Port: " << packet->server_portno << endl;
    cout << "Client Port: " << packet->client_portno << endl;
    cout << "Sequence Number: " << packet->seq_num << endl;
    cout << "Size: " << packet->size << endl;
    cout << "Data: " << endl;
    //cout << packet->data << endl;
    cout << endl;
}
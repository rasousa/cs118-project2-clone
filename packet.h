#define FILENAME_SIZE 256
#define PACKET_SIZE 512

using namespace std;

enum PType { DATA, ACK, REQ, INIT }; //Might need a few more types

struct Packet {
    PType type;
    int server_portno;
    int client_portno;
    int seq_num;
    int size;
    char data[PACKET_SIZE];
};
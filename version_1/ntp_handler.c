#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>


#define NTP_TIMESTAMP_DELTA 2208988800ull

// #define LI(packet)   (uint8_t) ((packet.li_vn_mode & 0xC0) >> 6) // (li   & 11 000 000) >> 6
// #define VN(packet)   (uint8_t) ((packet.li_vn_mode & 0x38) >> 3) // (vn   & 00 111 000) >> 3
// #define MODE(packet) (uint8_t) ((packet.li_vn_mode & 0x07) >> 0) // (mode & 00 000 111) >> 0



int server_port_number = 123; // NTP UDP port number.
char* host_name = "us.pool.ntp.org"; // NTP server host-name.


typedef struct
  {
    // the first row

    uint8_t li_vn_mode;      // Eight bits. li, vn, and mode.
                             // li.   Two bits.   Leap indicator.
                             // vn.   Three bits. Version number of the protocol.
                             // mode. Three bits. Client will pick mode 3 for client.

    uint8_t stratum;         // Eight bits. Stratum level of the local clock.
    uint8_t poll;            // Eight bits. Maximum interval between successive messages.
    uint8_t precision;       // Eight bits. Precision of the local clock.

  // ------ the second row

    uint32_t rootDelay;      // 32 bits. Total round trip delay time.
    uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
    uint32_t refId;          // 32 bits. Reference clock identifier.

    uint32_t refTm_s;        // 32 bits. Reference time-stamp seconds.
    uint32_t refTm_f;        // 32 bits. Reference time-stamp fraction of a second.

    uint32_t origTm_s;       // 32 bits. Originate time-stamp seconds.
    uint32_t origTm_f;       // 32 bits. Originate time-stamp fraction of a second.

    uint32_t rxTm_s;         // 32 bits. Received time-stamp seconds.
    uint32_t rxTm_f;         // 32 bits. Received time-stamp fraction of a second.

    uint32_t txTm_s;         // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
    uint32_t txTm_f;         // 32 bits. Transmit time-stamp fraction of a second.

  } ntp_packet;              // Total: 384 bits or 48 bytes.


void error( char* msg )
{
    perror( msg );
    exit( 0 ); 
}

int create_ntp_packet(ntp_packet *packet){
  memset( packet, 0, sizeof( ntp_packet ));
  // Set the first byte's bits to 00,011,011 for li = 0, vn = 3, and mode = 3. The rest will be left set to zero.
  *( ( char * ) packet + 0 ) = 0b00011011;
  return 0;
}

int create_udp_socket(int* udp_sockfd){
  // Create a UDP socket, convert the host-name to an IP address, set the port number,
  // connect to the server, send the packet, and then read in the return packet.
  *udp_sockfd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ); // Create a UDP socket.

  // AF_INET - AF- adresses, INET - from the Internet
  return 0;
}

int form_ntp_server_ip_address(struct sockaddr_in* server_ip_address){
  struct hostent *server;     
  server = gethostbyname( host_name ); // Convert URL to IP.
  
  if ( server == NULL )
    error( "ERROR, no such host" );

  server_ip_address -> sin_family = AF_INET;

  // Copy the server's IP address to the server address structure.
  bcopy( ( char* )server->h_addr, ( char* ) &(server_ip_address->sin_addr.s_addr), server->h_length );

  // Convert the port number integer to network big-endian style and save it to the server address structure.
  server_ip_address -> sin_port = htons( server_port_number );
  return 0;

}

int connect_to_ntp_server(int udp_sockfd, struct sockaddr_in server_ip_address){
  // Call up the server using its IP address and port number.
  return connect( udp_sockfd, ( struct sockaddr * ) &server_ip_address, sizeof( server_ip_address) );
}


int send_packet_to_server(int *fd, ntp_packet *packet){
  char * bytearray = (char*)packet;
  uint16_t send_total = 0;
  uint16_t send_len = 0;
  uint16_t ret = 0;
  do {
        if ((send_len = send(*fd, bytearray + send_total, sizeof(ntp_packet) - send_total, 0)) < 0) {
            if (errno == EINTR || errno == EAGAIN) {
                continue;
            }
            ret = -1;
        }

        send_total += send_len;
    } while (send_total < sizeof(ntp_packet));

  return ret;
}

int receive_packet_from_server(int *fd, ntp_packet *packet){
  char * bytearray = (char*)packet;
  uint16_t recv_total = 0;
  uint16_t recv_len = 0;
  uint16_t ret = 0;
  do {
        if ((recv_len = recv(*fd, bytearray + recv_total, sizeof(ntp_packet) - recv_total, 0)) < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            ret = -1;
        }

        recv_total += recv_len;
    } while (recv_total < sizeof(ntp_packet));

  return ret;
}


int convert_received_to_time(ntp_packet *packet, time_t *timestamp_seconds){
  // These two fields contain ttohe time-stamp seconds as the packet left the NTP server.
  // The number of seconds correspond to the seconds passed since 1900.
  // ntohl() converts the bit/byte order from the network's to host's "endianness".

  packet -> txTm_s = ntohl( packet -> txTm_s ); // Time-stamp seconds.
  packet -> txTm_f = ntohl( packet -> txTm_f ); // Time-stamp fraction of a second.

  // Extract the 32 bits that represent the time-stamp seconds (since NTP epoch) from when the packet left the server.
  // Subtract 70 years worth of seconds from the seconds since 1900.
  // This leaves the seconds since the UNIX epoch of 1970.
  // (1900)------------------(1970)**************************************(Time Packet Left the Server)

  *timestamp_seconds = ( time_t ) ( packet ->txTm_s - NTP_TIMESTAMP_DELTA );
}

// void printf_ctime(time_t *timestamp_seconds){
//     // Print the time we got from the server, accounting for local timezone and conversion from UTC time.
//     printf( "Time: %s", ctime( ( const time_t* ) &(timestamp_seconds) ) );
// }

int receive_ntp_server_time(time_t *timestamp_seconds)
{
  
  ntp_packet packet;
  create_ntp_packet(&packet);

  int udp_sockfd;
  create_udp_socket(&udp_sockfd);
  if (udp_sockfd < 0)
    error( "ERROR opening socket" );


  struct sockaddr_in server_ip_address;
  form_ntp_server_ip_address(&server_ip_address);


  int socket_result; 
  socket_result = connect_to_ntp_server(udp_sockfd, server_ip_address);
  if (socket_result < 0)
    error( "ERROR connecting to the NTP server" );


  socket_result = send_packet_to_server(&udp_sockfd, &packet);
  if (socket_result < 0)
    error( "ERROR writing to socket" );


  socket_result = receive_packet_from_server(&udp_sockfd, &packet);
  if ( socket_result < 0 )
    error( "ERROR reading from socket" );


  // time_t timestamp_seconds;
  convert_received_to_time(&packet, timestamp_seconds);

  // printf_ctime(timestamp_seconds);
  return socket_result;
}

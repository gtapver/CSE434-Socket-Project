// Implements the client side of an echo client-server application program.
// The client reads ITERATIONS strings from stdin, passes the string to the
// server, which simply echoes it back to the client.
//
// Compile on general.asu.edu as:
//   g++ -o client UDPEchoClient.c
//
// Only on general3 and general4 have the ports >= 1024 been opened for
// application programs.
#include <stdio.h>      // for printf() and fprintf()
#include <sys/socket.h> // for socket(), connect(), sendto(), and recvfrom()
#include <arpa/inet.h>  // for sockaddr_in and inet_addr()
#include <stdlib.h>     // for atoi() and exit()
#include <string.h>     // for memset()
#include <unistd.h>     // for close()
#include "defns.h"

#define TWEETMAX 140      // Longest tweet to send

void DieWithError( const char *errorMessage ) // External error handling function
{
    perror( errorMessage );
    exit(1);
}

void printMenu(){
	printf("\nPlease select one of the following options:\n");
	printf("1) Query Handles\n");
	printf("2) Follow Other User\n");
	printf("3) Stop Following User\n");
	printf("4) Tweet to Followers\n");
	printf("Q) Quit\n");
	return;
}

void registerUser(char *ip_ports){
	printf("\n\nRegistering with tracker\n\n");

	struct message registerMessage = {.type=REQUEST, .command=REGISTER, .status=INPROGRESS, .message=ip_ports}; 
	
	printf("\nType of Message: %d\nCommand: %d\nStatus: %d\nMessage: %s\n", registerMessage.type, registerMessage.command, registerMessage.status, registerMessage.message);
	printf("\n\nFinished Registering\n\n");
	return;
}

void query(){
	printf("\nYou Chose to Query\n");
	
	//initialize a message to send to server


	//end of request
	return;
}

void follow(){
	printf("\nYou Chose to Follow\n");
	return;
}

void unfollow(){
	printf("\nYou Chose to Unfollow\n");
	return;
}

void tweet(){
	printf("\nYou Chose to Tweet\n");
	return;
}

void quitTweeter(){
	printf("\nYou Chose to Quit\n");
	return;
}

int main( int argc, char *argv[] )
{
    size_t nread;
    int sock;                        // Socket descriptor
    struct sockaddr_in servAddr; // Server address
    struct sockaddr_in fromAddr;     // Source address of message
    unsigned short servPort;     // Server port
    unsigned int fromSize;           // In-out of address size for recvfrom()
    char *servIP;                    // IP address of server
    char *string = NULL;         // String to send to Server
    size_t stringLen = TWEETMAX;               // Length of string to tweet
    int respStringLen;               // Length of received response

    string = (char *) malloc( TWEETMAX );

    registerUser("255.255.255.255 4000 5000 6000");
    if (argc < 3)    // Test for correct number of arguments
    {
        fprintf( stderr, "Usage: %s <Server IP address> <Echo Port>\n", argv[0] );
        exit( 1 );
    }

    servIP = argv[ 1 ];  // First arg: server IP address (dotted decimal)
    servPort = atoi( argv[2] );  // Second arg: Use given port

    printf( "client: Arguments passed: server IP %s, port %d\n", servIP, servPort );

    // Create a datagram/UDP socket
    if( ( sock = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) < 0 )
        DieWithError( "client: socket() failed" );

    // Construct the server address structure
    memset( &servAddr, 0, sizeof( servAddr ) ); // Zero out structure
    servAddr.sin_family = AF_INET;                  // Use internet addr family
    servAddr.sin_addr.s_addr = inet_addr( servIP ); // Set server's IP address
    servAddr.sin_port = htons( servPort );      // Set server's port

    while(1)
    {
	printMenu();
	if( ( nread = getline( &string, &stringLen, stdin ) ) != -1 )
	{
		string[ (int) strlen( string) - 1] = '\0';
	}
	else
		DieWithError(" client: error reading string to echo\n" );


	if(strcmp(string, "1") == 0)
		query();
	else if(strcmp(string, "2") == 0)
		follow();
	else if(strcmp(string, "3") == 0)
		unfollow();
	else if(strcmp(string, "4") == 0)
		tweet();
	else if(strcmp(string, "Q") == 0 || strcmp(string, "q") == 0)
		break;
	else
		printf("\n\nPlease Choose Valid Choice\n\n");
    }
    quitTweeter();
    close( sock );
    exit( 0 );
}


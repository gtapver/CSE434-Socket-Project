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
    size_t stringLen = MAX_MESSAGE;               // Length of string to tweet
    int respStringLen;               // Length of received response
    
    struct sockaddr_in servFacingAddr;
    struct sockaddr_in inputAddr;
    struct sockaddr_in outputAddr;
    char *localHandle;
    char *localIP;
    unsigned short servFacingPort;  //local server facing port
    unsigned short inputPort;     //local input port
    unsigned short outputPort;    //local output port

    string = (char *) malloc( MAX_MESSAGE );
    servIP = "10.120.70.145";  //server IP address (dotted decimal)
    servPort = atoi( "46499" );  //Use given port

    printf( "client: Arguments passed: server IP %s, port %d\n", servIP, servPort );

    // Create a datagram/UDP socket
    if( ( sock = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) < 0 )
        DieWithError( "client: socket() failed" );

    // Construct the server address structure
    memset( &servAddr, 0, sizeof( servAddr ) ); // Zero out structure
    servAddr.sin_family = AF_INET;                  // Use internet addr family
    servAddr.sin_addr.s_addr = inet_addr( servIP ); // Set server's IP address
    servAddr.sin_port = htons( servPort );      // Set server's port

    //Register the User with the server
    printf("\nPlease select handle, IP address, and ports to use");
    printf("\nInput in format @<handle> <IP Address> <Server Facing Port> <Input Port> <Output Port>:\n");
    if( ( nread = getline( &string, &stringLen, stdin ) ) != -1 )
    {
	//Instantiate field
	char tmp[50]= "000";
	printf("Field: %s", tmp);
	//Read Handle
	localHandle = strtok(string, " ");
	if(localHandle[0] == '@' && strlen(localHandle) <= MAX_HANDLE+1 && strlen(localHandle) > 2){
		strcat(tmp, localHandle); 
		strcat(tmp, "$");
	}
	else
		DieWithError("client: invalid handle  ");
	//Read IP
	localIP = strtok(NULL, " ");
	strcat(tmp, localIP);
	//Read Ports
	char *token;
	//read server facing port
	token = strtok(NULL, " ");
	servFacingPort = atoi(token);
	strcat(tmp, "$");
	strcat(tmp, token);
	//read input port
	token = strtok(NULL, " ");
	inputPort = atoi(token);
	strcat(tmp, "$");
	strcat(tmp, token);
	//read output port
	token = strtok(NULL, " ");
	outputPort = atoi(token);
	strcat(tmp, "$");
	strcat(tmp, token);
        //send entire message to tracker
	string = tmp;
	printf("\n%s\n", string);
	string[ (int) strlen( string) - 1] = '\0';
	if( sendto( sock, string, strlen( string ), 0, (struct sockaddr *) &servAddr, sizeof( servAddr ) ) != strlen(string) )
		DieWithError( "client: sendto() sent a different number of bytes than expected" );
    }
    else
	DieWithError(" client: error reading handle, IP, or ports\n");



    while(1)
    {
	printMenu();
	if( ( nread = getline( &string, &stringLen, stdin ) ) != -1 )
	{
		string[ (int) strlen( string) - 1] = '\0';
	}
	else
		DieWithError(" client: error reading option selected\n" );


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


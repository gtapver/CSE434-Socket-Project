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
#include <stdbool.h>
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
	printf("Selected Option: ");
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
    char *thisHandle = (char *) malloc(MAX_HANDLE);
    string = (char *) malloc( MAX_MESSAGE );
    servIP = "10.120.70.106";  //server IP address (dotted decimal)
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
    printf("\nInput in format: \n@<handle> <IP Address> <Server Facing Port> <Input Port> <Output Port>:\n");
    if( ( nread = getline( &string, &stringLen, stdin ) ) != -1 )
    {
	//Instantiate field
	char tmp[50]= "000";
	//Read Handle
	localHandle = strtok(string, " ");
	if(localHandle[0] == '@' && strlen(localHandle) <= MAX_HANDLE+1 && strlen(localHandle) > 2){
		strcat(tmp, localHandle); 
		strcat(tmp, "$");
		strcat(thisHandle, localHandle);
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
	//printf("\nSending following string ``%s''\n", string);
	//replace new line
	string[ (int) strlen( string) - 1] = '\0';
	if( sendto( sock, string, strlen( string ), 0, (struct sockaddr *) &servAddr, sizeof( servAddr ) ) != strlen(string) )
		DieWithError( "client: sendto() sent a different number of bytes than expected" );
	fromSize = sizeof( fromAddr );
	if( ( respStringLen = recvfrom( sock, string, MAX_MESSAGE, 0, (struct sockaddr *) &fromAddr, &fromSize ) ) > MAX_MESSAGE )
		DieWithError("client: Error: received a packet from unknown source.\n");
	string[ respStringLen ] = '\0';
	if( servAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr )
		DieWithError("client: Error: received a packet from unknown source.\n");
	if(strcmp(string, "103") == 0)
		printf("\nclient: successfully registered with tracker.\n");
	else
		DieWithError("client: Error: registration failed.\n");
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
		DieWithError(" client: error reading option selected" );

	if(strcmp(string, "1") == 0){ //query
		strcpy(string, "010"); //message being sent, this is just the flags
		string[ (int) strlen( string )] = '\0';
		if(sendto(sock, string, strlen(string), 0, (struct sockaddr *) &servAddr, sizeof( servAddr ) ) != strlen(string) ) //send message
			DieWithError( "client: Error: unable to send message");
		fromSize = sizeof( fromAddr );
		if( ( respStringLen = recvfrom( sock, string, MAX_MESSAGE, 0, (struct sockaddr *) &fromAddr, &fromSize ) ) > MAX_MESSAGE ) //receive message
			DieWithError("client: Error: received a packet from unknown source. ");
		if(string[0] == '1' && string[1] == '1' && (string[2] == '1' || string[2] == '3')){//ensure received message has valid flags
			bool moreIncoming;//boolean to see how many more messages we will be receiving from the tracker
			int totalCount;//how many total handles there are
			int currCount = 0;//how many handles will be received this message
			int maxHandleInMessage = (MAX_MESSAGE - 4)/(MAX_HANDLE+1);
			if(string[2] == '1') //if the flag says "in progress"
				moreIncoming = true;
			else //if the flag says "success" ie complete
				moreIncoming = false;
			memmove(string, string+3, strlen(string)); //get rid of the flags
			sscanf(strtok(string, "$"), "%d", &totalCount); //read how many handles there are total
			printf("\nTotal User Count: %d\n", totalCount); //print total amount of users
			for(int i = 0; i < totalCount && i < maxHandleInMessage; i++){
				printf("User Handle: %s\n", strtok(NULL, "$")); //print each handle in the message
				currCount++;
			}
			while(moreIncoming){ //if we are expecting more message from the tracker
				if( ( respStringLen = recvfrom( sock, string, MAX_MESSAGE, 0, (struct sockaddr *) &fromAddr, &fromSize ) ) > MAX_MESSAGE ) //receive message
					DieWithError("client: Error: received a packet from unknown source. ");
				if(string[0] != '1' || string[1] != '1' || (string[2] != '1' && string[2] != '3')) //if the message is not in progress or successful
					DieWithError("client: Error: received an unknown message"); //error
				if(string[2] == '3') //if this is the last message, end loop after finishing this message
					moreIncoming = false;
				memmove(string, string+3, strlen(string)); //remove flags
				for(int i = 0; i < maxHandleInMessage && currCount < totalCount ; i++) 
					printf("User Handle: %s\n", strtok(NULL, "$")); //print each handle in the message
			}
		}
		else
			DieWithError("client: Error: unknown message received. ");
	}
	else if(strcmp(string, "2") == 0){
		printf("\nPlease enter the desired handles in the form of: \n@<Handle of new follower> @<Handle of user to follow>\n");
		if( ( nread = getline( &string, &stringLen, stdin ) ) != -1 ){
			string[ (int) strlen(string)] = '\0';
			char *tmp = (char*) malloc(MAX_HANDLE);
			strcpy(tmp, "020");
			char *tkn = strtok(string, " ");
			strcat(tmp, tkn);
			strcat(tmp, "$");
			tkn = strtok(NULL, "\n");
			strcat(tmp, tkn);
			strcat(tmp, "$");
			strcpy(string, tmp);
			string[ (int) strlen( string )] = '\0';
			free(tmp);
			//printf("Sending following string: ``%s''\n", string);
			if(sendto(sock, string, strlen(string), 0, (struct sockaddr *) &servAddr, sizeof( servAddr ) ) != strlen(string) ) //send message
				DieWithError("client: Error: Socket Error. ");
			if( ( respStringLen = recvfrom( sock, string, MAX_MESSAGE, 0, (struct sockaddr *) &fromAddr, &fromSize ) ) > MAX_MESSAGE ) //receive message
                                        DieWithError("client: Error: received a packet from unknown source. ");
			if(string[0] != '1' && string[1] != '2')
				DieWithError("client: Error: received a packet from unknown source. ");
			else if(string[2] == '3')
				printf("client: successfully followed\n");
			else if(string[2] == '2')
				printf("client: failed to follow\n");
			else
				DieWithError("client: Error: unable to read message. ");
		}
		else
			DieWithError("client: Error: invalid input. ");
	}
	else if(strcmp(string, "3") == 0){
                printf("\nPlease enter the desired handles in the form of: \n@<Handle of follower> @<Handle of user to stop following>\n");
                if( ( nread = getline( &string, &stringLen, stdin ) ) != -1 ){
                        string[ (int) strlen(string)] = '\0';
                        char *tmp = (char*) malloc(MAX_HANDLE);
                        strcpy(tmp, "030");
                        char *tkn = strtok(string, " ");
                        strcat(tmp, tkn);
                        strcat(tmp, "$");
                        tkn = strtok(NULL, "\n");
                        strcat(tmp, tkn);
                        strcat(tmp, "$");
                        strcpy(string, tmp);
                        string[ (int) strlen( string )] = '\0';
                        free(tmp);
                        //printf("Sending following string: ``%s''\n", string);
                        if(sendto(sock, string, strlen(string), 0, (struct sockaddr *) &servAddr, sizeof( servAddr ) ) != strlen(string) ) //send message
                                DieWithError("client: Error: Socket Error. ");
                        if( ( respStringLen = recvfrom( sock, string, MAX_MESSAGE, 0, (struct sockaddr *) &fromAddr, &fromSize ) ) > MAX_MESSAGE ) //receive message
                                        DieWithError("client: Error: received a packet from unknown source. ");
                        if(string[0] != '1' && string[1] != '3')
                                DieWithError("client: Error: received a packet from unknown source. ");
                        else if(string[2] == '3')
                                printf("client: successfully Unfollowed\n");
                        else if(string[2] == '2')
                                printf("client: failed to unfollow\n");
                        else
                                DieWithError("client: Error: unable to read message. ");
                }
                else
                        DieWithError("client: Error: invalid input. ");
	}
	else if(strcmp(string, "4") == 0)
		tweet();
	else if(strcmp(string, "Q") == 0 || strcmp(string, "q") == 0){
		strcpy(string, "060");
		strcat(string, thisHandle);
		strcat(string, "$");
		//printf("Sending following string: ``%s''\n", string);
		if(sendto(sock, string, strlen(string), 0, (struct sockaddr *) &servAddr, sizeof( servAddr ) ) != strlen(string) ) //send message
                               DieWithError("client: Error: Socket Error. ");
		if( ( respStringLen = recvfrom( sock, string, MAX_MESSAGE, 0, (struct sockaddr *) &fromAddr, &fromSize ) ) > MAX_MESSAGE ) //receive message
                                DieWithError("client: Error: received a packet from unknown source. ");
		if(string[0] != '1' && string[1] != '6')
                                DieWithError("client: Error: received a packet from unknown source. ");
		else if(string[2] == '3')
			printf("client: successfully quite Tweeter\n");
		else
			printf("client: could not delete user from database\n");
		break; //exit loop
	}
	else
		printf("\n\nPlease Choose Valid Choice\n\n");
    }
    quitTweeter();
    close( sock );
    exit( 0 );
}


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
#include <sys/time.h>
#include <sys/types.h>
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
	//printf("Selected Option: ");
	return;
}

int main( int argc, char *argv[] )
{
    size_t nread;
    int servSock;                        // Socket descriptor for server socket
    struct sockaddr_in servAddr; // Server address
    struct sockaddr_in fromAddr;     // Source address of message
    unsigned short servPort;     // Server port
    unsigned int fromSize;           // In-out of address size for recvfrom()
    char *servIP;                    // IP address of server
    char *string = NULL;         // String to send to Server
    size_t stringLen = MAX_MESSAGE;               // Length of string to tweet
    int respStringLen;               // Length of received response
    
    int peerSock;
    struct sockaddr_in peerAddr;
    //reuse fromAddr as server clntAddr
    //reuse respStringLen for length of incoming message
    //reuse string as buffer
    unsigned short peerPort;     //local peer2peer, set later

    char *thisHandle = (char *) malloc(MAX_HANDLE);
    char *peerHandle = (char *) malloc(MAX_HANDLE);
    strcpy(peerHandle, "");
    char *thisIP = (char *) malloc(MAX_IP);
    string = (char *) malloc( MAX_MESSAGE );

    //setup timeout timer
    fd_set rfds;
    struct timeval tv;
    int retval;

    if( argc < 2){ //make sure the ip is passed on the arguments
	fprintf( stderr, "Usage: %s <Server IP address>\n", argv[0] );
	exit(1);
    }
    servIP = argv[ 1 ];
    servPort = atoi( "46499" );  //Use given port

    printf( "client: Arguments passed: server IP %s, port %d\n", servIP, servPort );

    // Create a datagram/UDP socket
    if( ( servSock = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) < 0 )
        DieWithError( "client: socket() failed" );

    // Construct the server address structure
    memset( &servAddr, 0, sizeof( servAddr ) ); // Zero out structure
    servAddr.sin_family = AF_INET;                  // Use internet addr family
    servAddr.sin_addr.s_addr = inet_addr( servIP ); // Set server's IP address
    servAddr.sin_port = htons( servPort );      // Set server's port

    //Register the User with the server
    printf("\nPlease select handle, IP address, and ports to use");
    printf("\nInput in format: \n@<handle> <IP Address> <Peer2Peer Port>:\n");
    if( ( nread = getline( &string, &stringLen, stdin ) ) != -1 )
    {
	//Instantiate field
	char tmp[50]= "000";
	//Read Handle
	strcpy(thisHandle, strtok(string, " "));
	//ensure the handle is valid
	if(thisHandle[0] == '@' && strlen(thisHandle) <= MAX_HANDLE+1 && strlen(thisHandle) >= 2){
		//pass the handle to the message
		strcat(tmp, thisHandle); 
		strcat(tmp, "$");
	}
	else
		DieWithError("client: invalid handle  ");
	//read and pass IP
	strcpy(thisIP, strtok(NULL, " "));
	strcat(tmp, thisIP);
	//read and pass ports
	char *token = (char *) malloc( MAX_MESSAGE);
	//read and pass peer port
	strcpy(token, strtok(NULL, " "));
	peerPort = atoi(token);
	strcat(tmp, "$");
	strcat(tmp, token);
	strcat(tmp, "$");
        //store the temporary string in the message buffer
	string = tmp;
	string[ (int) strlen( string) - 1] = '\0';
	//send message
	if( sendto( servSock, string, strlen( string ), 0, (struct sockaddr *) &servAddr, sizeof( servAddr ) ) != strlen(string) )
		DieWithError( "client: sendto() sent a different number of bytes than expected" );
	fromSize = sizeof( fromAddr );
	//receive message
	if( ( respStringLen = recvfrom( servSock, string, MAX_MESSAGE, 0, (struct sockaddr *) &fromAddr, &fromSize ) ) > MAX_MESSAGE )
		DieWithError("client: Error: received a packet from unknown source.\n");
	string[ respStringLen ] = '\0';
	if( servAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr )
		DieWithError("client: Error: received a packet from unknown source.\n");
	if(strcmp(string, "103") == 0) //make sure the registration succeeded
		printf("\nclient: successfully registered with tracker.\n");
	else //the registration failed, exit the program
		DieWithError("client: Error: registration failed.\n");
    }
    else
	DieWithError(" client: error reading handle, IP, or ports\n");

    //setup local peer2peer port to read incoming tweets
    if((peerSock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	DieWithError(" client: socket() failed");
    memset( &peerAddr, 0, sizeof(peerAddr));
    peerAddr.sin_family = AF_INET;
    peerAddr.sin_addr.s_addr = htonl( INADDR_ANY);
    peerAddr.sin_port = htons( peerPort);
    if( bind(peerSock, (struct sockaddr *) &peerAddr, sizeof(peerAddr)) < 0)
	DieWithError(" client: bind() failed");


    while(1) //read inputs from user
    {
	printMenu();
	//read input	
	do{ //in this loop, we will alternate between reading input and recv from the socket
		FD_ZERO(&rfds);
		FD_SET(0, &rfds);
    		tv.tv_sec = 1;
    		tv.tv_usec = 0;
		retval = select(1, &rfds, NULL, NULL, &tv);
		if(retval == -1) //some error occured
			DieWithError("client: Error: timer failure\n");
		else if (retval) //there is some input available
			if( ( nread = getline( &string, &stringLen, stdin ) ) != -1 ){ //successfully read
				string[ (int) strlen( string) - 1] = '\0';
				break;
			}
			else
				DieWithError(" client: error reading option selected" ); //failed to read
		else{//the timer is finished. we should check for any incoming tweets now
			FD_ZERO(&rfds);
			FD_SET(peerSock, &rfds);
			//read for half a second
			tv.tv_sec = 0;
			tv.tv_usec = 500000;
			retval = select(peerSock+1, &rfds, NULL, NULL, &tv); //recv for 0.5 seconds, this is for when there is an incoming tweet
			if(retval == -1)
				DieWithError("client: Error: timer failure\n");
			else if(retval){ //some input available
				if( ( respStringLen = recvfrom( peerSock, string, MAX_MESSAGE, 0, (struct sockaddr *) &fromAddr, &fromSize ) ) > MAX_MESSAGE ) //receive message
                                        DieWithError("client: Error: received a packet from unknown source. ");
				/*
 				*set up a circle
 				*/ 
			}
			//else
			//	printf("\nTried Reading\n");
		}
	}while(1);
	//the selection was stored in the message buffer
	if(strcmp(string, "1") == 0){ //query
		strcpy(string, "010"); //message being sent, this is just the flags
		string[ (int) strlen( string )] = '\0';
		//send message
		if(sendto(servSock, string, strlen(string), 0, (struct sockaddr *) &servAddr, sizeof( servAddr ) ) != strlen(string) ) //send message
			DieWithError( "client: Error: unable to send message");
		fromSize = sizeof( fromAddr );
		//recv message
		if( ( respStringLen = recvfrom( servSock, string, MAX_MESSAGE, 0, (struct sockaddr *) &fromAddr, &fromSize ) ) > MAX_MESSAGE ) //receive message
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
				if( ( respStringLen = recvfrom( servSock, string, MAX_MESSAGE, 0, (struct sockaddr *) &fromAddr, &fromSize ) ) > MAX_MESSAGE ) //receive message
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
			printf("\n\nUnknown Message Received. Canceling Command.\n\n");
	}
	else if(strcmp(string, "2") == 0){ //follow user
		printf("\nPlease enter the desired handles in the form of: \n@<Handle of new follower> @<Handle of user to follow>\n");
		//read handles
		if( ( nread = getline( &string, &stringLen, stdin ) ) != -1 ){
			string[ (int) strlen(string)] = '\0';
			char *tmp = (char*) malloc(MAX_HANDLE*2 + 5); //temporary buffer for handles
			strcpy(tmp, "020"); //flags for starting a follow request
			char *tkn = strtok(string, " "); //first handle
			strcat(tmp, tkn);
			strcat(tmp, "$");
			tkn = strtok(NULL, "\n"); //second handle
			strcat(tmp, tkn);
			strcat(tmp, "$");
			strcpy(string, tmp); //store the message into the message buffer
			string[ (int) strlen( string )] = '\0';
			free(tmp); //free the temporary buffer
			//send message
			if(sendto(servSock, string, strlen(string), 0, (struct sockaddr *) &servAddr, sizeof( servAddr ) ) != strlen(string) ) //send message
				DieWithError("client: Error: Socket Error. ");
			//receive message
			fromSize = sizeof( fromAddr );
			if( ( respStringLen = recvfrom( servSock, string, MAX_MESSAGE, 0, (struct sockaddr *) &fromAddr, &fromSize ) ) > MAX_MESSAGE ) //receive message
                                        DieWithError("client: Error: received a packet from unknown source. ");
			if(string[0] != '1' && string[1] != '2') //the flags were not valid
				printf("\n\nUnknown Message Received. Canceling Command.\n\n");
			else if(string[2] == '3') //the follow request was a success
				printf("client: successfully followed\n");
			else if(string[2] == '2') //the follow request was a failure
				printf("client: failed to follow\n");
			else
				DieWithError("client: Error: unable to read message. ");
		}
		else
			DieWithError("client: Error: invalid input. ");
	}
	else if(strcmp(string, "3") == 0){ //drop user
                printf("\nPlease enter the desired handles in the form of: \n@<Handle of follower> @<Handle of user to stop following>\n");
                //read handles
		if( ( nread = getline( &string, &stringLen, stdin ) ) != -1 ){
                        string[ (int) strlen(string)] = '\0';
                        char *tmp = (char*) malloc(MAX_HANDLE*2 +5); //temporary buffer for handles
                        strcpy(tmp, "030"); //flags for starting a drop request
                        char *tkn = strtok(string, " "); //first handle
                        strcat(tmp, tkn);
                        strcat(tmp, "$");
                        tkn = strtok(NULL, "\n"); //second handle
                        strcat(tmp, tkn);
                        strcat(tmp, "$");
                        strcpy(string, tmp); //store the message into the message buffer
                        string[ (int) strlen( string )] = '\0';
                        free(tmp);
                        //send message
                        if(sendto(servSock, string, strlen(string), 0, (struct sockaddr *) &servAddr, sizeof( servAddr ) ) != strlen(string) ) //send message
                                DieWithError("client: Error: Socket Error. ");
                        //receive message
                        fromSize = sizeof( fromAddr );
			if( ( respStringLen = recvfrom( servSock, string, MAX_MESSAGE, 0, (struct sockaddr *) &fromAddr, &fromSize ) ) > MAX_MESSAGE ) //receive message
                                        DieWithError("client: Error: received a packet from unknown source. ");
                        if(string[0] != '1' && string[1] != '3') //the flags are not valid
                                printf("\n\nUnknown Message Received. Canceling Command.");
                        else if(string[2] == '3') //the drop request was a success
                                printf("client: successfully Unfollowed\n");
                        else if(string[2] == '2') //the dorp request was a failure
                                printf("client: failed to unfollow\n");
                        else
                                DieWithError("client: Error: unable to read message. ");
                }
                else
                        DieWithError("client: Error: invalid input. ");
	}
	else if(strcmp(string, "4") == 0){ //this aspect is not yet implemented into the program
		printf("\nPlease enter your desired tweet. [Max 140 characters]\n");
		if((nread = getline( &string, &stringLen, stdin)) != -1){
			char *tweet = (char *) malloc(MAX_TWEET + 1);
			strcpy(tweet, string);
			tweet[ (int) strlen(tweet)] = '\0';
			strcpy(string, "040");
			strcat(string, thisHandle);
			strcat(string, "$");
			if(sendto(servSock, string, strlen(string), 0, (struct sockaddr *) &servAddr, sizeof(servAddr))!=strlen(string))
				DieWithError("client: Error: Socket Error. ");
			strcpy(string, "");
			fromSize = sizeof(fromAddr);
			if((respStringLen = recvfrom(servSock, string, MAX_MESSAGE, 0, (struct sockaddr *) &fromAddr, &fromSize))>MAX_MESSAGE)
				DieWithError("client: Error: received a packet from unknown source. ");
			printf("Received the following string from server: %s\n", string);
			printf("This is the tweet we wrote: %s\n", tweet);
			/*
 			*Fill in circle
 			*/
			//at this point, the tweet circle has successfully completed
			strcpy(string, "153");
			if(sendto(servSock, string, strlen(string), 0, (struct sockaddr *) &servAddr, sizeof( servAddr)) != strlen(string))
				DieWithError("client: Error: Socket Error. ");
			free(tweet);
		}
		else
			DieWithError("client: Error: invalid input. ");
	}
	else if(strcmp(string, "Q") == 0 || strcmp(string, "q") == 0){ //quit
		strcpy(string, "060"); //flags for starting a quitting request
		strcat(string, thisHandle); //the stored handle is sent
		strcat(string, "$");
		//send message
		if(sendto(servSock, string, strlen(string), 0, (struct sockaddr *) &servAddr, sizeof( servAddr ) ) != strlen(string) ) //send message
                               DieWithError("client: Error: Socket Error. ");
		//receive message
		fromSize = sizeof( fromAddr );
		if( ( respStringLen = recvfrom( servSock, string, MAX_MESSAGE, 0, (struct sockaddr *) &fromAddr, &fromSize ) ) > MAX_MESSAGE ) //receive message
                                DieWithError("client: Error: received a packet from unknown source. ");
		if(string[0] != '1' && string[1] != '6') //the flags are not valid
                                printf("\n\nUnknown Command Received. Force Quit.\n\n");
		else if(string[2] == '3') //success
			printf("client: successfully quite Tweeter\n");
		else //faiure
			printf("client: could not delete user from database\n");
		break; //exit while loop
	}
	else //none of the selected options are valid
		printf("\n\nPlease Choose Valid Choice\n\n");
    }
    close( servSock );
    close( peerSock );
    exit( 0 );
}


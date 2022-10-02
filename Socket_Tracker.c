// Implements the server side of an echo client-server application program.
// The client reads ITERATIONS strings from stdin, passes the string to the
// this server, which simply sends the string back to the client.
//
// Compile on general.asu.edu as:
//   g++ -o server UDPEchoServer.c
//
// Only on general3 and general4 have the ports >= 1024 been opened for
// application programs.
#include <stdio.h>      // for printf() and fprintf()
#include <sys/socket.h> // for socket() and bind()
#include <arpa/inet.h>  // for sockaddr_in and inet_ntoa()
#include <stdlib.h>     // for atoi() and exit()
#include <string.h>     // for memset()
#include <unistd.h>     // for close()
#include <stdbool.h>
#include "defns.h"

void DieWithError( const char *errorMessage ) // External error handling function
{
    perror( errorMessage );
    exit( 1 );
}

int main( int argc, char *argv[] )
{
    int sock;                        // Socket
    struct sockaddr_in servAddr; // Local address of server
    struct sockaddr_in clntAddr; // Client address
    unsigned int cliAddrLen;         // Length of incoming message
    char buffer[ MAX_MESSAGE ];      // Buffer for echo string
    unsigned short servPort;     // Server port
    int recvMsgSize;                 // Size of received message
    struct Node *userList = mkNewNode();
    int userCount = 0;
    servPort = atoi("46499");  // First arg: local port

    // Create socket for sending/receiving datagrams
    if( ( sock = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) < 0 )
        DieWithError( "server: socket() failed" );

    // Construct local address structure */
    memset( &servAddr, 0, sizeof( servAddr ) ); // Zero out structure
    servAddr.sin_family = AF_INET;                  // Internet address family
    servAddr.sin_addr.s_addr = htonl( INADDR_ANY ); // Any incoming interface
    servAddr.sin_port = htons( servPort );      // Local port

    // Bind to the local address
    if( bind( sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0 )
        DieWithError( "server: bind() failed" );

	printf( "server: Port server is listening to is: %d\n", servPort );

    for(;;) // Run forever
    {
        cliAddrLen = sizeof( clntAddr );

        // Block until receive message from a client
        if( ( recvMsgSize = recvfrom( sock, buffer, MAX_MESSAGE, 0, (struct sockaddr *) &clntAddr, &cliAddrLen )) < 0 )
            DieWithError( "server: recvfrom() failed" );

        buffer[ recvMsgSize ] = '\0';
	
	printf( "server: received string ``%s'' from client on IP address %s\n", buffer, inet_ntoa( clntAddr.sin_addr ) );
	

	bool passed;
	if(buffer[0] == '0' && buffer[2] == '0'){//request start
		char *follower;
		char *following;
		int result;
		switch(buffer[1]) {
			case '0': //register
				if(buffer[2] != '0'){
					printf("\nInvalid\n");
					passed = false;
				}
				else{
					struct User *newUser = mkNewUser();
					memmove(buffer, buffer+3, strlen(buffer));
					strcpy(newUser->handle, strtok(buffer, "$"));
					strcpy(newUser->ip,  strtok(NULL, "$"));
					strcpy(newUser->servPort, strtok(NULL, "$"));
					strcpy(newUser->inputPort, strtok(NULL, "$"));
					strcpy(newUser->outputPort, strtok(NULL, "\0"));
					if(insert(userList, newUser) > 0){
						passed = true;
						userCount++;
					}
					else
						passed = false;
				}
				strcpy(buffer, "10");
				if(passed)
					strcat(buffer, "3");
				else
					strcat(buffer, "2");
				if( sendto( sock, buffer, strlen( buffer ), 0, (struct sockaddr *) &clntAddr, sizeof( clntAddr ) ) != strlen( buffer ) )
					DieWithError( "server: sendto() sent a different number of bytes than expected" );
				printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\nActive Users\n");
				print(userList);
				printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
				break;
			case '1': //query
				if(buffer[2] != '0'){
					printf("\nInvalid\n");
					passed = false;
				}
				else{
					int maxHandleInMessage = (MAX_MESSAGE-4)/(MAX_HANDLE+1); //measure how many handles can fit into the buffer
					bool moreOutgoing;
					int count = 0;
					strcpy(buffer, "11");//flags for reply query
					if(userCount > maxHandleInMessage){//there are more users than can be fit into a single message
						moreOutgoing = true;
						strcat(buffer, "1");
					}
					else{ //all handles can fit into one message
						moreOutgoing = false;
						strcat(buffer, "3");
					}
					int tmplen = snprintf( NULL, 0, "%d", userCount);//length of number
					char* tmp = malloc(tmplen+1);//temporary buffer for the sake of storing the userCount in string form
					snprintf( tmp, tmplen+1, "%d", userCount); //convert the userCount to a string
					strcat(buffer, tmp); //concat the userCount to the message
					strcat(buffer, "$"); //insert special delimiter
					free(tmp); //free the temprorary buffer since it is no longer needed
					struct Node *nodeptr = userList; //traverse list
					for(int i = 0; i < userCount && i < maxHandleInMessage && nodeptr != NULL; i++){ //traverse list until we read all users, read enough users to fit into message, or we have traveresed the entire list
						strcat(buffer, nodeptr->thisUser->handle); //cat the handle to the message
						strcat(buffer, "$");
						nodeptr = nodeptr->nextNode; //next node
						count++;
					}
					printf("server: sending following string ``%s''\n", buffer);
					if(sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *) &clntAddr, sizeof(clntAddr)) != strlen(buffer))
						DieWithError("server: sentto() sent a different number of bytes than expected" );
					while(moreOutgoing){
						strcpy(buffer, "11");
						if(count  >= userCount){ //we have traversed the entire list
							moreOutgoing = false;
							strcat(buffer, "3");
						}
						else
							strcat(buffer, "1");
						for(int i = 0; count < userCount && i < maxHandleInMessage && nodeptr != NULL; i++){
							strcat(buffer, nodeptr->thisUser->handle);
							strcat(buffer, "$");
							nodeptr = nodeptr->nextNode;
							count++;
						}
						printf("server: sending following string ``%s''\n", buffer);
						if(sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *) &clntAddr, sizeof(clntAddr)) != strlen(buffer))
							DieWithError("server: setnto() sent a different number of bytes than expected" );
					}
				}
				break;
			case '2': //Follow
				memmove(buffer, buffer+3, strlen(buffer));
				follower = strtok(buffer, "$");
				following = strtok(NULL, "$");
				result = follow(userList, follower, following);
				strcpy(buffer, "12");
				if(result > 0)
					strcat(buffer, "3");
				else
					strcat(buffer, "2");
				printf("server: sending following string ``%s''\n", buffer);
                                printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\nActive Users\n");
                                print(userList);
                                printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
				if(sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *) &clntAddr, sizeof(clntAddr)) != strlen(buffer))
					DieWithError("server: setnto() sent a different number of bytes than expected" );
				break;
			case '3': //Drop
                                memmove(buffer, buffer+3, strlen(buffer));
                                follower = strtok(buffer, "$");
                                following = strtok(NULL, "$");
                                result = drop(userList, follower, following);
                                strcpy(buffer, "13");
                                if(result > 0)
                                        strcat(buffer, "3");
                                else
                                        strcat(buffer, "2");
				printf("server: sending following string ``%s''\n", buffer);
                                printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\nActive Users\n");
                                print(userList);
                                printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
                                if(sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *) &clntAddr, sizeof(clntAddr)) != strlen(buffer))
                                        DieWithError("server: setnto() sent a different number of bytes than expected" );
				break;
			case '4': //tweet
				break;
			case '6': //exit
                                memmove(buffer, buffer+3, strlen(buffer));
                                char *user = strtok(buffer, "$");
                                result = exitUser(userList, user);
                                strcpy(buffer, "16");
                                if(result > 0){
                                        strcat(buffer, "3");
					userCount--; 
				}
                                else
                                        strcat(buffer, "2");
                                printf("server: sending following string ``%s''\n", buffer);
                                printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\nActive Users\n");
                                print(userList);
                                printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
                                if(sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *) &clntAddr, sizeof(clntAddr)) != strlen(buffer))
                                        DieWithError("server: setnto() sent a different number of bytes than expected" );
				break;
		}
	}
	else{
		printf("No Ongoing requests\n");
		strcpy(buffer, "102");
		if(sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *) &clntAddr, sizeof(clntAddr)) != strlen(buffer))
			DieWithError("server: setnto() sent a different number of bytes than expected" );
	}
    }
    // NOT REACHED */
}

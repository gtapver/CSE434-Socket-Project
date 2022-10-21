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
    struct Node *userList = mkNewNode(); //pointer to the user list
    int userCount = 0; //count of active users
    servPort = atoi("46498");  //local port (set)

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
		//temporary variable used in future commands
		char *follower;
		char *following;
		int result;
		switch(buffer[1]) { //the first three characters in the message specify: <Request/Response> <Type of command> <Status>
			case '0': //register
				if(buffer[2] != '0'){ //the message should be a start command message, otherwise return an error message later
					passed = false;
				}
				else{
					struct User *newUser = mkNewUser();
					memmove(buffer, buffer+3, strlen(buffer)); //remove the flags
					strcpy(newUser->handle, strtok(buffer, "$")); //the first segment will be the handle
					newUser->handle[strlen(newUser->handle)] = '\0';
					strcpy(newUser->ip,  strtok(NULL, "$"));  //the second segment will be the ip
					strcpy(newUser->peerPort, strtok(NULL, "$")); //the fourth segment will be the peer2peer port
					if(insert(userList, newUser) > 0){ //if the insert message succeeded
						passed = true;
						userCount++; //increment user count
					}
					else
						passed = false;
				}
				strcpy(buffer, "10"); //flags for "response register"
				if(passed)
					strcat(buffer, "3"); //flag for success
				else
					strcat(buffer, "2"); //flag for failure
				//send back message
				printf("server: sending following string ``%s''", buffer);
				if( sendto( sock, buffer, strlen( buffer ), 0, (struct sockaddr *) &clntAddr, sizeof( clntAddr ) ) != strlen( buffer ) )
					DieWithError( "server: sendto() sent a different number of bytes than expected" );
				//print updated active user list
				printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\nActive Users\n");
				print(userList);
				printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
				break;
			case '1': //query
				if(buffer[2] != '0'){ //the message should be a start command message, otherwise return an error message later
					printf("\nInvalid\n");
					passed = false;
				}
				else{
					//Something to note: there might be more handles than can be fit into a single message
					//To resolve this, we will be sending multiple messages as needed, including whether more will be sent in the flags
					int maxHandleInMessage = (MAX_MESSAGE-7)/(MAX_HANDLE+1); //measure how many handles can fit into the buffer
					bool moreOutgoing; //boolean to see if more message need to be sent
					int count = 0; //count of handles sent
					strcpy(buffer, "11");//flags for reply query
					if(userCount > maxHandleInMessage){//there are more users than can be fit into a single message
						moreOutgoing = true; 
						strcat(buffer, "1"); //flag for "In-Progress", which the client will read as signifying that they should read more
					}
					else{ //all handles can fit into one message
						moreOutgoing = false;
						strcat(buffer, "3"); //flag for "Complete", which the client will read as signifying that they do not need to read more
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
					//send the message
					if(sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *) &clntAddr, sizeof(clntAddr)) != strlen(buffer))
						DieWithError("server: sentto() sent a different number of bytes than expected" );
					while(moreOutgoing){ //there are more handles that need to be sent
						strcpy(buffer, "11"); //"response query"
						if(count  >= userCount){ //we have traversed the entire list
							moreOutgoing = false;
							strcat(buffer, "3"); // "complete"
						}
						else
							strcat(buffer, "1"); // "in-progress"
						for(int i = 0; count < userCount && i < maxHandleInMessage && nodeptr != NULL; i++){ //until NULL, max handles, or all handles (see above)
							strcat(buffer, nodeptr->thisUser->handle);
							strcat(buffer, "$");
							nodeptr = nodeptr->nextNode;
							count++;
						}
						printf("server: sending following string ``%s''\n", buffer);
						//send message
						if(sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *) &clntAddr, sizeof(clntAddr)) != strlen(buffer))
							DieWithError("server: setnto() sent a different number of bytes than expected" );
					}
				}
				break;
			case '2': //Follow
				memmove(buffer, buffer+3, strlen(buffer)); //remove the flags
				follower = strtok(buffer, "$"); //follower is the first field
				following = strtok(NULL, "$"); //followee is the second field
				result = follow(userList, follower, following); //follower follows the followee
				strcpy(buffer, "12"); //flags for response and follow
				if(result > 0) //the follow() command succeeded
					strcat(buffer, "3"); //success
				else //the follow() command failed
					strcat(buffer, "2"); //failure
				printf("server: sending following string ``%s''\n", buffer);
                                printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\nActive Users\n");
                                print(userList);
                                printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
				//send the message
				if(sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *) &clntAddr, sizeof(clntAddr)) != strlen(buffer))
					DieWithError("server: setnto() sent a different number of bytes than expected" );
				break;
			case '3': //Drop
                                memmove(buffer, buffer+3, strlen(buffer)); //remove the flags
                                follower = strtok(buffer, "$"); //follower is the first field
                                following = strtok(NULL, "$"); //followee is the second field
                                result = drop(userList, follower, following); //follower drops the followee
                                strcpy(buffer, "13"); //flags for response and drop
                                if(result > 0) //the drop() command succeeded
                                        strcat(buffer, "3"); //success
                                else //the drop() command failed
                                        strcat(buffer, "2"); //failure
				printf("server: sending following string ``%s''\n", buffer);
                                printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\nActive Users\n");
                                print(userList);
                                printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
                                //send message
				if(sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *) &clntAddr, sizeof(clntAddr)) != strlen(buffer))
                                        DieWithError("server: sentto() sent a different number of bytes than expected" );
				break;
			case '4': //tweet
				memmove(buffer, buffer+3, strlen(buffer));
				following = strtok(buffer, "$"); //handle in this field
				struct User *handle = findUser(userList, following); //find the user struct that matches the handle provided
				memset(buffer, 0, sizeof(buffer));
				if(handle == NULL){ //the user was not found, do not send any tweet
					strcpy(buffer, "142");
					printf("server: sending following string ``%s''\n", buffer);
					if(sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *) &clntAddr, sizeof(clntAddr)) != strlen(buffer))
						DieWithError("server: sendto() sent a different number of bytes than expected");
				}
				else if(handle->followCount <= 0){ //the user was found, but has no followers, no tweet can be sent
					strcpy(buffer, "143");
					printf("server: sending following string ``%s''\n", buffer);
					if(sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *) &clntAddr, sizeof(clntAddr)) != strlen(buffer))
						DieWithError("server: sendto() sent a different number of bytes than expected");
				}
				else{ //user found and has at least one follower, tweet will be sent
					strcpy(buffer, "141"); //code for tweet in-progress
                                        int tmplen = snprintf( NULL, 0, "%d", handle->followCount);//length of number
                                        char* tmp = malloc(tmplen+1);//temporary buffer for the sake of storing the follower count in string form
                                        snprintf( tmp, tmplen+1, "%d", handle->followCount); //convert the follower count to a string
                                        strcat(buffer, tmp); //concat the follower count to the message 
					strcat(buffer, "$");
					free(tmp);
					struct Node *fList = handle->followerList;
					for(int i = 0; i < handle->followCount; i++){ //prepare a list of followers
						strcat(buffer, fList->thisUser->handle);
						strcat(buffer, "$");
						strcat(buffer, fList->thisUser->ip);
						strcat(buffer, "$");
						strcat(buffer, fList->thisUser->peerPort);
						strcat(buffer, "$");
						fList = fList->nextNode;
					}
					//send the list of followers
					printf("server: sending following string ``%s''\n", buffer);
					if(sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *) &clntAddr, sizeof(clntAddr)) != strlen(buffer))
						DieWithError("server: sendto() sent a different number of bytes than expected.");
					do{ //reject commands until such as time as the tweet is completed
						if( ( recvMsgSize = recvfrom( sock, buffer, MAX_MESSAGE, 0, (struct sockaddr *) &clntAddr, &cliAddrLen )) < 0 ) //recover message
            						DieWithError( "server: recvfrom() failed" );
        					buffer[ recvMsgSize ] = '\0';
        					printf( "server: received string ``%s'' from client on IP address %s\n", buffer, inet_ntoa( clntAddr.sin_addr ) );
						if(strcmp(buffer, "153") == 0)
							break;
						else{ //reject the string
							printf("server: expecting end tweet command: rejecting command\n");
							strcpy(buffer, "777");//return an unknown code. This is to ensure that any command is cancelled on collecting response.
							printf("server: sending following string ``%s''\n", buffer);
							if(sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *) &clntAddr, sizeof(clntAddr)) != strlen(buffer))
								DieWithError("server: sendto() sent a different number of bytes than expected.");
						}
					}while(1);
				}
				break;
			case '6': //exit
                                memmove(buffer, buffer+3, strlen(buffer)); //remove the flags
                                char *user = strtok(buffer, "$"); //temp buffer for user to be deleted
                                result = exitUser(userList, user); //user is deleted from the user list
                                strcpy(buffer, "16"); //flags for response and exit
                                if(result > 0){ //the exitUser() command succeeded
                                        strcat(buffer, "3"); //success
					userCount--;  //decrement the user count
				}
                                else //the exitUser() command failed
                                        strcat(buffer, "2"); //failure
                                printf("server: sending following string ``%s''\n", buffer);
                                printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\nActive Users\n");
                                print(userList);
                                printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
                                //send messsage
				if(sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *) &clntAddr, sizeof(clntAddr)) != strlen(buffer))
                                        DieWithError("server: sentto() sent a different number of bytes than expected" );
				break;
		}
	}
	else{ //the message received specified an ongoing transaction
		printf("No Ongoing requests\n");
		strcpy(buffer, "102"); //general failure message
		//send message
		if(sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *) &clntAddr, sizeof(clntAddr)) != strlen(buffer))
			DieWithError("server: setnto() sent a different number of bytes than expected" );
	}
    }
    // NOT REACHED */
}

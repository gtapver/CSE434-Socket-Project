

#define REQUEST 0
#define RESPONSE 1

#define REGISTER 0
#define QUERY 1
#define FOLLOW 2
#define DROP 3
#define TWEET 4
#define ENDTWEET 5
#define EXIT 6

#define START 0
#define INPROGRESS 1
#define FAILURE 2
#define SUCCESS 3 

#define MAX_IP 15
#define MAX_TWEET 140
#define MAX_PORTS 3
#define MAX_PORT_LENGTH 5
#define MAX_HANDLE 15
#define MAX_MESSAGE 1024 



int timeout(int fd, int sec, int usec){ //function to read from specified file descriptor until either some input is detected OR the timer times out
        fd_set rfds;
        struct timeval tv;

        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        tv.tv_sec = sec;
        tv.tv_usec = usec;
        return select(fd+1, &rfds, NULL, NULL, &tv);
}
char* intToStr(int integer){ //converts an integer to a string
	int len = snprintf( NULL, 0, "%d", integer);
	char *answer = malloc(len + 1);
	snprintf( answer, len + 1, "%d", integer);
	return answer;
}
struct User{ //structure to hold user information
	char handle[MAX_HANDLE+2]; //handle of user
	char ip[MAX_IP+1]; //ip address of user
	char peerPort[MAX_PORT_LENGTH+1]; //input port
	struct Node *followerList; //linked list of followers
	int followCount;
};
struct Node{ //nodes that will hold the user information and will be implemented for the purposes of a linked list
	struct Node *lastNode; //previous node in the linked list
	struct Node *nextNode; //next node in the linked list
	struct User *thisUser; //user struct that the node will hold
};
struct User *mkNewUser(){ //create an empty user struct with NULL follower list
	struct User *newUser = (struct User*) malloc(sizeof(struct User) ); //memory allocate the structure
	//Instantiate the elements
	strcpy(newUser->handle, ""); 
	strcpy(newUser->ip, "");
	strcpy(newUser->peerPort, "");
	newUser->followerList = NULL;
	newUser->followCount = 0;
	return newUser;
}
struct Node *mkNewNode(){ //instantiates a new node. This will be called to make a new user list or to add to an existing list. Note that an empty list will have one node with an empty user handle ("").
	struct Node *newNode = (struct Node*) malloc(sizeof(struct Node) ); ///memory allocate the structure
	//Instantiate the elements
	newNode->lastNode = NULL;
	newNode->nextNode = NULL;
	newNode->thisUser = mkNewUser(); //automatically create an empty user
	return newNode;
}
void killNode(struct Node *node){ //frees the specified node. The node will NOT delete the stored user, so call killUser() before killNode() if you wish to delete a User
	node->lastNode = NULL;
	node->nextNode = NULL;
	node->thisUser = NULL;
	free(node);
}
void killUser(struct User *user){
	struct Node *dlt = NULL;
	struct Node *nodePTR = user->followerList;
	while(nodePTR != NULL){
		dlt = nodePTR;
		nodePTR = nodePTR->nextNode;
		killNode(dlt);
	}
	free(user);
}
struct User* findUser(struct Node *userList, char* user){
        struct Node *nodeptr = userList;
        while(nodeptr != NULL){
                if(strcmp(user, nodeptr->thisUser->handle)==0)
                        return nodeptr->thisUser;
                else
                        nodeptr = nodeptr->nextNode;
        }
        return NULL;
}
int insert(struct Node *list, struct User *insertedUser){ //Insert a User object into a linked list. This can be used either for the user list or follower list
	struct Node *prevNode = NULL;
	struct Node *nodeptr = list;
	if(strcmp(nodeptr->thisUser->handle, "") == 0){ //the starting node is a fresh one, meaning that the list is empty
		list->thisUser = insertedUser; //simply replace the empty user struct with the parameter user, nothing else needs to be done
		return 1;
	}
	else{ //the list is not empty, and there is at least one user stored within
		int cmp; //variable to store how the handles compare (used to store users in alphabetical order)
		while(nodeptr != NULL){
			cmp = strcmp(insertedUser->handle, nodeptr->thisUser->handle); //compare the inserted user's handle and the current user
			if(cmp == 0) //there is an existing user with the same handle as the one inserted. Cancel the insert command and return 0
				return 0;
			else if(cmp > 0){ //the current user has a handle that comes alphabetically earlier
				//move on to the next node
				prevNode = nodeptr;
				nodeptr = nodeptr->nextNode;
			}
			else{ //the current user has a handle taht comes alphabetically later, meaning this is where we will store the inserted user
				struct Node *newNode = mkNewNode(); //create a new node
				killUser(newNode->thisUser); //delete the empty user stored within the node
				newNode->thisUser = insertedUser; //save the inserted user in the newly created node
				if(nodeptr->lastNode != NULL){ //the inserted node will not the inserted at the start of the list
					nodeptr->lastNode->nextNode = newNode; //connect the previous node to the new node
					newNode->lastNode = nodeptr->lastNode; //connect the new node to the previous node
					nodeptr->lastNode = newNode; //connect the next node to the new node
					newNode->nextNode = nodeptr; //connect the new node to the next node
				}
				else{ //at the start of the list. We will need to more the User struct itself in this situation, since otherwise the program will freak out
					newNode->thisUser = nodeptr->thisUser; //store the current root user in the list into the new node
					nodeptr->thisUser = insertedUser; //store the inserted user into the root node
					//the new node will become the second node, the root node will remain where it is. The inserted user is within the root, the old root will be within the second node
					newNode->nextNode = nodeptr->nextNode; //connect the second node to the third node
					newNode->lastNode = nodeptr; //connect the second node to the root node
					if(nodeptr->nextNode != NULL) //if the third node is not NULL, ie there exists some third user
						nodeptr->nextNode->lastNode = newNode; //connect the third node to the second node
					nodeptr->nextNode = newNode; //connect the root node to the second node
					nodeptr->lastNode = NULL; //make sure the root node does not have a previous node
				}
				return 1;
			}
		}//end while
		//we reach here only when the entire list has been traversed and no error or location has been found to store the user. In this case, store the user at the end of the list
		struct Node *newNode = mkNewNode(); //create a new node
		killUser(newNode->thisUser); //delete the empty user stored within the node
		newNode->thisUser = insertedUser; //store the inserted user into the new node
		prevNode->nextNode = newNode; //connect the tail node to the new node
		newNode->lastNode = prevNode; //connect the new node to the tail node
		return 2;
	}//end else
	//unreachable in all cases, return error just in case
	return -1;
}//end insert

void printFollowers(struct User* user){ //this command will be used in print(), which in turn will be called by the server to keep track of the user base
        if(user->followerList == NULL) //if the user has no followers
                printf("This User has no followers\n");
        else{ //the user has some followers, traverse list and print
                printf("%s is being followed by: ", user->handle);
                struct Node *nodeptr = user->followerList; //temporary node pointer
                printf("%s", nodeptr->thisUser->handle); //print only the handle
                nodeptr = nodeptr->nextNode; //move on to next node
                while(nodeptr != NULL){ //traverse list and print the handles
                        printf(", %s", nodeptr->thisUser->handle);
                        nodeptr = nodeptr->nextNode;
                }
                printf("\n");
        }
}

void print(struct Node *list){ //this command will be used in the tracker and will print all users, their information, and their followers
	struct Node *nodeptr = list;
	if(strcmp(nodeptr->thisUser->handle, "") != 0) //if the list is not empty, ie some users exist within the list
		while(nodeptr != NULL){
			//print the handle, ip, server facing port, input port, output port, and a list of followers, in that order
			printf("\nHandle: %s\n", nodeptr->thisUser->handle);
			printf("IP: %s\n", nodeptr->thisUser->ip);
			printf("Peer2Peer Port: %s\n", nodeptr->thisUser->peerPort);
			printFollowers(nodeptr->thisUser);
			nodeptr = nodeptr->nextNode;
		}
	else
		printf("\nThere are no active users\n");
	printf("\n");
}

int follow(struct Node *userList, char* follower, char* following){ //this command will be used by the tracker to make one existing user follow another existing user
	struct Node *flNode = NULL; //pointer to new follower's node
	struct Node *flgNode = NULL; //pointer to followee's node
	struct Node *nodeptr = userList; //pointer used to traverse list
	
	if(strcmp(follower, following) == 0)//same user is input as follower and following
		return 0;
	while(nodeptr != NULL){ //traverse list
		if(strcmp(follower, nodeptr->thisUser->handle) == 0) //we found the follower
			flNode = nodeptr;
		else if(strcmp(following, nodeptr->thisUser->handle) == 0) //we found the user to follow
			flgNode = nodeptr;
		if(flNode != NULL && flgNode != NULL) //both the follower and followee have been found, so break loop
			break;
		nodeptr = nodeptr->nextNode; //next node
	}
	if(flNode == NULL || flgNode == NULL) //either user does not exist
		return -1;
	else{ //both users exist
		if(flgNode->thisUser->followerList == NULL) //this user does not yet have a follower list set-up
			flgNode->thisUser->followerList = mkNewNode(); //make an empty list for the user
		int result = insert(flgNode->thisUser->followerList, flNode->thisUser); //do an insert on the follower list
		if(result > 0)
			flgNode->thisUser->followCount++;
		return result;
	}
}

int drop(struct Node *userList, char* follower, char* following){ //remove a user from a follower list, but does not delete the user. Note the the userList specifies to the user list, not the followee's follower list
	struct Node *flgNode = NULL; //pointer to followee's node in the user list
	struct Node *nodeptr = userList; //pointer to traverse user list
	if(strcmp(follower, following) == 0) //check to make sure the follower and following are different people
		return 0;
	while(nodeptr != NULL){ //traverse list
		if(strcmp(following, nodeptr->thisUser->handle) == 0){ //we have found the user, store in pointer and break loop
			flgNode = nodeptr;
			break;
		}
		else
			nodeptr = nodeptr->nextNode; //next node
	}
	if(flgNode == NULL) //User to stop following not found
		return -1;
	else{ //User to stop following found
		nodeptr = flgNode->thisUser->followerList; //switch the pointer to now traverse the follower list of specified user
		while(nodeptr != NULL){ //traverse list
			if(strcmp(nodeptr->thisUser->handle, follower) == 0){ //follower found
				if(nodeptr->lastNode != NULL && nodeptr->nextNode != NULL){ //we found the user somewhere within the list
					nodeptr->lastNode->nextNode = nodeptr->nextNode; //connect the last follower to the next follower
					nodeptr->nextNode->lastNode = nodeptr->lastNode; //connect the next follower to the last follower
					killNode(nodeptr); //kill the follower's node
				}
				else if(nodeptr->lastNode == NULL){ //the user is at the start of the follower list
					if(nodeptr->nextNode == NULL){ //the user is the only node in the list
						flgNode->thisUser->followerList = NULL; //null out the follower list [base form]
						killNode(nodeptr); //kill the follower's node
					}
					else{ //there is some second user in the list
						struct Node *tmp = nodeptr->nextNode; //save the second node, we will delete that one
						nodeptr->thisUser = nodeptr->nextNode->thisUser;//swap the user in the second node to the first
						nodeptr->nextNode = nodeptr->nextNode->nextNode;//connect the first node to the third
						if(nodeptr->nextNode != NULL) //if there is some third node that is not null
							nodeptr->nextNode->lastNode = nodeptr;//connect the third node to the first
						killNode(tmp); //delete the superfluous node	
					}
				}
				else if(nodeptr->nextNode == NULL){ //the user is at the end of the follower list and there is some node before
					nodeptr->lastNode->nextNode = NULL; //make the previous follower the tail follower
					killNode(nodeptr); // kill the follower's node
				}
				flgNode->thisUser->followCount--;
				return 1;
			}
			else
				nodeptr = nodeptr->nextNode;
		}
		return -1; //follower not found
	}
}

int exitUser(struct Node *userList, char* user){ //used by tracker to remove user from database. It will first remove the user from all follower lists, before deleting the user itself
	struct Node *nodeptr = userList; //pointer used to traverse list
	struct Node *usrptr = NULL; //pointer to the user's node in the user list
	while(nodeptr != NULL){ //traverse list
		if(strcmp(user, nodeptr->thisUser->handle) == 0) //the user was found 
			usrptr = nodeptr; //store the user's node in the pointer
		else //the pointer is not pointing to the right user, but we can still drop the quitting user from this person's follower list
			drop(userList, user, nodeptr->thisUser->handle); //drop the quitting user from this person's follower list
		nodeptr = nodeptr->nextNode;
	}
	if(usrptr == NULL)//the user was never found
		return -1;
	else{//at this point, all instances of the user to be deleted has been deleted from each follower list
		if(usrptr->lastNode != NULL && usrptr->nextNode != NULL){ //the user is stored somewhere in the middle of the list
			usrptr->lastNode->nextNode = usrptr->nextNode; //connect the prev. node to the next node
			usrptr->nextNode->lastNode = usrptr->lastNode; //connect the next node to the prev. node
			killUser(usrptr->thisUser); //delete the user
			killNode(usrptr); //delete the node
		}
		else if(usrptr->lastNode == NULL){ //the user is stored at the start of the list
			if(usrptr->nextNode == NULL){ //the user is the only user in the list
				killUser(usrptr->thisUser); //delete the user in the node
				usrptr->thisUser = mkNewUser(); //store a fresh, empty user in the list [base state]
			}
			else{ //there is some second node
				struct Node *tmp = usrptr->nextNode; //store the second node for later
				struct User *tmpU = usrptr->thisUser; //store the user to be deleted for later
				usrptr->thisUser = tmp->thisUser; //store the second user in the first node
				usrptr->nextNode = tmp->nextNode; //connect the first node to the third
				if(tmp->nextNode != NULL) //if there is some third node
					tmp->nextNode->lastNode = usrptr; //connect the third node to the first
				killUser(tmpU);//kill the user to be deleted
				killNode(tmp);//kill the extra node
			}
		}
		else if(usrptr->nextNode == NULL){ //the node at the end but there is some node before
			usrptr->lastNode->nextNode = NULL;//make the prev. node the new end of list
			killUser(usrptr->thisUser); //delete the user
			killNode(usrptr); //delete the node
		}
		else //some unknown scenario happened
			return -1;
		//if we reach this point, the user and its node has been deleted
		return 1;
	}
}

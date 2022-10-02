

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
#define MAX_MESSAGE 140+15+4



struct User{
	char handle[MAX_HANDLE];
	char ip[MAX_IP];
	char servPort[MAX_PORT_LENGTH];
	char inputPort[MAX_PORT_LENGTH];
	char outputPort[MAX_PORT_LENGTH];
	struct Node *followerList;
};
struct Node{
	struct Node *lastNode;
	struct Node *nextNode;
	struct User *thisUser;
};
struct User *mkNewUser(){
	struct User *newUser = (struct User*) malloc(sizeof(struct User) );
	strcpy(newUser->handle, "");
	strcpy(newUser->ip, "");
	strcpy(newUser->servPort, "");
	strcpy(newUser->inputPort, "");
	strcpy(newUser->outputPort, "");
	newUser->followerList = NULL;
	return newUser;
}
void killUser(struct User *user){
	user->followerList = NULL;
	free(user);
}
struct Node *mkNewNode(){
	struct Node *newNode = (struct Node*) malloc(sizeof(struct Node) );
	newNode->lastNode = NULL;
	newNode->nextNode = NULL;
	newNode->thisUser = mkNewUser();
	return newNode;
}
void killNode(struct Node *node){
	node->lastNode = NULL;
	node->nextNode = NULL;
	node->thisUser = NULL;
	free(node);
}

int insert(struct Node *list, struct User *insertedUser){
	struct Node *prevNode = NULL;
	struct Node *nodeptr = list;
	if(strcmp(nodeptr->thisUser->handle, "") == 0){ //empty node
		list->thisUser = insertedUser;
		return 1;
	}
	else{
		int cmp;
		while(nodeptr != NULL){
			cmp = strcmp(insertedUser->handle, nodeptr->thisUser->handle);
			if(cmp == 0)
				return 0;
			else if(cmp > 0){
				prevNode = nodeptr;
				nodeptr = nodeptr->nextNode;
			}
			else{
				struct Node *newNode = mkNewNode();
				killUser(newNode->thisUser);
				newNode->thisUser = insertedUser;
				if(nodeptr->lastNode != NULL){ //not at the start of the list
					nodeptr->lastNode->nextNode = newNode;
					newNode->lastNode = nodeptr->lastNode;
					nodeptr->lastNode = newNode;
					newNode->nextNode = nodeptr;
				}
				else{ //at the start of the list
					newNode->thisUser = nodeptr->thisUser; //replace the second user with the old first
					nodeptr->thisUser = insertedUser;
					newNode->nextNode = nodeptr->nextNode;
					newNode->lastNode = nodeptr;
					nodeptr->nextNode = newNode;
					nodeptr->lastNode = NULL;
				}
				return 1;
			}
		}//end while
		struct Node *newNode = mkNewNode();
		killUser(newNode->thisUser);
		newNode->thisUser = insertedUser;
		prevNode->nextNode = newNode;
		newNode->lastNode = prevNode;
		return 2;
	}//end else
	return -1;
}//end insert

void printFollowers(struct User* user){
        if(user->followerList == NULL)
                printf("This User has no followers\n");
        else{
                printf("%s is being followed by: ", user->handle);
                struct Node *nodeptr = user->followerList;
                printf("%s", nodeptr->thisUser->handle);
                nodeptr = nodeptr->nextNode;
                while(nodeptr != NULL){
                        printf(", %s", nodeptr->thisUser->handle);
                        nodeptr = nodeptr->nextNode;
                }
                printf("\n");
        }
}

void print(struct Node *list){
	struct Node *nodeptr = list;
	int i = 0;
	if(strcmp(nodeptr->thisUser->handle, "") != 0)
		while(nodeptr != NULL && i < 10){
			printf("\nHandle: %s\n", nodeptr->thisUser->handle);
			printf("IP: %s\n", nodeptr->thisUser->ip);
			printf("Server Facing Port: %s\n", nodeptr->thisUser->servPort);
			printf("Input Port: %s\n", nodeptr->thisUser->inputPort);
			printf("Output Port: %s\n", nodeptr->thisUser->outputPort);
			printFollowers(nodeptr->thisUser);
			nodeptr = nodeptr->nextNode;
			i++;
		}
	else
		printf("\nThere are no active users\n");
	printf("\n");
}

int follow(struct Node *userList, char* follower, char* following){
	struct Node *flNode = NULL;
	struct Node *flgNode = NULL;
	struct Node *nodeptr = userList;
	
	if(strcmp(follower, following) == 0)//same user is input as follower and following
		return 0;
	while(nodeptr != NULL){ //traverse list
		if(strcmp(follower, nodeptr->thisUser->handle) == 0) //we found the follower
			flNode = nodeptr;
		else if(strcmp(following, nodeptr->thisUser->handle) == 0) //we found the user to follow
			flgNode = nodeptr;
		nodeptr = nodeptr->nextNode; //next node
	}
	if(flNode == NULL || flgNode == NULL) //either user does not exist
		return -1;
	else{
		if(flgNode->thisUser->followerList == NULL) //this user does not yet have a follower list set-up
			flgNode->thisUser->followerList = mkNewNode(); //make an empty list for the user
		return insert(flgNode->thisUser->followerList, flNode->thisUser); //do an insert on the follower list
	}
}

int drop(struct Node *userList, char* follower, char* following){ //remove a user from a list, do not delete user itself
	struct Node *flgNode = NULL;
	struct Node *nodeptr = userList;
	if(strcmp(follower, following) == 0) //check to make sure the follower and following are different people
		return 0;
	while(nodeptr != NULL){ //traverse list
		if(strcmp(following, nodeptr->thisUser->handle) == 0){ //we have found the user to stop following, no longer need to traverse list
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
			if(strcmp(nodeptr->thisUser->handle, follower) == 0){
				if(nodeptr->lastNode != NULL && nodeptr->nextNode != NULL){ //we found the user somewhere within the list
					nodeptr->lastNode->nextNode = nodeptr->nextNode;
					nodeptr->nextNode->lastNode = nodeptr->lastNode;
					killNode(nodeptr);
				}
				else if(nodeptr->lastNode == NULL){ //the user is at the start of the follower list
					if(nodeptr->nextNode == NULL){ //the user is the only node in the list
						flgNode->thisUser->followerList = NULL; //null out the follower list [base form]
						killNode(nodeptr); //kill the user's node
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
					nodeptr->lastNode->nextNode = NULL;
					killNode(nodeptr);
				}
				return 1;
			}
			else
				nodeptr = nodeptr->nextNode;
		}
		return -1; //follower not found
	}
}

int exitUser(struct Node *userList, char* user){
	struct Node *nodeptr = userList;
	struct Node *usrptr = NULL;
	while(nodeptr != NULL){
		if(strcmp(user, nodeptr->thisUser->handle) == 0)
			usrptr = nodeptr;
		else
			drop(userList, user, nodeptr->thisUser->handle);
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
		return 1;
	}
}

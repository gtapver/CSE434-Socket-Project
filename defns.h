

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
	struct User thisUser;
};
int insert(struct Node list, struct User insertedUser){
	struct Node *nodeptr = &list;
	if(strcmp(nodeptr->thisUser.handle, "") == 0){
		list.thisUser = insertedUser;
		return 1;
	}
	else{
		struct User userptr;
		struct Node newNode;
		while(strcmp(nodeptr->thisUser.handle, "") == 0){
			userptr = nodeptr->thisUser;
			int cmp = strcmp(insertedUser.handle, userptr.handle);
			if(cmp == 0)
				return 0;
			else if(cmp > 0)
				nodeptr = nodeptr->nextNode;
			else{
				if(strcmp(nodeptr->lastNode->thisUser.handle, "") == 0)
					nodeptr->lastNode->nextNode = &newNode;
				newNode.lastNode = nodeptr->lastNode;
				nodeptr->lastNode = &newNode;
				newNode.nextNode = nodeptr;
			}//end else
		}//end while
		return -1;
	}//end else
}//end insert

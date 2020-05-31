#include <random>

#include <random>

/**********************************
 * FILE NAME: MP1Node.cpp
 *
 * DESCRIPTION: Membership protocol run by this Node.
 * 				Definition of MP1Node class functions.
 **********************************/

#include "MP1Node.h"
#include "Member.h"

/*
 * Note: You can change/add any functions in MP1Node.{h,cpp}
 */

/**
 * Overloaded Constructor of the MP1Node class
 * You can add new members to the class if you think it
 * is necessary for your logic to work
 */
MP1Node::MP1Node(Member *member, Params *params, EmulNet *emul, Log *log, Address *address) {
	for( int i = 0; i < 6; i++ ) {
		NULLADDR[i] = 0;
	}
	this->memberNode = member;
	this->emulNet = emul;
	this->log = log;
	this->par = params;
	this->memberNode->addr = *address;
}

/**
 * Destructor of the MP1Node class
 */
MP1Node::~MP1Node() {}

/**
 * FUNCTION NAME: recvLoop
 *
 * DESCRIPTION: This function receives message from the network and pushes into the queue
 * 				This function is called by a node to receive messages currently waiting for it
 */
int MP1Node::recvLoop() {
    if ( memberNode->bFailed ) {
    	return false;
    }
    else {
    	return emulNet->ENrecv(&(memberNode->addr), enqueueWrapper, NULL, 1, &(memberNode->mp1q));
    }
}

/**
 * FUNCTION NAME: enqueueWrapper
 *
 * DESCRIPTION: Enqueue the message from Emulnet into the queue
 */
int MP1Node::enqueueWrapper(void *env, char *buff, int size) {
	Queue q;
	return q.enqueue((queue<q_elt> *)env, (void *)buff, size);
}

/**
 * FUNCTION NAME: nodeStart
 *
 * DESCRIPTION: This function bootstraps the node
 * 				All initializations routines for a member.
 * 				Called by the application layer.
 */
void MP1Node::nodeStart(char *servaddrstr, short servport) {
    Address joinaddr;
    joinaddr = getJoinAddress();

    // Self booting routines
    if( initThisNode(&joinaddr) == -1 ) {
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "init_thisnode failed. Exit.");
#endif
        exit(1);
    }

    if( !introduceSelfToGroup(&joinaddr) ) {
        finishUpThisNode();
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "Unable to join self to group. Exiting.");
#endif
        exit(1);
    }

    return;
}

/**
 * FUNCTION NAME: initThisNode
 *
 * DESCRIPTION: Find out who I am and start up
 */
int MP1Node::initThisNode(Address *joinaddr) {
	/*
	 * This function is partially implemented and may require changes
	 */
	int id = *(int*)(&memberNode->addr.addr);
	int port = *(short*)(&memberNode->addr.addr[4]);

	memberNode->bFailed = false;
	memberNode->inited = true;
	memberNode->inGroup = false;
    // node is up!
	memberNode->nnb = 0;
	memberNode->heartbeat = 0;
	memberNode->pingCounter = TFAIL;
	memberNode->timeOutCounter = -1;
    initMemberListTable(memberNode);

    return 0;
}

/**
 * FUNCTION NAME: introduceSelfToGroup
 *
 * DESCRIPTION: Join the distributed system
 */
int MP1Node::introduceSelfToGroup(Address *joinaddr) {
	MessageHdr *msg;
#ifdef DEBUGLOG
    static char s[1024];
#endif

    if ( 0 == memcmp((char *)&(memberNode->addr.addr), (char *)&(joinaddr->addr), sizeof(memberNode->addr.addr))) {
        // I am the group booter (first process to join the group). Boot up the group
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "Starting up group...");
#endif
        memberNode->inGroup = true;
    }
    else {
        size_t msgsize = sizeof(MessageHdr) + sizeof(joinaddr->addr) + sizeof(long);
        msg = (MessageHdr *) malloc(msgsize * sizeof(char));

        // create JOINREQ message: format of data is {struct Address myaddr}
        msg->msgType = JOINREQ;
        memcpy((char *)(msg+1), &memberNode->addr.addr, sizeof(memberNode->addr.addr));
        memcpy((char *)(msg+1) + sizeof(memberNode->addr.addr), &memberNode->heartbeat, sizeof(long));

#ifdef DEBUGLOG
        sprintf(s, "Trying to join...");
        log->LOG(&memberNode->addr, s);
#endif

        // send JOINREQ message to introducer member
        emulNet->ENsend(&memberNode->addr, joinaddr, (char *)msg, msgsize);

        free(msg);
    }

    return 1;

}

/**
 * FUNCTION NAME: finishUpThisNode
 *
 * DESCRIPTION: Wind up this node and clean up state
 */
int MP1Node::finishUpThisNode(){
    memberNode->memberList.clear();
    memberNode->heartbeat = 0;
    memberNode->inGroup = false;
    queue<q_elt> empty;
    swap(empty, memberNode->mp1q);
    memberNode->nnb = 0;
   /*
    * Your code goes here
    */
}

/**
 * FUNCTION NAME: nodeLoop
 *
 * DESCRIPTION: Executed periodically at each member
 * 				Check your messages in queue and perform membership protocol duties
 */
void MP1Node::nodeLoop() {
    if (memberNode->bFailed) {
    	return;
    }

    // Check my messages
    checkMessages();

    // Wait until you're in the group...
    if( !memberNode->inGroup ) {
    	return;
    }

    // ...then jump in and share your responsibilites!
    nodeLoopOps();

    return;
}

/**
 * FUNCTION NAME: checkMessages
 *
 * DESCRIPTION: Check messages in the queue and call the respective message handler
 */
void MP1Node::checkMessages() {
    void *ptr;
    int size;

    // Pop waiting messages from memberNode's mp1q
    while ( !memberNode->mp1q.empty() ) {
    	ptr = memberNode->mp1q.front().elt;
    	size = memberNode->mp1q.front().size;
    	memberNode->mp1q.pop();
    	recvCallBack((void *)memberNode, (char *)ptr, size);
    }
    return;
}

/**
 * FUNCTION NAME: recvCallBack
 *
 * DESCRIPTION: Message handler for different message types
 */
bool MP1Node::recvCallBack(void *env, char *data, int size ) {

    auto *localInfo = (Member*)env;

    auto *msgType =  (MessageHdr*) data;

    auto *msg = (MemberMsg*)(msgType+1);

    if (localInfo->memberList.empty()) {
        localInfo->memberList.emplace_back(
                localInfo->addr.getId(),
                localInfo->addr.getPort(),
                localInfo->heartbeat,
                localInfo->heartbeat,
                ACTIVE);

        log->logNodeAdd(&localInfo->addr,&localInfo->addr);
    }

    switch (msgType->msgType) {
        case JOINREQ:{
            localInfo->memberList.emplace_back(MemberListEntry(msg->address.getId(),
                    msg->address.getPort(),
                    msg->memberHeartBeat,
                    localInfo->heartbeat,
                    ACTIVE));

            log->logNodeAdd(&localInfo->addr, &msg->address);

            free(msgType);

            size_t msgSize = (localInfo->memberList.size()-1) * sizeof(MemberMsg) + sizeof(MessageHdr);
            msgType = (MessageHdr*)malloc(msgSize * sizeof(char));

            size_t actual_size = sizeof(MessageHdr);

            msgType->msgType = JOINREP;
            msg = (MemberMsg*)(msgType+1);

            for (auto first = localInfo->memberList.begin(); first != localInfo->memberList.end()-1; first++) {

                if ((*first).state == ACTIVE) {
                    msg->address = (*first).address;
                    msg->memberHeartBeat = (*first).heartbeat;
                    msg++;
                    actual_size += sizeof(MemberMsg);
                }
            }

            emulNet->ENsend(&localInfo->addr, &localInfo->memberList.back().address, (char*)msgType, actual_size);

            free(msgType);
            break;
        }

        case JOINREP: {

            for (size_t sumSize = sizeof(MessageHdr); sumSize < size; sumSize+= sizeof(MemberMsg)) {

                if (find(memberNode->memberList.begin(), memberNode->memberList.end(), msg->address) == memberNode->memberList.end()) {

                    localInfo->memberList.emplace_back(msg->address.getId(),
                                                       msg->address.getPort(),
                                                       msg->memberHeartBeat,
                                                       localInfo->heartbeat,
                                                       ACTIVE);

                    log->logNodeAdd(&localInfo->addr, &msg->address);
                }
                msg++;
            }

            localInfo->inGroup = true;
            free(msgType);
            break;
        }

        case NORMAL: {

            for (size_t sumSize = sizeof(MessageHdr); sumSize < size; sumSize+= sizeof(MemberMsg)) {

                auto iter = find(localInfo->memberList.begin(), localInfo->memberList.end(), msg->address);

                if (iter == localInfo->memberList.end()) {

                    localInfo->memberList.emplace_back(
                            msg->address.getId(),
                            msg->address.getPort(),
                            msg->memberHeartBeat,
                            localInfo->heartbeat,
                            ACTIVE
                    );

                    log->logNodeAdd(&localInfo->addr, &msg->address);
                }
                else if ((*iter).address == localInfo->addr) {

                }
                else if ((*iter).heartbeat < msg->memberHeartBeat) {
                    (*iter).heartbeat = msg->memberHeartBeat;
                    (*iter).timestamp = localInfo->heartbeat;
                    (*iter).state = ACTIVE;
                }

                msg++;
            }

            free(msgType);
            break;
        }

        case DUMMYLASTMSGTYPE: {
            break;
        }





    }

    return true;



}

/**
 * FUNCTION NAME: nodeLoopOps
 *
 * DESCRIPTION: Check if any node hasn't responded within a timeout period and then delete
 * 				the nodes
 * 				Propagate your membership list
 */
void MP1Node::nodeLoopOps() {

    const int N = 2, K = 2; //N个消息，发送给K个对象

    const int timeOut = 5;

    const int clear = timeOut + 45;

	int index, n = 0, k = 0, size = memberNode->memberList.end() - memberNode->memberList.begin();

	//注意这个地方
	srand(par->getcurrtime() + time(nullptr));

	memberNode->heartbeat++;

    auto *msgType = (MessageHdr*) malloc(sizeof(MessageHdr) + sizeof(MemberMsg) * N);

    auto *msg = (MemberMsg*)(msgType+1);

    msgType->msgType = NORMAL;

    //选取任意n个消息，发送给K个对象
    for (auto first = memberNode->memberList.begin(); first != memberNode->memberList.end();) {


        if (first != memberNode->memberList.begin()) {

            if (size - (n + 1) > 0 && n < N) {

                index = rand() % (size - (n + 1)) + n + 1; //随机生成一个位置，然后二者交换

                Swap(*first, *(memberNode->memberList.begin() + index));//这个可以保证在一个包里的三个消息是不重复的，复杂度也比较低

            }

            if ((*first).timestamp + timeOut < memberNode->heartbeat && (*first).state == ACTIVE) {

                (*first).state = TENTATIVE;

            } else if ((*first).state == TENTATIVE && (*first).timestamp + clear < memberNode->heartbeat) {
                log->logNodeRemove(&memberNode->addr, &(*first).address);
                Swap(*first, *(memberNode->memberList.end()-1));
                memberNode->memberList.pop_back();//复杂度由n^2降为n
                size--;
                continue;
            }
        } else {
            //self
            (*first).heartbeat = memberNode->heartbeat;
            (*first).timestamp = memberNode->heartbeat;
        }

        if ((*first).state == ACTIVE && n < N) {
            msg->address = (*first).address;
            msg->memberHeartBeat = (*first).heartbeat;
            msg++;
            n++;
        }

        first++;

    }

    index = (size - 1) > 0 ? rand()%(size-1)+1 : 1;

    int flag = 0;
    //shuffle(memberNode->memberList.begin()+1, memberNode->memberList.end(), std::mt19937(std::random_device()()));
    //随机给一个发送消息的起始点
    for (auto first = memberNode->memberList.begin()+index; first != memberNode->memberList.end() && k < K;) {

        if ((*first).state == ACTIVE) {
            emulNet->ENsend(&memberNode->addr,
                            &(*first).address,
                            (char *) msgType,
                            sizeof(MessageHdr) + sizeof(MemberMsg) * n);
            k++;
        }

        first++;

        if (first == memberNode->memberList.end()) {
            if (flag == 0) {
                first = memberNode->memberList.begin() + 1;
                flag = 1;
            } else
                break;
        }

    }

    free(msgType);


    return;
}

/**
 * FUNCTION NAME: isNullAddress
 *
 * DESCRIPTION: Function checks if the address is NULL
 */
int MP1Node::isNullAddress(Address *addr) {
	return (memcmp(addr->addr, NULLADDR, 6) == 0 ? 1 : 0);
}

/**
 * FUNCTION NAME: getJoinAddress
 *
 * DESCRIPTION: Returns the Address of the coordinator
 */
Address MP1Node::getJoinAddress() {
    Address joinaddr;

    memset(&joinaddr, 0, sizeof(Address));
    *(int *)(&joinaddr.addr) = 1;
    *(short *)(&joinaddr.addr[4]) = 0;

    return joinaddr;
}

/**
 * FUNCTION NAME: initMemberListTable
 *
 * DESCRIPTION: Initialize the membership list
 */
void MP1Node::initMemberListTable(Member *memberNode) {
	memberNode->memberList.clear();
}

/**
 * FUNCTION NAME: printAddress
 *
 * DESCRIPTION: Print the Address
 */
void MP1Node::printAddress(Address *addr)
{
    printf("%d.%d.%d.%d:%d \n",  addr->addr[0],addr->addr[1],addr->addr[2],
                                                       addr->addr[3], *(short*)&addr->addr[4]) ;    
}

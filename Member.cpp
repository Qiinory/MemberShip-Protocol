/**********************************
 * FILE NAME: Member.cpp
 *
 * DESCRIPTION: Definition of all Member related class
 **********************************/

#include "Member.h"

/**
 * Constructor
 */
q_elt::q_elt(void *elt, int size): elt(elt), size(size) {}

/**
 * Copy constructor
 */
Address::Address(const Address &anotherAddress) {
	// strcpy(addr, anotherAddress.addr);
	memcpy(&addr, &anotherAddress.addr, sizeof(addr));
}

/**
 * Assignment operator overloading
 */
Address& Address::operator =(const Address& anotherAddress) {
	// strcpy(addr, anotherAddress.addr);
	memcpy(&addr, &anotherAddress.addr, sizeof(addr));
	return *this;
}

/**
 * Compare two Address objects
 * Return true/non-zero if they have the same ip address and port number 
 * Return false/zero if they are different 
 */
bool Address::operator ==(const Address& anotherAddress) {
	return !memcmp(this->addr, anotherAddress.addr, sizeof(this->addr));
}

/**
 * Constructor
 */
MemberListEntry::MemberListEntry(int id, short port, long heartbeat, long timestamp, MemberListEntryState state): id(id), port(port), heartbeat(heartbeat), timestamp(timestamp), state(state) {
    address.setId(id);
    address.setPort(port);
}

/**
 * Constuctor
 */
MemberListEntry::MemberListEntry(int id, short port): id(id), port(port) {address.setId(id); address.setPort(port); this->state = BAD;}

/**
 * Copy constructor
 */
MemberListEntry::MemberListEntry(const MemberListEntry &anotherMLE) {
	this->heartbeat = anotherMLE.heartbeat;
	this->id = anotherMLE.id;
	this->port = anotherMLE.port;
	this->timestamp = anotherMLE.timestamp;
	this->address = anotherMLE.address;
	this->state = anotherMLE.state;

}

/**
 * Assignment operator overloading
 */
MemberListEntry& MemberListEntry::operator =(const MemberListEntry &anotherMLE) {
	MemberListEntry temp(anotherMLE);
	swap(heartbeat, temp.heartbeat);
	swap(id, temp.id);
	swap(port, temp.port);
	swap(timestamp, temp.timestamp);
	swapAddr(address, temp.address);
	swap(state, temp.state);
	return *this;
}

void MemberListEntry::swapAddr(Address& local, Address& other) {
    Address tmpAddr = other.getAddress();

    local = other;

    other = tmpAddr;
}

/**
 * FUNCTION NAME: getid
 *
 * DESCRIPTION: getter
 */
int MemberListEntry::getid() {
	return id;
}

/**
 * FUNCTION NAME: getport
 *
 * DESCRIPTION: getter
 */
short MemberListEntry::getport() {
	return port;
}

/**
 * FUNCTION NAME: getheartbeat
 *
 * DESCRIPTION: getter
 */
long MemberListEntry::getheartbeat() {
	return heartbeat;
}

/**
 * FUNCTION NAME: gettimestamp
 *
 * DESCRIPTION: getter
 */
long MemberListEntry::gettimestamp() {
	return timestamp;
}

/**
 * FUNCTION NAME: setid
 *
 * DESCRIPTION: setter
 */
void MemberListEntry::setid(int id) {
	this->id = id;
}

/**
 * FUNCTION NAME: setport
 *
 * DESCRIPTION: setter
 */
void MemberListEntry::setport(short port) {
	this->port = port;
}

/**
 * FUNCTION NAME: setheartbeat
 *
 * DESCRIPTION: setter
 */
void MemberListEntry::setheartbeat(long hearbeat) {
	this->heartbeat = hearbeat;
}

/**
 * FUNCTION NAME: settimestamp
 *
 * DESCRIPTION: setter
 */
void MemberListEntry::settimestamp(long timestamp) {
	this->timestamp = timestamp;
}

bool MemberListEntry::operator==(const Address& addr) {
    return this->address == addr;
}

/**
 * Copy Constructor
 */
Member::Member(const Member &anotherMember) {
	this->addr = anotherMember.addr;
	this->inited = anotherMember.inited;
	this->inGroup = anotherMember.inGroup;
	this->bFailed = anotherMember.bFailed;
	this->nnb = anotherMember.nnb;
	this->heartbeat = anotherMember.heartbeat;
	this->pingCounter = anotherMember.pingCounter;
	this->timeOutCounter = anotherMember.timeOutCounter;
	this->memberList = anotherMember.memberList;
	this->myPos = anotherMember.myPos;
	this->mp1q = anotherMember.mp1q;
	this->traPointer = anotherMember.traPointer;
}

/**
 * Assignment operator overloading
 */
Member& Member::operator =(const Member& anotherMember) {
	this->addr = anotherMember.addr;
	this->inited = anotherMember.inited;
	this->inGroup = anotherMember.inGroup;
	this->bFailed = anotherMember.bFailed;
	this->nnb = anotherMember.nnb;
	this->heartbeat = anotherMember.heartbeat;
	this->pingCounter = anotherMember.pingCounter;
	this->timeOutCounter = anotherMember.timeOutCounter;
	this->memberList = anotherMember.memberList;
	this->myPos = anotherMember.myPos;
	this->mp1q = anotherMember.mp1q;
	this->traPointer = anotherMember.traPointer;
	return *this;
}

void Swap(MemberListEntry& MLE_1, MemberListEntry& MEL_2) {

    MemberListEntry tmpMLE = MEL_2;

    MEL_2 = MLE_1;

    MLE_1 = tmpMLE;

}
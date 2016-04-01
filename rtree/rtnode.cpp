#include "rtnode.h"

//======================== Entry implementation =====================================================

Entry::Entry():mbr() {
	this->rid = -1;
	this->ptr = NULL;
}

Entry::Entry(const BoundingBox& thatMBR, const int rid):mbr(thatMBR) {
	this->rid = rid;
	this->ptr = NULL;
}

Entry::Entry(const BoundingBox& thatMBR, RTNode* ptr):mbr(thatMBR) {
	this->rid = -1;
	this->ptr = ptr;
}

Entry& Entry::operator=(const Entry& other)
{
	if (&other != this) {
		this->mbr = other.mbr;
		this->rid = other.rid;
		this->ptr = other.ptr;

	}
	return *this;
}

Entry::~Entry() {
	this->ptr = NULL;
}

const BoundingBox& Entry::get_mbr() const {
	return this->mbr;
}	


RTNode* Entry::get_ptr() const {
	return this->ptr;
}

int Entry::get_rid() const { 
	return this->rid;
}

void Entry::set_rid(int rid){
	this->rid = rid;
}



void Entry::set_mbr(const BoundingBox& thatMBR) {
	this->mbr.set_boundingbox(thatMBR);//ATTENTION: should be deep copy...
}

void Entry::set_ptr(RTNode* ptr) {
	this->ptr = ptr;
}


void Entry::print() {
	this->mbr.print();
	cout << this->rid << endl;
	cout << this->ptr << endl;
}



//======================== RTNode implementation ==============================================

RTNode::RTNode(int lev, int s)
{
	entry_num = 0;
	entries = new Entry[s];
	level = lev;
	size = s;
	parent = NULL;
}

RTNode::RTNode(const RTNode& other)
{
	entries = new Entry[other.size];
	*this = other;
}

RTNode& RTNode::operator=(const RTNode& other)
{
	if (&other != this) {
		entry_num = other.entry_num;
		level = other.level;
		size = other.size;
		parent = other.parent;
		for (int i = 0; i < entry_num; i++)
			entries[i] = other.entries[i];
	}

	return *this;
}


RTNode::~RTNode()
{
	if (level != 0) {
		for (int i = 0; i < entry_num; i++) {
			delete entries[i].get_ptr();
			entries[i].set_ptr(NULL);
		}
	}
	delete []entries;
	parent = NULL;
	entries = NULL;
}
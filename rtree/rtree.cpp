/* Implementations of R tree */
#include <cmath>
#include "rtree.h"


const double EPSILON = 1E-10;

RTree::RTree(int entry_num)
{
	max_entry_num = entry_num;
	dimension = 2;//by default
	root = new RTNode(0, entry_num);
}

RTree::RTree(int entry_num, int dim)
{
	max_entry_num = entry_num;
	dimension = dim;//by default
	root = new RTNode(0, entry_num);
}

RTree::~RTree()
{
	delete root;
	root = NULL;
}


bool RTree::insert(const vector<int>& coordinate, int rid)
{
	if (coordinate.size() != this->dimension)
	{
		cerr << "R-tree dimensionality inconsistency\n";
	}

	/***
	ADD YOUR CODE HERE
	****/
}

void RTree::query_range(const BoundingBox& mbr, int& result_count, int& node_travelled)
{
	if (mbr.get_dim() != this->dimension)
	{
		cerr << "R-tree dimensionality inconsistency\n";
	}

	/***
	ADD YOUR CODE HERE
	****/
}


bool RTree::query_point(const vector<int>& coordinate, Entry& result)
{
	if (coordinate.size() != this->dimension)
	{
		cerr << "R-tree dimensionality inconsistency\n";
	}

	/***
	ADD YOUR CODE HERE
	****/
}


/**********************************
 *
 * Please do not modify the codes below
 *
 **********************************/

//
// Calcuate the MBR of a set of entries, of size ``len''.
// Store the MBR in the first entry
//
BoundingBox RTree::get_mbr(Entry* entry_list, int len)
{
	BoundingBox mbr(entry_list[0].get_mbr());
	for (int i = 1; i < len; i++) {        
		mbr.group_with(entry_list[i].get_mbr());
	}
	return mbr;
}


/*********************************************************
  Return true means choose box1 for tie breaking.
  If the two boxes is the same, return true.
  This is to give a unified way of tie-breaking such that if your program is correct, then the result should be same, not influnced by any ties.
 *********************************************************/
bool RTree::tie_breaking(const BoundingBox& box1, const BoundingBox& box2)
{
	//for every dimension, try to break tie by the lowest value, then the highest
	for (int i = 0; i < box1.get_dim(); i++)
	{
		if (box1.get_lowestValue_at(i) != box2.get_lowestValue_at(i))
		{
			return box1.get_lowestValue_at(i) < box2.get_lowestValue_at(i);
		}
		else if (box1.get_highestValue_at(i) != box2.get_highestValue_at(i))
		{
			return box1.get_highestValue_at(i) > box2.get_highestValue_at(i);
		}
	}
	return true;
}


void RTree::stat(RTNode* node, int& record_cnt, int& node_cnt)
{
	if (node->level == 0) {
		record_cnt += node->entry_num;
		node_cnt++;
	}
	else {
		node_cnt++;
		for (int i = 0; i < node->entry_num; i++)
			stat((node->entries[i]).get_ptr(), record_cnt, node_cnt);
	}
}

void RTree::stat()
{
	int record_cnt = 0, node_cnt = 0;
	stat(root, record_cnt, node_cnt);
	cout << "Height of R-tree: " << root->level + 1 << endl;
	cout << "Number of nodes: " << node_cnt << endl;
	cout << "Number of records: " << record_cnt << endl;
	cout << "Dimension: " << dimension << endl;
}


void RTree::print_node(RTNode* node, int indent_level)
{
	BoundingBox mbr = get_mbr(node->entries, node->entry_num);

	char* indent = new char[4*indent_level+1];
	memset(indent, ' ', sizeof(char) * 4 * indent_level);
	indent[4*indent_level] = '\0';

	if (node->level == 0) {
		cout << indent << "Leaf node (level = " << node->level << ") mbr: (";
		for (int i = 0; i < mbr.get_dim(); i++)
		{
			cout << mbr.get_lowestValue_at(i) << " " << mbr.get_highestValue_at(i);
			if (i != mbr.get_dim() - 1)
			{
				cout << " ";
			}
		}
		cout << ")\n";
	}
	else {

		cout << indent << "Non leaf node (level = " << node->level << ") mbr: (";
		for (int i = 0; i < mbr.get_dim(); i++)
		{
			cout << mbr.get_lowestValue_at(i) << " " << mbr.get_highestValue_at(i);
			if (i != mbr.get_dim() - 1)
			{
				cout << " ";
			}
		}
		cout << ")\n";
	}

	Entry *copy = new Entry[node->entry_num];
	for (int i = 0; i < node->entry_num; i++) {
		copy[i] = node->entries[i];
	}

	for (int i = 0; i < node->entry_num; i++) {
		int index = 0; // pick next.
		for (int j = 1; j < node->entry_num - i; j++) {
			if (tie_breaking(copy[j].get_mbr(), copy[index].get_mbr())) {
				index = j;
			}
		}

		if (node->level == 0) {
			Entry& e = copy[index];
			cout << indent << "    Entry: <";
			for (int i = 0; i < e.get_mbr().get_dim(); i++)
			{
				cout << e.get_mbr().get_lowestValue_at(i) << ", ";
			}
			cout << e.get_rid() << ">\n";
		}
		else {
			print_node(copy[index].get_ptr(), indent_level+1);
		}
		// Move the output one to the rear.
		Entry tmp = copy[node->entry_num - i - 1];
		copy[node->entry_num - i - 1] = copy[index];
		copy[index] = tmp;

	}

	delete []indent;
	delete []copy;
}

void RTree::print_tree()
{
	if (root->entry_num == 0)
		cout << "The tree is empty now." << endl;
	else
		print_node(root, 0);
}
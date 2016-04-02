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

/*
* Helper function to calculate the area enlargement after enclose a new entry in a bounding box
*/
int RTree::calc_area_enlargement(const BoundingBox& original,const BoundingBox& addition)
{
	BoundingBox tmp = BoundingBox(original);
	tmp.group_with(addition);
	return tmp.get_area()-original.get_area();
}

/* Helper function to swap two entries in RTNode. */

void RTree::swap_leaf_node_entry(Entry& entry1, Entry& entry2)
{
				//swap needs deep copy
    			Entry tmp = Entry(entry1.get_mbr(),entry1.get_rid());
    			tmp.set_ptr(entry1.get_ptr());
    			entry1.set_rid(entry2.get_rid());
    			entry1.set_mbr(entry2.get_mbr());
    			entry1.set_ptr(entry2.get_ptr());
    			entry2.set_rid(tmp.get_rid());
    			entry2.set_mbr(tmp.get_mbr());
    			entry2.set_ptr(tmp.get_ptr());

}

/*
* function to choose the leaf for new entry insertion
*/
RTNode* RTree::choose_leaf(const Entry& newEntry)
{
	RTNode* cur = root;
	while(true)
	{
		if(cur->level==0) //when arrive at leaf node
		{
			return cur;
		}

		int min_diff = calc_area_enlargement(cur->entries[0].get_mbr(),newEntry.get_mbr());
		int minIdx = 0;
		for(int i=1;i<cur->entry_num;++i)
		{
			int diff_area = calc_area_enlargement(cur->entries[i].get_mbr(),newEntry.get_mbr());
			if(diff_area<min_diff)
			{
				min_diff = diff_area;
				minIdx = i;
			}
			else if(diff_area==min_diff)
			{
				//choose smaller area to resolve tie, if still tie, use tie_breaking to resolve
				if(cur->entries[i].get_mbr().get_area()<cur->entries[minIdx].get_mbr().get_area())
				{
					min_diff = diff_area;
					minIdx = i;
				}
				else if(cur->entries[i].get_mbr().get_area()==cur->entries[minIdx].get_mbr().get_area())
				{
					if(tie_breaking(cur->entries[i].get_mbr(),cur->entries[minIdx].get_mbr()))
					{
						min_diff = diff_area;
                        minIdx = i;
					}
				}
			}
		}// find the appropriate entry to include the new entry

		cur = cur->entries[minIdx].get_ptr();

	}

}
/*
* function to set the two index number of the entries list which form the extreme pair
* according to linear-cost algorithm.
*/
void RTree::linear_pick_seeds(RTNode* l,Entry& newEntry,int& idx1,int& idx2)
{
	vector<int> lowMax_idxs;
	vector<int> highMin_idxs;
	vector<int> norm_width_diff;
	lowMax_idxs.reserve(dimension);//newEntry.get_mbr().get_dim()
	highMin_idxs.reserve(dimension);
	norm_width_diff.reserve(dimension);

	for(int i=0;i<dimension;++i)
	{
		int lowMax = newEntry.get_mbr().get_lowestValue_at(i);
		int highMin = newEntry.get_mbr().get_highestValue_at(i);
		int lowMax_idx = max_entry_num;//virtual index for newEntry
		int highMin_idx = max_entry_num;//for newEntry
		int lower_bound = lowMax;
		int higher_bound = highMin;
		for(int j=0;j<max_entry_num;++j)
		{
			int lowTmp = l->entries[j].get_mbr().get_lowestValue_at(i);
			int highTmp = l->entries[j].get_mbr().get_highestValue_at(i);

			lower_bound = lower_bound<lowTmp?lower_bound:lowTmp;
            higher_bound = higher_bound>highTmp?higher_bound:highTmp;

			if((lowTmp>lowMax)|| (lowTmp==lowMax && !tie_breaking(l->entries[j].get_mbr(),newEntry.get_mbr())))
			{
				lowMax = lowTmp;
				lowMax_idx = j;
			}

			if((highTmp<highMin)|| (highTmp==highMin && tie_breaking(l->entries[j].get_mbr(),newEntry.get_mbr())))
            {
        		highMin = highTmp;
            	highMin_idx = j;
          	}

		}
		lowMax_idxs[i] = lowMax_idx;
		highMin_idxs[i] = highMin_idx;
		norm_width_diff[i] = (highMin-lowMax)/(higher_bound-lower_bound);

	}//looping over dimensions
	//after finding the information on each dimension.

	int extreme_pair_idx = 0;
	int extreme_pair_value = norm_width_diff[0];
	for(int i=1;i<dimension;++i)
	{
		if(norm_width_diff[i]>extreme_pair_value)
		{
			extreme_pair_value = norm_width_diff[i];
			extreme_pair_idx = i;
		}
	}


	//if the lowMax and highMin belong to the same entry
	if(lowMax_idxs[extreme_pair_idx]==highMin_idxs[extreme_pair_idx])
	{
		int tmp_index = lowMax_idxs[extreme_pair_idx];
		//insert the selected one if
		//sort the remaining entries except for A. O(n^2) time complexity using tie_breaking.but this is just corner case
		//so does not affect the amortized analysis
		if(tmp_index!=max_entry_num)
		{
			//if the special element is not the newEntry, just insert newEntry to the spefic position and sort the entries list
			//otherwise leave the list its way
			swap_leaf_node_entry(l->entries[tmp_index],newEntry);
		}

//		for(int i=0;i<max_entry_num;++1)
//		{
//			for(int j=i+1;j<max_entry_num;++j)
//			{
//				if(tie_breaking(l->entries[i].get_mbr(),l->entries[j].get_mbr()))
//				{
//					swap_leaf_node_entry(l->entries[i].get_mbr(),l->entries[j].get_mbr());
//				}
//			}
//		}
		//one of them is copied to newEntry, one of them if changed to the first entry of l->entries
//		idx1 = 0;
//		idx2 = max_entry_num;


	}
	else //successfully selected out the two seeds
	{
		int lowMax_idx = lowMax_idxs[extreme_pair_idx];
        int highMin_idx = highMin_idxs[extreme_pair_idx];

		idx1 = lowMax_idx<highMin_idx?lowMax_idx:highMin_idx; //the smaller one of the two indexs
		idx2 = lowMax_idx>highMin_idx?lowMax_idx:highMin_idx; //the larger one of the two indexs

		//actually just swapping it to the first entry and newEntry. and sort the remaining ones for the pickNext

		swap_leaf_node_entry(l->entries[idx1],l->entries[0]);
		if(idx2<max_entry_num)
		{
			swap_leaf_node_entry(l->entries[idx2],newEntry);
		}

//		idx1 = 0;
//      idx2 = max_entry_num;

	}

	for(int i=0;i<max_entry_num;++i)
    {
    	for(int j=i+1;j<max_entry_num;++j)
   		{
   			if(tie_breaking(l->entries[i].get_mbr(),l->entries[j].get_mbr()))
    		{
    			swap_leaf_node_entry(l->entries[i],l->entries[j]);
   			}
    	}
   	}

   	idx1 = 0;
    idx2 = max_entry_num;

}


/*
* function to split the node when the entries number is exceeding the max_entry_num
*/

void RTree::split_node(RTNode* l, RTNode* ll, Entry& newEntry)
{

	int idx1 = 0;
	int idx2 = 0;
	linear_pick_seeds(l,newEntry,idx1,idx2);
	ll->entries[0] = newEntry;

	int l_next_idx = 1;
	int ll_next_idx = 1;
	//assign the remaining entries in the order of sorted list

	BoundingBox l_mbr = BoundingBox(l->entries[0].get_mbr());
	BoundingBox ll_mbr =  BoundingBox(newEntry.get_mbr());

	int i;
	for(i=1;i<max_entry_num;++i)
	{
		if(l_next_idx>=max_entry_num/2 || ll_next_idx>=max_entry_num/2)
		{//one of the node arrive the max number of entries,resulting that the other one has too few entries
			break;
		}
		int l_area_expansion = calc_area_enlargement(l_mbr,l->entries[i].get_mbr());
		int ll_area_expansion = calc_area_enlargement(ll_mbr,l->entries[i].get_mbr());

		if(l_area_expansion<ll_area_expansion)
		{
			l->entries[l_next_idx++] = l->entries[i];
        	l_mbr.group_with(l->entries[i].get_mbr());

		}
		else if(l_area_expansion>ll_area_expansion)
		{
			ll->entries[ll_next_idx++] = l->entries[i];
			ll_mbr.group_with(l->entries[i].get_mbr());

		}
		else if(l_next_idx<ll_next_idx)
		{
			l->entries[l_next_idx++] = l->entries[i];
			l_mbr.group_with(l->entries[i].get_mbr());

		}
		else if(l_next_idx>ll_next_idx)
		{
			ll->entries[ll_next_idx++] = l->entries[i];
			ll_mbr.group_with(l->entries[i].get_mbr());

		}
		else if(tie_breaking(l_mbr,ll_mbr))
		{
			l->entries[l_next_idx++] = l->entries[i];
        	l_mbr.group_with(l->entries[i].get_mbr());

		}
		else
		{
			ll->entries[ll_next_idx++] = l->entries[i];
			ll_mbr.group_with(l->entries[i].get_mbr());

		}
	}
	//put the remaining value together into one which has too few entries.
	if(l_next_idx<ll_next_idx)
	{
		for(;i<max_entry_num;++i)
		{
			l->entries[l_next_idx++] = l->entries[i];
            l_mbr.group_with(l->entries[i].get_mbr());
		}
	}
	else
	{
		for(;i<max_entry_num;++i)
        {
        	ll->entries[ll_next_idx++] = l->entries[i];
            ll_mbr.group_with(l->entries[i].get_mbr());
        }

	}

	l->entry_num = l_next_idx;
	ll->entry_num = ll_next_idx;
}

void RTree::adjust_tree(RTNode* l,RTNode* ll)
{
	//remember to grow the tree taller if neccessary
	RTNode* N = l;
	RTNode* NN = ll;
	while(true)
	{
		if(N==root)
		{
			if(NN!=NULL){//if the root splits, grow the tree height
        		root = new RTNode(N->level+1,N->size);
        		Entry N_entry = Entry();
                N_entry.set_mbr(get_mbr(N->entries,N->entry_num));
                N->parent = root;
                N_entry.set_ptr(N);
       			root->entries[root->entry_num++] = N_entry;

       			Entry NN_entry = Entry();
       			NN_entry.set_mbr(get_mbr(NN->entries,NN->entry_num));
       			NN->parent = root;
                NN_entry.set_ptr(NN);
                root->entries[root->entry_num++] = NN_entry;
             }
             //when arriving at root, end the tree adjustment.
                break;
        }

		RTNode* p = N->parent;
		//find the entry En in parent RTNode pointing to N
		for(int i=0;i< p->entry_num;++i) //update the bounding box area
		{
			if(p->entries[i].get_ptr()==N)
			{
				p->entries[i].set_mbr(get_mbr(N->entries,N->entry_num));
				break;
			}
		}

		//if split, propagate the node split upward
		if(NN!=NULL)
		{
			Entry newEntry = Entry();
			newEntry.set_mbr(get_mbr(NN->entries,NN->entry_num));
			newEntry.set_ptr(NN);
			if(p->entry_num<max_entry_num) //still have free entry space,insert
			{
				p->entries[p->entry_num++] = newEntry;
				NN = NULL;
			}
			else // split the node again
			{
				RTNode* pp = new RTNode(p->level,p->size);
				pp->parent = p->parent;// every time construct a slit node dont forget to set its parent
                split_node(p,pp,newEntry);
               	NN = pp; //if a node split happens, include it into the
			}
		}
		N = p;//move up to next level
	}

}


bool RTree::insert(const vector<int>& coordinate, int rid)
{
	if (coordinate.size() != this->dimension)
	{
		cerr << "R-tree dimensionality inconsistency\n";
		return false;
	}

	/***
	ADD YOUR CODE HERE
	****/
	BoundingBox bb = BoundingBox(coordinate,coordinate);
	Entry newEntry = Entry(bb,rid);

	RTNode* l = choose_leaf(newEntry);
	//first check if there already is the point with same key.(it will definitedly be chosen becaruse the area
	//enlargment is 0.
	for(int i=0;i<l->entry_num;++i)
	{
		if(newEntry.get_mbr().is_equal(l->entries[i].get_mbr())) return false;
	}

	// the leafNode to insert is full,need to do the split,or could use l.entry_num==l.size
	if(l->entry_num == max_entry_num)
	{
		RTNode* ll = new RTNode(l->level,l->size);
		ll->parent = l->parent; //set to the same parent
		split_node(l,ll,newEntry);
		adjust_tree(l,ll);
	}
	else //insert the newEntry into the leaf node
	{
		l->entries[l->entry_num++] = newEntry;
		adjust_tree(l,NULL);
	}


	return true;
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
	return false;
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
	if (node->level == 0) {//when level==0, it is leaf node
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

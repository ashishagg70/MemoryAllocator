/*
 * mm2.c - 
 * Here we have build a binary search tree based on address to maintain freenodes in a tree.
 * Header of each node is just maintaining size of block and free node's tree information 
 * is stored in free node's data part.
 * Tree is modified to maintain max size on right subtree and left subtree to make searching of 
 * hole faster.
 * 
 * searching in tree is following first fit policy.
 * 
 * whenever a malloc/ realloc request comes it first searches in tree for the appropriate size hole
 * and return that if matches the request. NOTE it don't need to search whole tree for eg:
 * if size of root and max size on left and right is less than size requerired it will return from 
 * root node only.asymptotic analysis of search a whole even if we have sorted based on address 
 * in this case is log(n). In worst case it is O(n) though worst case won't occur often as no of 
 * nodes are reducing while coalescing if node are inserting on same side there is high probability 
 * they will be coalesced.
 * 
 * we are doing coalesce and split on tree node if possible.
 * 
 * This is better than linklist and naive implementation because here free node insertion
 * searching of hole is taking amortised log(n) time.
 * Also in term of utilization it is better than naive because we are doing coalescing and splitting.
 * 
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"
void * findHole(size_t holeSize);
void * splitNode(info_t * root, size_t holeSize);
void deleteNode(info_t *root);
void deleteAdujstMaxSizes(info_t * node);
void insert(info_t * root, info_t * newFreeNode );
void coalesce(info_t * root, info_t* newFreeNode );
void initializeInfoBlock(info_t* info);
/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Team_Rocket",
    /* First member's full name */
    "Ashish Aggarwal",
    /* First member's email address */
    "ashishaggarwal@cse.iitb.ac.in",
    /* Second member's full name (leave blank if none) */
    "Ajaykumar Kushwaha",
    /* Second member's email address (leave blank if none) */
    "ajaykushwaha@cse.iitb.ac.in"
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* 
 * mm_init - initialize the malloc package.
 */

void *init_mem_sbrk_break = NULL;
info_t *head=NULL;
size_t hsize;
size_t minBlock;

int mm_init(void)
{
	hsize=sizeof(header_t);
	hsize=((hsize+7)/8)*8;
	minBlock=sizeof(info_t);
	head=NULL;
		
    return 0;		//Returns 0 on successfull initialization.
}

//---------------------------------------------------------------------------------------------------------------
/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer or from free tree.
 * Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{	

	/* 
	 * This function should keep track of the allocated memory blocks.
	 * The block allocation should minimize the number of holes (chucks of unusable memory) in the heap memory.
	 * The previously freed memory blocks should be reused.
	 * If no appropriate free block is available then the increase the heap  size using 'mem_sbrk(size)'.
	 * Try to keep the heap size as small as possible.
	 */

	if(size <= 0){		// Invalid request size
		return NULL;
	}
	if(size<minBlock)
		size=minBlock;
	if(head!=NULL){
		size_t holeSize=((size+7)/8)*8;
		char * hole=findHole(holeSize);
		if(hole!=NULL){
			header_t *header=hole-hsize;
			return hole;
		}
			
		
	}
	size+=hsize;
	
	size = ((size+7)/8)*8;		//size alligned to 8 bytes
	char * newAllocAddress=(char*)mem_sbrk(size);
	header_t *header=newAllocAddress;
	header->size=size-hsize;
	return newAllocAddress+hsize;

}


void mm_free(void *ptr)
{
	header_t *header=(char *)ptr - hsize;
	info_t * info = ptr;
	initializeInfoBlock(info);
	info->selfAddress=ptr;
	info->size=header->size;
	
	if(head==NULL){
		head=info;
	}
	else{
		insert(head, info);
	}
	/* 
	 * Searches the previously allocated node for memory block with base address ptr.
	 * 
	 * It should also perform coalesceing on both ends i.e. if the consecutive memory blocks are 
	 * free(not allocated) then they should be combined into a single block.
	 * 
	 * It should also keep track of all the free memory blocks.
	 */
}

void initializeInfoBlock(info_t* info){
	info->maxLeft=0;
	info->maxRight=0;
	info->next=NULL;
	info->prev=NULL;
	info->parent=NULL;
	info->selfAddress=NULL;
	info->size=0;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{		
	int n=10;
	if(ptr == NULL){			//memory was not previously allocated
		return mm_malloc(n*size);
	}
	if(size == 0){				//new size is zero
		mm_free(ptr);
		return NULL;
	}
	header_t* ptrHeader=(char*)ptr-hsize;
	if(ptrHeader->size>=size)
		return ptr;

	void * addr = mm_malloc(n*size);
	header_t* addrHeader = (char*)addr-hsize;
	memcpy(addr, ptr, n*size);
	mm_free(ptr);
	return addr;
	
}

void * findHole(size_t holeSize){
	if(head->size<holeSize && head->maxLeft<holeSize && head->maxRight<holeSize)
		return NULL;
	info_t * curr=head;
	while(curr!=NULL && curr->size<holeSize){
		if(curr->maxLeft>=holeSize){
			curr=curr->prev;
		}
		else if(curr->maxRight>=holeSize){
			curr=curr->next;
		}
	}
	char * hole =NULL;
	if(curr->size>=hsize+holeSize+minBlock){
		hole = splitNode(curr, holeSize);
	}
	else{
		header_t *h = (char*)curr->selfAddress - hsize;
		h->size=curr->size;
		hole = curr->selfAddress;
		deleteNode(curr);
	}
	return hole;

}

void * splitNode(info_t * root, size_t holeSize){
	
	root->size-=(holeSize+hsize);
	header_t * hole=root->selfAddress+root->size;
	hole->size=holeSize;
	deleteAdujstMaxSizes(root);
	return (char *)hole+hsize;

}

void deleteNode(info_t *root){
	//adust max
	if(root->next==NULL || root->prev==NULL){
		//one child or no child
		info_t *child=root->next?root->next:root->prev;
		size_t maxchildSize=root->next?root->maxRight:root->maxLeft;
		if(child)
				child->parent=root->parent;
		if(root->parent==NULL)
			head=child;
		else{
			if(root->parent->prev==root){
				root->parent->prev=child;
				root->parent->maxLeft=maxchildSize;
			}
			else if(root->parent->next==root){
				root->parent->next=child;
				root->parent->maxRight=maxchildSize;
			}
			deleteAdujstMaxSizes(root->parent);
		}
	}
	else{
		// both left and right of node to be delete are not null
		info_t * successor=root->next;
		while(successor->prev!=NULL)
			successor = successor->prev;
		//free the sucesser from right bond
		if(successor->parent->prev==successor){
			successor->parent->prev=successor->next;
			successor->parent->maxLeft=successor->maxRight;
			deleteAdujstMaxSizes(successor->parent);
		}
		else if(successor->parent->next==successor){
			successor->parent->next=successor->next;
			successor->parent->maxRight=successor->maxRight;
			deleteAdujstMaxSizes(successor->parent);
		}
		if(successor->next!=NULL){
			successor->next->parent=successor->parent;
		}
		//freeing from right bond done
		if(root->parent==NULL)
			head=successor;
		else{
			if(root->parent->next==root){
				root->parent->next=successor;
			}
			else if(root->parent->prev==root){
				root->parent->prev=successor;
			}	
		}
		successor->prev=root->prev;
		root->prev->parent=successor;
		successor->maxLeft=root->maxLeft;
		
		successor->next=root->next;
		if(root->next!=NULL)
			root->next->parent=successor;
		successor->maxRight=root->maxRight;
		successor->parent=root->parent;
		/*printNode(successor);
		if(successor->parent)
			printNode(successor->parent);*/
		deleteAdujstMaxSizes(successor);			
	}
}
void deleteAdujstMaxSizes(info_t * node){
	if(node->parent==NULL)
		return;
	info_t * curr=node;
	while(curr->parent!=NULL){	
		if(curr->parent->prev==curr){
			curr->parent->maxLeft=curr->maxLeft;
			if(curr->parent->maxLeft<curr->maxRight)
				curr->parent->maxLeft=curr->maxRight;
			if(curr->parent->maxLeft<curr->size)
				curr->parent->maxLeft=curr->size;
		}
		else if(curr->parent->next==curr){
			curr->parent->maxRight=curr->maxLeft;
			if(curr->parent->maxRight<curr->maxRight)
				curr->parent->maxRight=curr->maxRight;
			if(curr->parent->maxRight<curr->size)
				curr->parent->maxRight=curr->size;
		}
		curr=curr->parent;
	}
}
/*
 * This function will be called when new free block arrives.
*/
void insert(info_t * root, info_t * newFreeNode ){
	if((root->selfAddress+root->size==newFreeNode->selfAddress-hsize) || (newFreeNode->selfAddress+newFreeNode->size==root->selfAddress-hsize)){
		coalesce(root, newFreeNode);
		return;
	}
	else{
		if(newFreeNode->selfAddress<root->selfAddress)
		{
			if(root->prev==NULL)
			{
				root->prev=newFreeNode;
				root->maxLeft=newFreeNode->size;
				newFreeNode->parent=root;
				deleteAdujstMaxSizes(root);

			}
			else{
				insert(root->prev, newFreeNode);
				return;
			}
			
		}
		else{
			if(root->next==NULL)
			{
				root->next=newFreeNode;
				root->maxRight=newFreeNode->size;
				newFreeNode->parent=root;
				deleteAdujstMaxSizes(root);

			}
			else{
				insert(root->next, newFreeNode);
				return;
			}
		}
	}
	

}


void coalesce(info_t * root, info_t* newFreeNode ){
	//free node aage attach hoga
	if(root->selfAddress+root->size==newFreeNode->selfAddress-hsize){
		root->size+=newFreeNode->size+hsize;
		//check if successor is insuccession
		if(root->next!=NULL){
			info_t *successor=root->next;
			while(successor->prev!=NULL){
				successor=successor->prev;
			}
			if(root->selfAddress+root->size==successor->selfAddress-hsize){
				root->size+=successor->size+hsize;
				deleteNode(successor);
			}
			else
				deleteAdujstMaxSizes(root);

		}
		else
			deleteAdujstMaxSizes(root);
		

		
	}
	else if(newFreeNode->selfAddress+newFreeNode->size==root->selfAddress-hsize){
		root->selfAddress=newFreeNode->selfAddress;
		root->size+=newFreeNode->size+hsize;

		if(root->prev!=NULL){
			info_t * predecessor = root->prev;
			while(predecessor->next!=NULL)
				predecessor = predecessor->next;
			if(predecessor->selfAddress+predecessor->size==root->selfAddress-hsize){
				root->selfAddress=predecessor->selfAddress;
				root->size+=predecessor->size+hsize;
				deleteNode(predecessor);
			}
			else
				deleteAdujstMaxSizes(root);
			
		}
		else
			deleteAdujstMaxSizes(root);
		
		memcpy(root->selfAddress, root, minBlock);

		//update parent
		if(root->parent!=NULL){
			if(root->parent->prev==root){
				root->parent->prev=root->selfAddress;
			}
			else if(root->parent->next==root){
				root->parent->next=root->selfAddress;
			}
		}
		else
			head=root->selfAddress;
		//update children	
		if(root->next!=NULL)
			root->next->parent=root->selfAddress;
		if(root->prev!=NULL)
			root->prev->parent=root->selfAddress;
	}
}












/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 * 
 * keep inmind min size for block to keep the info_t in freeblock
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
void adjustMaxSizes(info_t* root);
void coalesce(info_t * root, info_t* newFreeNode );
void initializeInfoBlock(info_t* info);
void copyStructure(info_t *source, info_t* destination);
void printInorder(info_t * root) ;
int printTreeSize(info_t * root) ;
void printpreorder(info_t * root) ;
void assertStruct(info_t* I1, info_t* I2);
void printNode(info_t * root);
int count=0;
/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Convolution",
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
/*int main()
{
	mem_init();
	char* addr=mem_sbrk(28);
	
	info_t* info=addr;
	header_t* h1=addr+sizeof(info);
	h1->size=10;
	info->maxLeft=10;
	int x= sizeof(h1);
	int x2= sizeof(info);
	
	printf("addr=%ld\n",addr);
	printf("addr=%ld\n",(info_t*)addr+1);
	printf("x=%d, x2=%d\n",h1->size,info->maxLeft);
	mem_deinit();
	return 0;
}*/
int mm_init(void)
{
	hsize=sizeof(header_t);
	hsize=((hsize+7)/8)*8;
	minBlock=sizeof(info_t);
	head=NULL;
		
	//This function is called every time before each test run of the trace.
	//It should reset the entire state of your malloc or the consecutive trace runs will give wrong answer.	
	

	/* 
	 * This function should initialize and reset any data structures used to represent the starting state(empty heap)
	 * 
	 * This function will be called multiple time in the driver code "mdriver.c"
	 */
	
    return 0;		//Returns 0 on successfull initialization.
}

//---------------------------------------------------------------------------------------------------------------
/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{	
	printf("\nraw allocation demand: %d\n", size);
	printf("min block size: %d\n", minBlock);
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
	//printpreorder(head);
	printf("stuck in malloc\n");
	if(head!=NULL){
		printf("head%lu\n", head->selfAddress);
		size_t holeSize=((size+7)/8)*8;
		char * hole=findHole(holeSize);
		printTreeSize(head);
		printpreorder(head);
		if(hole!=NULL)
			return hole;
		else
			printf("hole found not");
		
	}
	size+=hsize;
	
	size = ((size+7)/8)*8;		//size alligned to 8 bytes
	printf("\nallocation size: %d\n", size-hsize);
	char * newAllocAddress=(char*)mem_sbrk(size);
	header_t *header=newAllocAddress;
	header->size=size-hsize;
	printf("\nstarting address header: %lu\n", newAllocAddress);
	printf("\nstarting address block: %lu\n", newAllocAddress+hsize);
	return newAllocAddress+hsize;		//mem_sbrk() is wrapper function for the sbrk() system call. 
								//Please use mem_sbrk() instead of sbrk() otherwise the evaluation results 
								//may give wrong results
}


void mm_free(void *ptr)
{
	printf("\nfree block address: %ul\n", ptr);
	header_t *header=(char *)ptr - hsize;
	
	info_t * info = ptr;
	initializeInfoBlock(info);
	info->selfAddress=ptr;
	info->size=header->size;
	printf("\nfree block header size: %d\n", info->size);
	if(head==NULL){
		head=info;
	}
	else{
		printf("before adding tree\n");
		//printInorder(head);
		insert(head, info);
		printf("after adding tree\n");
		
		printf("treesize: %d\n", printTreeSize(head));
		printpreorder(head);
		//printInorder(head);
		
	}
	/* 
	 * Searches the previously allocated node for memory block with base address ptr.
	 * 
	 * It should also perform coalesceing on both ends i.e. if the consecutive memory blocks are 
	 * free(not allocated) then they should be combined into a single block.
	 * 
	 * It should also keep track of all the free memory blocks.
	 * If the freed block is at the end of the heap then you can also decrease the heap size 
	 * using 'mem_sbrk(-size)'.
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
	size = ((size+7)/8)*8; //8-byte alignement	
	
	if(ptr == NULL){			//memory was not previously allocated
		return mm_malloc(size);
	}
	
	if(size == 0){				//new size is zero
		mm_free(ptr);
		return NULL;
	}

	/*
	 * This function should also copy the content of the previous memory block into the new block.
	 * You can use 'memcpy()' for this purpose.
	 * 
	 * The data structures corresponding to free memory blocks and allocated memory 
	 * blocks should also be updated.
	*/

	mm_free(ptr);
	return mem_sbrk(size);
	
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
	printf("\nhole choosen to be deleted: %lu\n", curr->selfAddress);
	char * hole =NULL;
	if(curr->size>=hsize+holeSize+minBlock){
		printf("splitting starting\n");
		hole = splitNode(curr, holeSize);
		printf("splitsuccessful\n");
	}
	else{
		printf("no split direct delete\n");
		header_t *h = (char*)curr->selfAddress - hsize;
		h->size=curr->size;
		hole = curr->selfAddress;
		deleteNode(curr);
	}
	return hole;

}

void * splitNode(info_t * root, size_t holeSize){
	printf("in splitnode\n");
	printf("\nsplit block new address(root->selfAddress): %lu\n", root->selfAddress);
	printf("split block address(root): %lu\n", root);
	printf("split block size: %d\n", root->size);
	printf("holesize: %d\n", holeSize);
	
	root->size-=(holeSize+hsize);
	header_t * hole=root->selfAddress+root->size;
	hole->size=holeSize;
	//adjust sizes
	deleteAdujstMaxSizes(root);
	//adjusting done
	//printInorder(head);
	
	
	printf("out splitnode\n");
	return (char *)hole+hsize;

}
void printNode(info_t * root){
	printf("maxLeft: %5d, maxright: %5d, start: %10lu, end: %10lu, size: %5d, left: %10lu, right: %10lu, parent: %10lu\n", root->maxLeft,root->maxRight, root->selfAddress, root->selfAddress+root->size, root->size, root->prev,root->next, root->parent);
}
void printInorder(info_t * root) 
{ 
    /*if (root == NULL) 
        return; 
    printInorder(root->prev); 
    printnode
    printInorder(root->next);*/
} 
void printpreorder(info_t * root) 
{ 
   /* if (root == NULL) 
        return; 
	printNode(root);
    printpreorder(root->prev); 
    printpreorder(root->next);*/
} 
int printTreeSize(info_t * root) 
{ 
    if (root == NULL)  
        return 0;  
	else
        return(printTreeSize(root->prev) + 1 + printTreeSize(root->next));
} 
void deleteNode(info_t *root){
	//adust max
	printf("in deletel %lu\n", root->selfAddress);
	if(root->next==NULL || root->prev==NULL){
		//one child or no child
		printf("single or no child delete\n");
		info_t *child=root->next?root->next:root->prev;
		size_t maxchildSize=root->next?root->maxRight:root->maxLeft;
		if(root->parent==NULL)
			head=child;
		else{
			if(child)
				child->parent=root->parent;
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
		printf("both child delete\n");
		info_t * successor=root->next;
		while(successor->prev!=NULL)
			successor = successor->prev;
		//free the sucesser from right bond
		if(successor->parent->prev==successor){
			successor->parent->prev=successor->next;
			successor->parent->maxLeft=successor->maxRight;
		}
		else if(successor->parent->next==successor){
			successor->parent->next=successor->next;
			successor->parent->maxRight=successor->maxRight;
		}
		if(successor->next!=NULL){
			successor->next->parent=successor->parent;
			deleteAdujstMaxSizes(successor->parent);
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
		printNode(successor);
		if(successor->parent)
			printNode(successor->parent);
		deleteAdujstMaxSizes(successor);			
	}
	printf("out deletel %lu\n", root->selfAddress);
}
void deleteAdujstMaxSizes(info_t * node){
	//printf("here %lu\n", node->selfAddress);
	//printpreorder(head);
	if(node->parent==NULL)
		return;
	info_t * curr=node;
	printf("in deleteAdujstMaxSizes: %lu, curr->size: %d\n", curr->selfAddress, curr->size);
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
		//printf("%lu\n",curr->selfAddress);
	}
	printf("out deleteAdujstMaxSizes\n");
}
/*
 * This function will be called when new free block arrives.
*/
void insert(info_t * root, info_t * newFreeNode ){
	//printf("insert\n");
	if((root->selfAddress+root->size==newFreeNode->selfAddress-hsize) || (newFreeNode->selfAddress+newFreeNode->size==root->selfAddress-hsize)){
		coalesce(root, newFreeNode);
		printf("after coalesce\n");
		return;
	}
	else{
		//printf("no coalesce\n");
		if(newFreeNode->selfAddress<root->selfAddress)
		{
			if(root->prev==NULL)
			{
				printf("left insert\n");
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
			//printf("root->next: %lu, freenode: %lu",root->next, newFreeNode->selfAddress);
			if(root->next==NULL)
			{
				printf("right insert\n");
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
	
	//printf("adjust sizes\n");
	//printInorder(head);

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
		//copyStructure(root, root->selfAddress);
		printf("memcopy done%lu\n", root->parent);
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
		assertStruct(root->selfAddress, root);
	}
}
void copyStructure(info_t *source, info_t* destination){
	destination->maxLeft=source->maxLeft;
	destination->maxRight=source->maxRight;
	destination->next=source->next;
	destination->prev=source->prev;
	destination->parent=source->parent;
	destination->selfAddress=source->selfAddress;
	destination->size=source->size;
}

void assertStruct(info_t* I1, info_t* I2){

	assert(I1->selfAddress==I2->selfAddress);
	assert(I1->prev==I2->prev);
	assert(I1->next==I2->next);
	assert(I1->parent==I2->parent);
	assert(I1->maxLeft==I2->maxLeft);
	assert(I1->size==I2->size);
	assert(I1->maxRight==I2->maxRight);
}






void adjustMaxSizes(info_t* root){
	info_t * curr=root;
	if(curr->parent!=NULL){
		if(curr->parent->prev==curr){
			if(curr->parent->maxLeft==curr->maxLeft || curr->parent->maxLeft==curr->maxRight)
				return;
			else{
				curr->parent->maxLeft=curr->maxLeft;
				if(curr->parent->maxLeft<curr->maxRight)
					curr->parent->maxLeft=curr->maxRight;
				if(curr->parent->maxLeft<curr->size)
					curr->parent->maxLeft=curr->size;
			}
		}
		else if(curr->parent->next==curr){
			if(curr->parent->maxRight==curr->maxLeft || curr->parent->maxRight==curr->maxRight)
				return;
			else{
				curr->parent->maxRight=curr->maxLeft;
				if(curr->parent->maxRight<curr->maxRight)
					curr->parent->maxRight=curr->maxRight;
				if(curr->parent->maxRight<curr->size)
					curr->parent->maxRight=curr->size;

			}
			
		}
	}
	// if(root->prev!=NULL && root->maxLeft<root->prev->size)
	// 	root->maxLeft=root->prev->size;
	// if(root->next!=NULL && root->maxLeft<root->prev->maxLeft)
	// 	root->maxLeft=root->prev->maxLeft;
	// if(root->prev!=NULL && root->maxRight<root->next->size)
	// 	root->maxRight=root->next->size;
	// if(root->next!=NULL && root->maxRight<root->next->maxRight)
	// 	root->maxRight=root->next->maxRight;
}







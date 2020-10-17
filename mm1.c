/*
 * mm1.c - Implemented a best-fit + two-sided coalescing memory manager via both header and footer
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

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


struct free_element
{
	int size;
	void *down;
	struct free_element *next_free_element;
	struct free_element *prev_free_element;
}*head,use_for_size_only;

/* 
 * mm_init - initialize the malloc package.
 */


int mm_init(void)
{
	
	head = NULL;
    return 0;		//Returns 0 on successfull initialization.
}


void display()
{
	struct free_element *ptr;

	struct free_element *pre_ptr;

	if(head==NULL)
	{
			printf("\nList is empty:\n");
			return;
	}
	else
	{
		ptr=head;
		//printf("\nThe List elements__ are:\n");
		while(ptr!=NULL)
		{
			
			printf("\n| %d - ",ptr->size);
			printf("%p %p  %p %p| --> ",ptr,ptr->prev_free_element,ptr->next_free_element,ptr->down);
			//pre_ptr = ptr;
			ptr=ptr->next_free_element ;
		}
		printf("\n\n\n");
	}
}

void swap_nodes(struct free_element *A,struct free_element *B)
{
	if(A==NULL || B==NULL)
	{
		printf("\nWrong Implmentation\n");
		return;
	}
	else
	{
		if ( A->prev_free_element )
			A->prev_free_element->next_free_element = B;

		if ( B->next_free_element )
			B->next_free_element->prev_free_element = A;

		A->next_free_element  = B->next_free_element;
		B->prev_free_element = A->prev_free_element;

		B->next_free_element = A;
		A->prev_free_element = B;

	}
}

void sort_free_item(struct free_element *element)
{

	if(element->next_free_element ==NULL && element->prev_free_element == NULL)
		return;
	
	while (element->prev_free_element != NULL)
	{
		if(element->prev_free_element > element)
		{
			swap_nodes(element->prev_free_element,element);
			//printf("\nSwapped   %p   %p\n",element,element->next_free_element);
			if(element->prev_free_element == NULL)
			{
				head = element;
			}
			//display();
		}
		else
		{
			break; 
		}
	}
	
	while (element->next_free_element != NULL)
	{
		if(element->next_free_element < element)
		{
			if(element->prev_free_element == NULL)
				head = element->next_free_element;

			swap_nodes(element,element->next_free_element);
			
		}
		else
		{
			break;
		}
	}

}

void coalese()
{
	int header_size;
	header_size = (((int)sizeof(struct free_element)+7)/8)*8;

	struct free_element *traverse;

	if(head == NULL || head->next_free_element == NULL)
		return;
	
	traverse = head;

	while (traverse!=NULL && traverse->next_free_element != NULL)
	{
		if(traverse->down == (void*)traverse->next_free_element - 1)
		{
			traverse->size = traverse->size + traverse->next_free_element->size + header_size;
			traverse->down = traverse->next_free_element->down;
			traverse->next_free_element = traverse->next_free_element->next_free_element;
			
			if(traverse->next_free_element !=NULL)
				traverse->next_free_element->prev_free_element = traverse;
		}
		else
		{
			traverse = traverse->next_free_element;
		}
	}
}

void *mm_malloc(size_t size)
{
	void *head_ptr,*base_ptr;

	struct free_element *element,*traverse,*min_space;
	size_t with_header_size;
	int header_size,extra_space;
	int min_space_found = 0;


	header_size = (((int)sizeof(struct free_element)+7)/8)*8;

	size = (((int)size+7)/8)*8; 

	with_header_size = size + header_size;

	if(size <= 0){		// Invalid request size
		return NULL;
	}


	traverse = head;
	min_space = head;

	if(head!=NULL)
	{
		traverse = head->next_free_element;

		if(head->size >= (int)size)
		{
			min_space_found = 1;
		}
	}

	//display();
	while (traverse!=NULL )
	{
		
		//printf("\nTraverse Size %d\n",min_space->size);
		if(traverse->size >= (int)size && (traverse->size < min_space->size || !min_space_found))
		{
			min_space_found = 1;
			min_space = traverse;
		}
		traverse = traverse->next_free_element;
	} 

	
	
	if(min_space!=NULL && min_space->size >= (int)size)
	{
		
		extra_space = min_space->size - (int)with_header_size;
		
		if(min_space->size == (int)size || extra_space <= 0)
		{
			size = (size_t)min_space->size; // overlapping with header

			if(min_space->next_free_element !=NULL)
				min_space->next_free_element->prev_free_element = min_space->prev_free_element;
			
			if(min_space->prev_free_element!=NULL)
				min_space->prev_free_element->next_free_element = min_space->next_free_element;       

			if(min_space->prev_free_element == NULL)  //Deleting Head
				head = min_space->next_free_element;   //make new Head

			head_ptr = min_space;
		}
		else
		{

			min_space->size = extra_space;
			min_space->down = (void*)min_space+min_space->size+header_size-1;

			head_ptr = (void*)min_space + header_size + extra_space;
		}

	}
	else
	{
		head_ptr = mem_sbrk(with_header_size);
	}


	

	element = (struct free_element *)head_ptr;

	element->size =(int)size;
	element->next_free_element = NULL;
	element->prev_free_element = NULL;
	element->down = (void*)element+element->size+header_size-1;

	base_ptr = head_ptr + header_size;

	//printf("\n Malloc %d",(int)size,header_size,with_header_size);
	//display();
	return base_ptr;
}


void mm_free(void *ptr)
{
	struct free_element *traverse,*element;
	void *head_ptr;
	int header_size;

	header_size = (((int)sizeof(struct free_element)+7)/8)*8;

	head_ptr = (void*)ptr - header_size;
	element = (struct free_element *)head_ptr;
	element->next_free_element = NULL;
	element->prev_free_element = NULL;

	if(head == NULL)
	{
		head = element;
	}
	else
	{

		element->next_free_element = head;
		head->prev_free_element = element;

		head = element;
	}
	



	//printf("\n Freed  %d\n",(int)element->size);
	//display();

	sort_free_item(element);
	//printf("\n Sorted %d\n",(int)element->size);
	//display();

	coalese();
	//printf("\nTried to coalese\n");
	//display();
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{	
	int header_size;
	struct free_element *element,*new_element;
	void *head_ptr,*new_ptr;

	size = ((size+7)/8)*8; //8-byte alignement	
	
	if(ptr == NULL){			//memory was not previously allocated
		return mm_malloc(10*size);
	}
	
	if(size == 0){				//new size is zero
		mm_free(ptr);
		return NULL;
	}


	header_size = (((int)sizeof(struct free_element)+7)/8)*8;
	head_ptr = (void*)ptr - header_size;
	element = (struct free_element *)head_ptr;

	//printf("\nrealloc %d  to  %d",element->size,(int)size);
	if(element->size < (int)size)
	{
		new_ptr = mm_malloc(10*size);

		memcpy(new_ptr,ptr,element->size);

		mm_free(ptr);

		return new_ptr;
	}
	else
	{
		return ptr;
	}


	
}















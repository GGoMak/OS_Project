#include <mem/palloc.h>
#include <bitmap.h>
#include <type.h>
#include <round.h>
#include <mem/mm.h>
#include <synch.h>
#include <device/console.h>
#include <mem/paging.h>
#include<mem/swap.h>

/* Page allocator.  Hands out memory in page-size (or
   page-multiple) chunks.  
   */
/* pool for memory */
struct memory_pool
{
	struct lock lock;                   
	struct bitmap *bitmap; 
	uint32_t *addr;                    
};
/* kernel heap page struct */
struct khpage{
	uint16_t page_type;
	uint16_t nalloc;
	uint32_t used_bit[4];
	struct khpage *next;
};

/* free list */
struct freelist{
	struct khpage *list;
	int nfree;
};

static struct khpage *khpage_list;
static struct freelist freelist;
static uint32_t page_alloc_index;

static struct memory_pool kernel_pool;
static struct memory_pool user_pool;
/* Initializes the page allocator. */
//
	void
init_palloc (void) 
{
	/* Calculate the space needed for the khpage list */

	kernel_pool.bitmap = create_bitmap(((uint32_t)USER_POOL_START - (uint32_t)KERNEL_ADDR)/PAGE_SIZE, (void *)KERNEL_ADDR, NULL);
	user_pool.bitmap = create_bitmap(((uint32_t)RKERNEL_HEAP_START -(uint32_t)USER_POOL_START)/PAGE_SIZE, (void *)USER_POOL_START, NULL);

	set_bitmap(kernel_pool.bitmap, 0, 1);
	set_bitmap(user_pool.bitmap, 0, 1);
}


/* Obtains and returns a group of PAGE_CNT contiguous free pages.
   */
	void *
palloc_get_multiple_page (enum palloc_flags flags, size_t page_cnt)
{
	
	size_t kernel_pool_size = bitmap_size(kernel_pool.bitmap);
  	size_t user_pool_size = bitmap_size(user_pool.bitmap);
	void *pages = NULL;

	if(page_cnt == 0)
		return NULL;

	if(flags == 0){
		if((page_alloc_index = (uint32_t)find_set_bitmap(kernel_pool.bitmap, 1, page_cnt, 0)) != BITMAP_ERROR){	// kernel pool bitmap할당
			pages = (void *)((uint32_t)kernel_pool.bitmap + (page_alloc_index * PAGE_SIZE));
		}
	}

	if(flags == 1){
		if((page_alloc_index = (uint32_t)find_set_bitmap(user_pool.bitmap, 1, page_cnt, 0)) != BITMAP_ERROR){	// user pool bitmap 할당
			pages = (void *)((uint32_t)user_pool.bitmap + (page_alloc_index * PAGE_SIZE));
		}
	}

	if(pages != NULL){
		memset(pages, 0, PAGE_SIZE * page_cnt);	// pages 초기화
	}
	return pages;
}

/* Obtains a single free page and returns its address.
   */
	void *
palloc_get_one_page (enum palloc_flags flags) 
{
	return palloc_get_multiple_page (flags, 1);
}

/* Frees the PAGE_CNT pages starting at PAGES. */
	void
palloc_free_multiple_page (void *pages, size_t page_cnt) 
{
	uint32_t index;
	uint32_t addr = (uint32_t)pages;

	if(addr > (uint32_t)USER_POOL_START){	// user pool free
		addr -= (uint32_t)USER_POOL_START;
		index = addr / PAGE_SIZE;
		set_multi_bitmap(user_pool.bitmap, index, page_cnt, 0);	// bitmap 0으로 변경
	}
	else{	// kernel pool free
		addr -= (uint32_t)KERNEL_ADDR;
		index = addr / PAGE_SIZE;
		set_multi_bitmap(kernel_pool.bitmap, index, page_cnt, 0);	// bitmap 0으로 변경
	}

}

/* Frees the page at PAGE. */
	void
palloc_free_one_page (void *page) 
{
	palloc_free_multiple_page (page, 1);
}


void palloc_pf_test(void)
{
	
	uint32_t *one_page1 = palloc_get_one_page(user_area);
	uint32_t *one_page2 = palloc_get_one_page(user_area);
	uint32_t *two_page1 = palloc_get_multiple_page(user_area,2);
	uint32_t *three_page;
	printk("one_page1 = %x\n", one_page1); 
	printk("one_page2 = %x\n", one_page2); 
	printk("two_page1 = %x\n", two_page1);

	printk("=----------------------------------=\n");
	palloc_free_one_page(one_page1);
	palloc_free_one_page(one_page2);
	palloc_free_multiple_page(two_page1,2);

	one_page1 = palloc_get_one_page(user_area);
	one_page2 = palloc_get_one_page(user_area);
	two_page1 = palloc_get_multiple_page(user_area,2);

	printk("one_page1 = %x\n", one_page1);
	printk("one_page2 = %x\n", one_page2);
	printk("two_page1 = %x\n", two_page1);

	printk("=----------------------------------=\n");
	palloc_free_multiple_page(one_page2, 3);
	one_page2 = palloc_get_one_page(user_area);
	three_page = palloc_get_multiple_page(user_area,3);

	printk("one_page1 = %x\n", one_page1);
	printk("one_page2 = %x\n", one_page2);
	printk("three_page = %x\n", three_page);

	palloc_free_one_page(one_page1);
	palloc_free_one_page(three_page);
	three_page = (uint32_t*)((uint32_t)three_page + 0x1000);
	palloc_free_one_page(three_page);
	three_page = (uint32_t*)((uint32_t)three_page + 0x1000);
	palloc_free_one_page(three_page);
	palloc_free_one_page(one_page2);
	
}

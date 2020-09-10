#include <device/io.h>
#include <mem/mm.h>
#include <mem/paging.h>
#include <device/console.h>
#include <proc/proc.h>
#include <interrupt.h>
#include <mem/palloc.h>
#include <ssulib.h>
#include<device/ide.h>

uint32_t *PID0_PAGE_DIR;

intr_handler_func pf_handler;

//모든 함수는 수정이 가능

uint32_t scale_up(uint32_t base, uint32_t size)
{
	uint32_t mask = ~(size-1);
	if(base & mask != 0){
		base = base & mask + size;
	}
	return base;
}
//해당 코드를 사용하지 않고 구현해도 무관함
void pagememcpy(void* from, void* to, uint32_t len)
{
	uint32_t *p1 = (uint32_t*)from;
 	uint32_t *p2 = (uint32_t*)to;
 	int i, e;

 	e = len/sizeof(uint32_t);
 	for(i = 0; i<e; i++)
 		p2[i] = p1[i];

 	e = len%sizeof(uint32_t);
 	if( e != 0)
 	{
 		uint8_t *p3 = (uint8_t*)p1;
 		uint8_t *p4 = (uint8_t*)p2;

 		for(i = 0; i<e; i++)
 			p4[i] = p3[i];
 	}
}

uint32_t scale_down(uint32_t base, uint32_t size)
{
	uint32_t mask = ~(size-1);
	if(base & mask != 0)
		base = base & mask - size;
	return base;
}

void init_paging()
{
	uint32_t *page_dir = palloc_get_one_page(kernel_area);
	uint32_t *page_tbl = palloc_get_one_page(kernel_area);
	//page_dir = VH_TO_RH(page_dir);
	//page_tbl = VH_TO_RH(page_tbl);
	PID0_PAGE_DIR = page_dir;

	int NUM_PT, NUM_PE;
	uint32_t cr0_paging_set;
	int i;

	// mem_size()는 전체 메모리 크기
	NUM_PE = mem_size() / PAGE_SIZE;
	NUM_PT = NUM_PE / 1024;
	//페이지 디렉토리 구성
	page_dir[0] = (uint32_t)page_tbl | PAGE_FLAG_RW | PAGE_FLAG_PRESENT;
	
	NUM_PE = RKERNEL_HEAP_START / PAGE_SIZE;
	//페이지 테이블 구성
	for ( i = 0; i < NUM_PE; i++ ) {
		page_tbl[i] = (PAGE_SIZE * i)
			| PAGE_FLAG_RW
			| PAGE_FLAG_PRESENT;
		//writable & present
	}

	//CR0레지스터 설정
	cr0_paging_set = read_cr0() | CR0_FLAG_PG;		// PG bit set

	//컨트롤 레지스터 저장
	write_cr3( (unsigned)page_dir );		// cr3 레지스터에 PDE 시작주소 저장
	write_cr0( cr0_paging_set );          // PG bit를 설정한 값을 cr0 레지스터에 저장

	reg_handler(14, pf_handler);
}




void memcpy_hard(void* from, void* to, uint32_t len)
{
	write_cr0( read_cr0() & ~CR0_FLAG_PG);
	memcpy(from, to, len);
	write_cr0( read_cr0() | CR0_FLAG_PG);
}

uint32_t* get_cur_pd()
{
	return (uint32_t*)read_cr3();
}

uint32_t pde_idx_addr(uint32_t* addr)
{
	uint32_t ret = ((uint32_t)addr & PAGE_MASK_PDE) >> PAGE_OFFSET_PDE;
	return ret;
}

uint32_t pte_idx_addr(uint32_t* addr)
{
	uint32_t ret = ((uint32_t)addr & PAGE_MASK_PTE) >> PAGE_OFFSET_PTE;
	return ret;
}
//page directory에서 index 위치의 page table 얻기
uint32_t* pt_pde(uint32_t pde)
{
	uint32_t * ret = (uint32_t*)(pde & PAGE_MASK_BASE);
	return ret;
}
//address에서 page table 얻기
uint32_t* pt_addr(uint32_t* addr)
{
	uint32_t idx = pde_idx_addr(addr);
	uint32_t* pd = get_cur_pd();
	return pt_pde(pd[idx]);
}
//address에서 page 주소 얻기
uint32_t* pg_addr(uint32_t* addr)
{
	uint32_t *pt = pt_addr(addr);
	uint32_t idx = pte_idx_addr(addr);
	uint32_t *ret = (uint32_t*)(pt[idx] & PAGE_MASK_BASE);
	return ret;
}

/*
    page table 복사
*/
uint32_t pt_copy(uint32_t *pd, uint32_t *dest_pd, uint32_t idx, uint32_t* start, uint32_t* end, bool share)
{
	uint32_t *pt = pt_pde(pd[idx]);
	uint32_t *new_pt;
	uint32_t size = PAGE_SIZE;
	uint32_t *pte_start = (uint32_t *)scale_down((uint32_t)start, size);
	uint32_t *pte_end = (uint32_t *)scale_up((uint32_t)end, size);

	new_pt = palloc_get_one_page(kernel_area);	// pageg 할당
	dest_pd[idx] = (uint32_t)new_pt | (pd[idx] & ~PAGE_MASK_BASE & ~PAGE_FLAG_ACCESS);	// dest_pd 초기화

	if(start == end)
		return NULL;

	uint32_t start_idx = pte_idx_addr(pte_start);	// entry시작 인덱스
	uint32_t end_idx = pte_idx_addr(pte_end);		// entry끝 인덱스

	if(end_idx == 0 && start != end)	// 예외처리
		end_idx = PAGE_TBL_SIZE/sizeof(uint32_t);
	
	for(int i = start_idx; i < end_idx; i++){	// entry 탐색
		if(pt[i] & PAGE_FLAG_PRESENT){	// PRESENT비트가 설정된 항목 검색
			if(share == true){	// 공유
				new_pt[i] = pt[i];
			}
			else{	// 복사
				uint32_t* tmp = palloc_get_one_page(kernel_area);	// page frame할당
				new_pt[i] = (uint32_t)tmp | (pt[i] & ~PAGE_MASK_BASE & ~PAGE_FLAG_ACCESS);	// new_pt[i] 초기화
				pagememcpy((void*)(pt[i] & PAGE_MASK_BASE), (void*)tmp, PAGE_SIZE);	// pt[i] page frame을 new_pt[i] page frame으로 복사
			}
		}
	}

	return dest_pd[idx];	// 실 주소 반환
}

uint32_t pd_copy(uint32_t* pd, uint32_t* dest_pd, uint32_t idx, uint32_t* start, uint32_t* end, bool share)
{
	uint32_t size = PAGE_SIZE * PAGE_TBL_SIZE/sizeof(uint32_t);
	uint32_t *pde_start = (uint32_t *)scale_down((uint32_t)start, size);
	uint32_t *pde_end = (uint32_t *)scale_up((uint32_t)end, size);
	uint32_t start_idx = pde_idx_addr(pde_start);	// entry시작 인덱스
	uint32_t end_idx = pde_idx_addr(pde_end);	// entry끝 인덱스

	for(int i = start_idx; i < end_idx; i++){	// dir 탐색
		if(pd[i] & PAGE_FLAG_PRESENT){	// PRESENT비트가 설정된 항목 검색
			dest_pd[idx] = pt_copy(pd, dest_pd, i, start, end, share);	// pt_copy 호출
		}
	}

	return dest_pd[idx];
}

uint32_t* pd_create (pid_t pid)
{
	uint32_t *pd = palloc_get_one_page(kernel_area);	// page할당

	pd_copy(PID0_PAGE_DIR, pd, 0, (uint32_t*)0, (uint32_t*)RKERNEL_HEAP_START, true);	// pd_copy호출

	return pd;

}
void pf_handler(struct intr_frame *iframe)
{
	void *fault_addr;

	asm ("movl %%cr2, %0" : "=r" (fault_addr));
#ifdef SCREEN_SCROLL
	refreshScreen();
#endif

	uint32_t pdi, pti;
    uint32_t *pta;
    uint32_t *pda = (uint32_t*)read_cr3();

    pdi = pde_idx_addr(fault_addr);
    pti = pte_idx_addr(fault_addr);

    if(pda == PID0_PAGE_DIR){
        write_cr0( read_cr0() & ~CR0_FLAG_PG);
        pta = pt_pde(pda[pdi]);
        write_cr0( read_cr0() | CR0_FLAG_PG);
    }
    else{
        //pda = RH_TO_VH(pda);
        pta = pt_pde(pda[pdi]);
    }

    if(pta == NULL){
        write_cr0( read_cr0() & ~CR0_FLAG_PG);

        pta = palloc_get_one_page(kernel_area);
        //pta = VH_TO_RH(pta);
        memset(pta,0,PAGE_SIZE);
        
        pda[pdi] = (uint32_t)pta | PAGE_FLAG_RW | PAGE_FLAG_PRESENT;

        //fault_addr = VH_TO_RH(fault_addr);
        pta[pti] = (uint32_t)fault_addr | PAGE_FLAG_RW  | PAGE_FLAG_PRESENT;

        //pta = RH_TO_VH(pta);
        pdi = pde_idx_addr(pta);
        pti = pte_idx_addr(pta);

        uint32_t *tmp_pta = pt_pde(pda[pdi]);
        //tmp_pta[pti] = (uint32_t)VH_TO_RH(pta) | PAGE_FLAG_RW | PAGE_FLAG_PRESENT;

        write_cr0( read_cr0() | CR0_FLAG_PG);
    }
    else{
        //pta = RH_TO_VH(pta);
        //fault_addr = VH_TO_RH(fault_addr);
        pta[pti] = (uint32_t)fault_addr | PAGE_FLAG_RW  | PAGE_FLAG_PRESENT;
    }
}

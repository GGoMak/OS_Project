# OS_Project_6 : Virtual Memory

  - 본 과제는 기존에 구현된 메모리 구조와 page frame 구조를 변경하여 새로운 page allocator를 구현하는 것이다.
  
## 구현

  - Kernel pool과 user pool의 구현
  - void* palloc_get_multiple_page(enum palloc_flags flags, sizt_t page_cnt) 함수 구현
  - 프로세스 생성 시 page directory 및 page table 생성 수정 구현
    - pd_copy(), pt_copy() 수정
  - page frame 삭제 기능 구현
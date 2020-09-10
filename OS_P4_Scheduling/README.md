# OS_Project_4 : Scheduling

  - 본 과제는 FreeBSD 5.4 이후 구현된 실제 스케줄링 기법인 ULE(non-interacitve)를 구현한다.
  
## 구현

  - 스케줄링에 사용되는 자료구조 및 변수 구현
    - priority, time slice, time used, time sleep
    - runq 리스크 구현
  - priority 값 재계산
  - schedule() 수정
  - get_next_proc() 함수 구현
  - 실행 될 프로세스 내용 구현

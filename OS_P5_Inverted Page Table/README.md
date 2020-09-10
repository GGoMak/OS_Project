# OS_Project_5 : Inverted Page Table

  - 본 과제는 메모리 사용량이 늘어남에 따라 디스크의 일부를 마치 확장된 RAM처럼 사용할 수 있게 해주는 기법인 가상 메모리의 Page Allocator를 이해하고 메모리 쓰기 성능 향상을 위한 해싱 기법이며 Top-level과 Bottom_level의 2-level hash table로 구성된 Level Hash를 구현한다.
  - 또한 많은 프로세스를 생성하면 페이지 테이블이 차지하는 메모리의 양이 많아지는 것을 해결하기 위한 방법인 역페이지 테이블을 구현한다.
  
## 구현

  - Level Hash 구현
    - Level Hash의 삽입, 삭제 구현
  - Level Hash를 사용한 Inverted Page Table 구현
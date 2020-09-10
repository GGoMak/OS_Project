# OS_Project_3 : System Call

  - 본 과제는 시스템 콜의 이해를 위하여 기존 open 시스템 콜 함수에 O_APPEND, O_TRUNC 기능 추가와 fcntl 시스템 콜 구현 및 추가하는 것이다.
  
## 구현

  - open() 시스템 콜 O_APPEND, O_TRUNC 플래그 구현
  - fcntl() 시스템 콜 구현
    - F_DUPFD, F_GETFL, F_SETFL 플래그 구현
# Operating System Project

  - 3학년 2학기 OS과목 프로젝트입니다.
  - 해당 OS코드의 저작권은 숭실대학교 OSlab에 있습니다.
  - 각 프로젝트에 대한 설명은 폴더내의 README파일과 명세서를 참조하세요.
  
## Summary

  - OS_P1_Assembly : Assembly에 대한 이해
  - OS_P2_Bootstrap : 부트로더 이해 및 분석
  - OS_P3_System Call : System call 이해
  - OS_P4_Scheduling : 스케줄링 기법 구현
  - OS_P5_Inverted Page Table : Level Hash 및 역페이지 테이블 구현
  - OS_P6_Virtual Memory : 가상메모리 Page Allocator 구현

## Usage

  - libxrandr-dev 및 g++설치
```
sudo apt-get install libxrandr-dev g++
```
  - bochs 환경설정 및 설치
```
tar -xvf {다운로드한 압축된 bochs 파일}
cd {압축이 풀린 bochs폴더 경로}
./configure --with-nogui --with-x11 --enable-gdb-stub
sudo make
sudo make install
```
  
  - nasm 설치
```
sudo apt-get install nasm
```

  - 소스코드 컴파일 및 실행
```
cd OS_P1_Assembly
make
make run
```

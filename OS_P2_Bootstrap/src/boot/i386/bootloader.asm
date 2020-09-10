org	0x7c00   

[BITS 16]

START:  
	mov		ax, 0xb800		; 비디오 메모리 주소 설정
	mov		es, ax			; es에 ax복사
	mov		ax, 0			; ax 초기화
	mov		cx, 80*25*2		; cs에 80*25*2(가로80*세로25)
	mov		si, 0			; si 초기화
	mov		ax, 1			
	mov		[partition_num], ax	; partition_num 1로 초기화

CLS:	; 스크린 초기화
	mov		byte [es:si], 0			; 0입력
	mov		byte [es:si+1], 0x07	; gray속성
	
	add		si, 2
	
	cmp		si, 80*25*2		; 80*25 만큼 반복
	jl		CLS

	mov		si, ssuos_1		; si에 문자열 설정
	mov		di, 0			; 출력 위치 설정
	call	print_string	; ssuos_1 출력

	mov		si, ssuos_2		; si에 문자열 설정
	mov		di, 30			; 출력 위치 설정
	call	print_string	; ssuos_2 출력

	mov		si, ssuos_3		; si에 문자열 설정
	mov		di, 160			; 출력 위치 설정
	call	print_string	; ssuos_3 출력

	call	print_select	; [o] 출력

	call	chose			; 키보드 인터럽트 실행

print_string:	;	ssuos 출력

	mov		cl, byte[si]
	cmp		cl, 0
	je		.endFunc
		
	mov		byte [es:di], cl	; 문자 출력
	add		si, 1				; 다음문자 이동
	add		di, 2				; 출력할 칸 이동
	jmp		print_string

	.endFunc	; 함수 종료
		ret

print_select:	;	[o]출력

	mov		si, select	; [o] 문자열 가져옴

	mov		ax, 1
	mov		di, 0
	cmp		[partition_num], ax	; 1번 선택
	je		print_string		; di위치에 [0]출력

	mov		ax, 2
	mov		di, 30
	cmp		[partition_num], ax	; 2번 선택
	je		print_string		; di위치에 [0]출력

	mov		ax, 3
	mov		di, 160
	cmp		[partition_num], ax	; 3번 선택
	je		print_string		; di위치에 [0]출력

	ret

chose:	;	os 선택 인터럽트
		
	mov		ah, 00h
	int		16h

	cmp		ah, 4Dh	; right
	je		right

	cmp		ah, 4Bh	; left
	je		left

	cmp		ah, 48h	; up
	je		up
	
	cmp		ah, 50h	; down
	je		down

	cmp		ah, 0x1c	; enter
	je		BOOT1_LOAD
	
	jmp		CLS	
	
right:					
	mov		bx, 2
	cmp		[partition_num], bx
	jae		CLS		; 2또는 3에 위치해 있을 경우 무시

	mov		[partition_num], bx	; 2입력
	jmp		CLS		; 2로 이동

left:
	mov		bx,	1
	cmp		[partition_num], bx
	je		CLS		; 1에 위치해 있을 경우 무시

	mov		bx, 3
	cmp		[partition_num], bx
	je		CLS		; 3에 위치해 있을 경우 무시
	
	mov		bx, 1
	mov		[partition_num], bx	; 1입력
	jmp		CLS		; 1로 이동

up:
	mov		bx, 2
	cmp		[partition_num], bx
	jbe		CLS		; 1또는 2에 있을 경우 무시

	mov		bx, 1 
	mov		[partition_num], bx	; 1입력
	jmp		CLS		; 1로 이동

down:
	mov		bx, 2
	cmp		[partition_num], bx
	jae		CLS		; 2또는 3에 있을 경우 무시
	
	mov		bx, 3
	mov		[partition_num], bx	; 3입력
	jmp		CLS		; 3으로 이동

BOOT1_LOAD:
	mov     ax, 0x0900 
    mov     es, ax
    mov     bx, 0x0

	mov     ah, 2		; 섹터읽기
    mov     al, 0x4		; 섹터수 지정
    mov     ch, 0		; 실린더 번호
    mov     cl, 2		; 읽기 시작할 섹터 번호
    mov     dh, 0		; 읽기 시작할 헤드 번호
    mov     dl, 0x80	; 드라이브 번호

    int     0x13		; 인터럽트 호출
    jc      BOOT1_LOAD	; Carry 플래그 발생 시 다시시도

KERNEL_LOAD:

	mov		ax,	1 
	cmp		[partition_num], ax	; 선택된 번호가 1이면 ssuos_1부팅
	je		load1

	mov		ax, 2 
	cmp		[partition_num], ax	; 선택된 번호가 2이면 ssuos_2부팅
	je		load2

	mov		ax, 3
	cmp		[partition_num], ax	; 선택된 번호가 3이면 ssuos_3부팅
	je		load3

	load1:
		mov     ax, 0x1000	
        mov     es, ax		
        mov     bx, 0x0		

        mov     ah, 2		; 섹터읽기
        mov     al, 0x3f	; 섹터 수 지정
        mov     ch, 0		; 실린더 번호
        mov     cl, 0x6		; 섹터 번호
        mov     dh, 0		; 헤드 번호
        mov     dl, 0x80	; 드라이브 번호

        int     0x13		; 인터럽트 호출
        jc      KERNEL_LOAD	; Carry 플래그 발생시 다시시도
		jmp		end

	load2:
		mov     ax, 0x1000
        mov     es, ax		
        mov     bx, 0x0

        mov     ah, 2		; 섹터읽기
        mov     al, 0x3f	; 섹터 수 지정
        mov     ch, 9		; 실린더 번호
        mov     cl, 47		; 섹터 버호
        mov     dh, 14		; 헤드 번호
        mov     dl, 0x80	; 드라이브 번호

        int     0x13		; 인터럽트 호출
        jc      KERNEL_LOAD	; Carry 플래그 발생 시 다시시도
		jmp		end

	load3:
		mov     ax, 0x1000	
        mov     es, ax		
        mov     bx, 0x0		

        mov     ah, 2		; 섹터 읽기
        mov     al, 0x3f	; 섹터 수 지정
        mov     ch, 14		; 실런더 번호
        mov     cl, 7		; 섹터 번호
        mov     dh, 14		; 헤드 번호
        mov     dl, 0x80	; 드라이브 번호

        int     0x13		; 인터럽트 호출
        jc      KERNEL_LOAD	; Carry 플래그 발생 시 다시시도
		jmp		end
	

end:
	jmp		0x0900:0x0000

select db "[O]",0
ssuos_1 db "[ ] SSUOS_1",0
ssuos_2 db "[ ] SSUOS_2",0
ssuos_3 db "[ ] SSUOS_3",0
ssuos_4 db "[ ] SSUOS_4",0
partition_num : resw 1

times   446-($-$$) db 0x00

PTE:
partition1 db 0x80, 0x00, 0x00, 0x00, 0x83, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x3f, 0x0, 0x00, 0x00
partition2 db 0x80, 0x00, 0x00, 0x00, 0x83, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x3f, 0x0, 0x00, 0x00
partition3 db 0x80, 0x00, 0x00, 0x00, 0x83, 0x00, 0x00, 0x00, 0x98, 0x3a, 0x00, 0x00, 0x3f, 0x0, 0x00, 0x00
partition4 db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
times 	510-($-$$) db 0x00
dw	0xaa55

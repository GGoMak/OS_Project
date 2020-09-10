org	0x9000  

[BITS 16]  

		cli		; Clear Interrupt Flag

		mov     ax, 0xb800	; ax에 0xb800대입
        mov     es, ax		; es에 ax대입
        mov     ax, 0x00	; ax 초기화
        mov     bx, 0		; bx 초기화
        mov     cx, 80*25*2	; cs에 80*25*2대입(가로80*세로25*2)
CLS:
        mov     [es:bx], ax	; 0씀
        add     bx, 1		; bx + 1
        loop    CLS			; cx가 0이 될때까지 loop
 
Initialize_PIC:
		;ICW1 - 두 개의 PIC를 초기화 
		mov		al, 0x11	; ICW1 저장
		out		0x20, al	; PICM로 출력
		out		0xa0, al	; PICS로 출력

		;ICW2 - 발생된 인터럽트 번호에 얼마를 더할지 결정
		mov		al, 0x20	; PICM 저장
		out		0x21, al	; 0x21로 출력
		mov		al, 0x28	; al에 0x28복사 
		out		0xa1, al	; PICS로 출력

		;ICW3 - 마스터/슬레이브 연결 핀 정보 전달
		mov		al, 0x04	; 0x04저장 
		out		0x21, al	; IRQ2
		mov		al, 0x02	; 0x02저장
		out		0xa1, al	; IRQ2

		;ICW4 - 기타 옵션 
		mov		al, 0x01	; ICW4저장
		out		0x21, al	; PICM로 출력
		out		0xa1, al	; PICS로 출력

		mov		al, 0xFF	; al에 0xff복사
		;out		0x21, al
		out		0xa1, al	; 

Initialize_Serial_port:
		xor		ax, ax		; ax 초기화
		xor		dx, dx		; dx 초기화
		mov		al, 0xe3	; al에 0xe3 복사
		int		0x14		; 인터럽트 발생

READY_TO_PRINT:
		xor		si, si				; si 초기화
		xor		bh, bh				; bh 초기화
PRINT_TO_SERIAL:					; 시리얼 출력
		mov		al, [msgRMode+si]	; al에 msgRMode의 si번째 문자 복사
		mov		ah, 0x01			; 속성 설정
		int		0x14				; 인터럽트 발생
		add		si, 1				; si + 1
		cmp		al, 0				; 복사된 문자가 없으면
		jne		PRINT_TO_SERIAL		; PRINT_TO_SERIAL로 점프
PRINT_NEW_LINE:						; 라인 출력
		mov		al, 0x0a			; al에 0x0a 복사
		mov		ah, 0x01			; ah에 0x01 복사
		int		0x14				; 직렬 포트 인터럽트 발생
		mov		al, 0x0d			; al에 0x0d 복사
		mov		ah, 0x01			; ah에 0x01 복사
		int		0x14				; 직렬 포트 인터럽트 발생

; OS assignment 2
; add your code here
; print current date to boch display




Activate_A20Gate:					; a20gate 활성화
		mov		ax,	0x2401			; ax에 0x2401 복사
		int		0x15				; 기타 시스템 인터럽트

;Detecting_Memory:
;		mov		ax, 0xe801
;		int		0x15

PROTECTED:
        xor		ax, ax				; ax 초기화
        mov		ds, ax              ; dx에 ax복사

		call	SETUP_GDT			; SETUP_GDT호출

        mov		eax, cr0			; 컨트롤 레지스터 설정
        or		eax, 1				
        mov		cr0, eax			; 컨트롤 레지스터 설정 

		jmp		$+2					; 현재주소 + 2
		nop							
		nop
		jmp		CODEDESCRIPTOR:ENTRY32	; CODEDESCRIPTOR:ENTRY32로 점프

SETUP_GDT:							; 
		lgdt	[GDT_DESC]			; GDT_DESC에 GDT의 위치를 저장
		ret							; 종료

[BITS 32]  

ENTRY32:
		mov		ax, 0x10			; ax에 0x10복사
		mov		ds, ax				; ds에 ax복사
		mov		es, ax				; es에 ax복사
		mov		fs, ax				; fs에 ax복사
		mov		gs, ax				; gs에 ax복사

		mov		ss, ax				; ss에 ax복사
  		mov		esp, 0xFFFE			; 스택포인터 설정
		mov		ebp, 0xFFFE			; 베이스 스택포인터 설정

		mov		edi, 80*2			; edi에 80*2 복사
		lea		esi, [msgPMode]		; esi에 msgPMode문자열의 주소 복사
		call	PRINT				; PRINT 호출

		;IDT TABLE
	    cld							; direction flag 0으로 초기화
		mov		ax,	IDTDESCRIPTOR	; ax에 IDTDESCRIPTOR 주소 복사
		mov		es, ax				; es에 ax복사
		xor		eax, eax			; eax 초기화
		xor		ecx, ecx			; ecs 초기화
		mov		ax, 256				; ax에 256복사
		mov		edi, 0				; edi에 0복사
 
IDT_LOOP:
		lea		esi, [IDT_IGNORE]	; IDT_IGNORE의 주소값을 esi에 복사
		mov		cx, 8				; cx에 8복사
		rep		movsb				; 스트림 명령어 반복(
		dec		ax					; ax--
		jnz		IDT_LOOP			; ax가 아니면 반복

		lidt	[IDTR]				

		sti							; 인터럽트 플래그 초기화
		jmp	CODEDESCRIPTOR:0x10000	; 점프

PRINT:
		push	eax					; eax push
		push	ebx					; ebx push
		push	edx					; edx push
		push	es					; es push
		mov		ax, VIDEODESCRIPTOR	; ax에 VIDIODESCRIPTOR복사
		mov		es, ax				; es에 ax복사
PRINT_LOOP:
		or		al, al				; al 비교
		jz		PRINT_END			; 0이면 PRINT_END로 점프
		mov		al, byte[esi]		; al에 esi복사
		mov		byte [es:edi], al	; al출력
		inc		edi					; edi++
		mov		byte [es:edi], 0x07	; 속성 설정

OUT_TO_SERIAL:
		mov		bl, al				; bl에 al복사
		mov		dx, 0x3fd			; dx에 0x3fd복사
CHECK_LINE_STATUS:
		in		al, dx				; dx를 al로 입력
		and		al, 0x20			; al, 0x20비교
		cmp		al, 0				
		jz		CHECK_LINE_STATUS	; al이 0이면 CHECK_LINE_STATUS로 점프
		mov		dx, 0x3f8			; dx에 0x3f8복사
		mov		al, bl				; al에 bl복사
		out		dx, al				; al을 dx로 출력

		inc		esi					; esi++
		inc		edi					; edi++
		jmp		PRINT_LOOP			; PRINT_LOOP로 점프
PRINT_END:
LINE_FEED:
		mov		dx, 0x3fd			; dx에 0x3fd복사
		in		al, dx				; dx를 al로 입력
		and		al, 0x20			
		cmp		al, 0
		jz		LINE_FEED			; al이 0x20이 아니면 루프
		mov		dx, 0x3f8			; dx에 0x3f8복사
		mov		al, 0x0a			; al에 0x0a복사
		out		dx, al				; al을 dx로 출력
CARRIAGE_RETURN:
		mov		dx, 0x3fd			; dx에 0x3fd복사
		in		al, dx				; dx를 al로 입력
		and		al, 0x20
		cmp		al, 0
		jz		CARRIAGE_RETURN		; al이 0x20이 아니면 루프
		mov		dx, 0x3f8			; dx에 0x3f8복사
		mov		al, 0x0d			; al에 0x0d복사
		out		dx, al				; al을 dx로 출력

		pop		es					; es pop 
		pop		edx					; edx pop
		pop		ebx					; ebx pop
		pop		eax					; eax pop
		ret							; 종료

GDT_DESC:
        dw GDT_END - GDT - 1		; gdt limit설정
        dd GDT						; 시작설정
GDT:
		NULLDESCRIPTOR equ 0x00		; NULLDESCRIPTOR 정의
			dw 0 
			dw 0 
			db 0 
			db 0 
			db 0 
			db 0
		CODEDESCRIPTOR  equ 0x08	; CODEDESCRIPTOR 정의
			dw 0xffff             
			dw 0x0000              
			db 0x00                
			db 0x9a                    
			db 0xcf                
			db 0x00                
		DATADESCRIPTOR  equ 0x10	; DATADESCRIPTOR 정의
			dw 0xffff              
			dw 0x0000              
			db 0x00                
			db 0x92                
			db 0xcf                
			db 0x00                
		VIDEODESCRIPTOR equ 0x18	; VIDEODESCRIPTOR 정의
			dw 0xffff              
			dw 0x8000              
			db 0x0b                
			db 0x92                
			db 0x40                    
			;db 0xcf                    
			db 0x00                 
		IDTDESCRIPTOR	equ 0x20	; IDTDESCRIPTOR 정의
			dw 0xffff
			dw 0x0000
			db 0x02
			db 0x92
			db 0xcf
			db 0x00
GDT_END:
IDTR:
		dw 256*8-1
		dd 0x00020000
IDT_IGNORE:
		dw ISR_IGNORE
		dw CODEDESCRIPTOR
		db 0
		db 0x8E
		dw 0x0000
ISR_IGNORE:
		push	gs	; gs push
		push	fs	; fs push
		push	es	; es push
		push	ds	; ds push
		pushad	; 레지스터 push
		pushfd	; 플래그 저장
		cli		; 인터럽트 플래그 clr
		nop	
		sti		; 인터럽트 플래그 설정
		popfd	; 플래그 pop
		popad	; 레지스터 pop
		pop		ds	; ds pop
		pop		es	; es pop
		pop		fs	; fs pop
		pop		gs	; gs pop
		iret	; 처리 완료 후 테스크로 복귀



msgRMode db "Real Mode", 0
msgPMode db "Protected Mode", 0

 
times 	2048-($-$$) db 0x00

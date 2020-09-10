[BITS 16]
[ORG 0x7c00]

START:   
	mov		ax, 0xb800	; 비디오 메모리 주소 설정
	mov		es, ax		; es에 ax 복사
	mov		ax, 0x00	; ax 초기화
	mov		bx, 0		; bx 초기화
	mov		cx, 80*25*2	; cx에 80*25(가로80 * 세로 25 * 2)

CLS:
	mov		byte [es:si], 0			; 0x00 출력
	mov		byte [es:si+1], 0x07	; 속성 설정
	add		si, 2					; s += 2
	cmp		si, cx					
	jl		CLS						; si가 80*25*2일때까지 반복

	mov		si, msg					; msg의 주소 저장
	mov		di, 0						

PRINT:
	mov		cl, byte [si]			; msg의 si번째 문자 cl에 복사
	cmp		cl, 0					; 복사된 문자가 없으면 종료
	je		FIN	

	mov		ah, 0x07				; 속성 설정
	mov		byte [es:di], cl		; cl 출력
	add		si, 1
	add		di, 2
	jmp		PRINT					; 반복

FIN:
	ret								; 종료


msg db "Hello, JISU's World", 0		; 문자열

times 510-($-$$) db 0x00
dw 0xaa55

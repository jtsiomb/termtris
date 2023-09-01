	bits 16
	cpu 8086

segment _TEXT class=CODE

	global ansi_reset_
	global ansi_clearscr_
ansi_reset_:
ansi_clearscr_:
	mov ax, 0f00h
	int 10h
	xor ah, ah
	int 10h
	ret


	align 2
curpos dw 0

	global ansi_cursor_
ansi_cursor_:
	push bx
	push cx
	test ax, ax
	jnz .show
	; save current cursor position
	mov ax, 300h
	xor bx, bx
	int 10h
	mov [curpos], dx
	; hide by moving off-screen
	xor dx, 2600h
	jmp .setcur
.show:
	mov dx, [curpos]
.setcur:
	mov ax, 200h
	int 10h
	pop cx
	pop bx
	ret
	

	global ansi_setcursor_
ansi_setcursor_:
	push bx
	mov dh, al	; row (col is already in dl)
	xor bx, bx
	mov ax, 200h
	int 10h
	pop bx
	ret


	align 2
attr dw 0700h

	global ansi_setcolor_
ansi_setcolor_:
	shl ax, 1
	shl ax, 1
	shl ax, 1
	shl ax, 1
	or ax, dx
	mov [attr], al
	ret

advcursor:
	mov ax, 300h
	xor bx, bx
	int 10h
	inc dx
	mov ax, 200h
	int 10h
	ret
	
	global ansi_ibmchar_
ansi_ibmchar_:
	push bx
	push cx
	mov ah, 9
	xor bx, bx
	rol dl, 1
	rol dl, 1
	rol dl, 1
	rol dl, 1
	mov bl, dl
	mov cx, 1
	int 10h
	call advcursor
	pop cx
	pop bx
	ret
	
	; vi:ft=nasm ts=8 sts=8 sw=8:

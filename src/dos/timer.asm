	bits 16
	cpu 8086

segment _TEXT class=CODE

	global init_timer_
init_timer_:
	push bx
	push dx
	push es
	mov ax, 351ch
	int 21h
	mov [prev_isr], bx
	mov [prev_isr+2], es

	mov ax, 251ch
	mov dx, timer_isr	; ds is already equal to cs
	int 21h
	pop es
	pop dx
	pop bx
	ret

	align 4
prev_isr dd 0

	global cleanup_timer_
cleanup_timer_:
	push dx
	push ds
	mov dx, [prev_isr]
	mov ds, [prev_isr+2]
	mov ax, 251ch
	int 21h
	pop ds
	pop dx
	ret
	
	extern _timer_ticks
	
timer_isr:
	inc word [cs:_timer_ticks]
	jnz .done
	inc word [cs:_timer_ticks+2]
.done:	iret
	
	; vi:ft=nasm ts=8 sts=8 sw=8:

; Termtris - a tetris game for ANSI/VT100 terminals
; Copyright (C) 2019-2023  John Tsiombikas <nuclear@member.fsf.org>
; 
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
; 
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
; 
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <https://www.gnu.org/licenses/>.

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

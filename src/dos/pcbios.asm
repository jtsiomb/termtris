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

%include "video.inc"

	extern _monochrome
	extern _use_gfxchar
	extern vidtype

	extern _term_type
	extern _term_reset
	extern _term_clearscr
	extern _term_setcursor
	extern _term_cursor
	extern _term_setcolor
	extern _term_ibmchar

	global pcbios_init_
pcbios_init_:
	mov word [_term_type], 0xb105
	mov word [_term_type + 2], 0
	mov word [_term_reset], pcbios_reset_
	mov word [_term_clearscr], pcbios_clearscr_
	mov word [_term_setcursor], pcbios_setcursor_
	mov word [_term_cursor], pcbios_cursor_
	mov word [_term_setcolor], pcbios_setcolor_
	mov word [_term_ibmchar], pcbios_ibmchar_

	mov ax, [_use_gfxchar]
	test ax, ax
	jz .end

	push bx
	push cx
	push dx
	push bp
	push es

	mov ax, cs
	mov es, ax
	cmp word [vidtype], VIDTYPE_VGA
	jz .vga
	mov bp, glyphs_ega
	mov bx, 0e00h	; 14 bytes per char, load block 0
	jmp .skipvga
.vga:	mov bp, glyphs_vga
	mov bx, 1000h	; 16 bytes per char, load block 0
.skipvga:
	mov cx, 3	; load 3 chars: [ \ ]
	mov dx, 0dbh	; load at graphics range to dup last column (set bit 7)
	mov ax, 1110h	; user alpha load
	int 10h

	pop es
	pop bp
	pop dx
	pop cx
	pop bx
.end:	ret

	global pcbios_reset_
pcbios_reset_:
	mov ax, 0f00h
	int 10h
	xor ah, ah
	int 10h
	ret

	global pcbios_clearscr_
pcbios_clearscr_:
	call pcbios_reset_
	jmp pcbios_init_

	global pcbios_cursor_
pcbios_cursor_:
	push bx
	push cx

	push ax
	mov ax, 300h
	xor bx, bx
	int 10h
	and ch, 0dfh
	pop ax

	test ax, ax
	jnz .skiphide
	or ch, 20h
.skiphide:
	mov ax, 100h
	int 10h
	pop cx
	pop bx
	ret


	global pcbios_setcursor_
pcbios_setcursor_:
	push bx
	mov dh, al	; row (col is already in dl)
	xor bx, bx
	mov ax, 200h
	int 10h
	pop bx
	ret


	align 2
attr dw 0700h

	global pcbios_setcolor_
pcbios_setcolor_:
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


	global pcbios_ibmchar_
pcbios_ibmchar_:
	push bx
	push cx
	; if it's [ or ], set bit 7 to use the graphic blocks
	cmp word [_use_gfxchar], 0
	jz .noremap
	cmp al, 5bh
	jz .remap
	cmp al, 5dh
	jnz .noremap
.remap: or al, 80h
.noremap:
	mov ah, 9
	mov bx, 7
	cmp word [_monochrome], 0
	jnz .skipattr
	rol dl, 1
	rol dl, 1
	rol dl, 1
	rol dl, 1
	mov bl, dl
.skipattr:
	mov cx, 1
	int 10h
	call advcursor
	pop cx
	pop bx
	ret

glyphs_vga:
	db 0xff
	db 0xff
	db 0xf5
	db 0xda
	db 0xd6
	db 0xeb
	db 0xda
	db 0xed
	db 0xeb
	db 0xf5
	db 0xd6
	db 0xeb
	db 0xed
	db 0xd5
	db 0xe0
	db 0x08

	times 16 db 0

	db 0xff
	db 0xfe
	db 0x6a
	db 0xb4
	db 0xd6
	db 0x5a
	db 0xea
	db 0x54
	db 0xae
	db 0xd4
	db 0xba
	db 0x56
	db 0xea
	db 0x5a
	db 0x00
	db 0x84

glyphs_ega:
	db 0xff
	db 0xff
	db 0xf5
	db 0xd6
	db 0xeb
	db 0xda
	db 0xed
	db 0xeb
	db 0xf5
	db 0xd6
	db 0xed
	db 0xd5
	db 0xe0
	db 0x08

	times 14 db 0

	db 0xff
	db 0xfe
	db 0x6a
	db 0xd6
	db 0x5a
	db 0xea
	db 0x54
	db 0xae
	db 0xd4
	db 0xba
	db 0xea
	db 0x5a
	db 0x00
	db 0x84

	; vi:ft=nasm ts=8 sts=8 sw=8:

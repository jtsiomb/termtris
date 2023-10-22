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

	extern _monochrome
	extern _use_gfxchar

%include "video.inc"

	global detect_video_
detect_video_:
	push bx
	; try the VGA detect call first
	mov ax, 1a00h
	int 10h
	cmp al, 1ah
	jnz .skip_vgainfo	; function not available
	cmp bl, 0ffh
	jz .skip_vgainfo	; unknown display type
	cmp bl, 1
	jnz .nomda
	mov word [vidtype], VIDTYPE_MDA
	mov word [_monochrome], 1
	jmp .done
.nomda:	cmp bl, 4
	jae .nocga
	mov word [vidtype], VIDTYPE_CGA
	mov word [_monochrome], 0
	jmp .done
.nocga:	; for ega/vga first determine monochrome/color display
	mov ax, bx
	and ax, 1
	mov [_monochrome], ax
	cmp bl, 7
	jae .noega
	mov word [vidtype], VIDTYPE_EGA
	jmp .done
.noega:	mov word [vidtype], VIDTYPE_VGA
	jmp .done

.skip_vgainfo:
	; try get ega info
	mov ah, 12h
	mov bx, 0ff10h
	int 10h
	cmp bh, 0ffh
	jz .skip_egainfo
	mov word [vidtype], VIDTYPE_EGA
	test bh, bh
	jnz .ega_mono
	mov word [_monochrome], 0
	jmp .done
.ega_mono:
	mov word [_monochrome], 1
	jmp .done

.skip_egainfo:
	; try int 11h (get equipment list)
	int 11h
	and ax, 30h
	cmp ax, 30h
	jz .mda
	mov word [vidtype], VIDTYPE_CGA
	mov word [_monochrome], 0
	jmp .done
.mda:	mov word [vidtype], VIDTYPE_MDA
	mov word [_monochrome], 1

.done:	cmp word [vidtype], VIDTYPE_EGA
	jb .end
	mov word [_use_gfxchar], 1
.end:	pop bx
	ret

	align 2
	global vidtype
vidtype dw 0

	; vi:ft=nasm ts=8 sts=8 sw=8:

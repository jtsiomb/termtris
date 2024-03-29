!ifdef __UNIX__
obj = src/dos/main.obj src/dos/timer.obj src/dos/video.obj src/dos/pcbios.obj &
	src/dos/scoredb.obj src/game.obj src/term.obj src/ansi.obj &
	src/vt52.obj src/adm3.obj
inc = -Isrc -Isrc/dos
asminc = -i src/dos/
!else
obj = src\dos\main.obj src\dos\timer.obj src\dos\video.obj src\dos\pcbios.obj &
	src\dos\scoredb.obj src\game.obj src\term.obj src\ansi.obj &
	src\vt52.obj src\adm3.obj
inc = -Isrc -Isrc\dos
asminc = -i src\dos\
!endif

bin = termtris.com

#opt = -otexan
#opt = -od
dbg = -d3

AS = nasm
CC = wcc
LD = wlink
ASFLAGS = -fobj $(asminc)
CFLAGS = $(dbg) $(opt) $(inc) $(def) -ms -s -zq -bt=com $(incpath)
LDFLAGS = option map $(libpath)

$(bin): $(obj)
	%write objects.lnk $(obj)
	%write ldflags.lnk $(LDFLAGS)
	$(LD) name $@ system com file { @objects } @ldflags

#	$(LD) debug all name $@ system com file { @objects } @ldflags

.c: src;src/dos
.asm: src;src/dos

.c.obj: .autodepend
	$(CC) -fo=$@ $(CFLAGS) $[*

.asm.obj:
	$(AS) $(ASFLAGS) -o $@ $[*.asm

!ifdef __UNIX__
clean: .symbolic
	rm -f $(obj)
	rm -f $(bin)
	rm -f *.lnk
	rm -f termtris.map
!else
clean: .symbolic
	del src\*.obj
	del src\dos\*.obj
	del $(bin)
	del *.lnk
	del termtris.map
!endif

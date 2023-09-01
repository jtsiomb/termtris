!ifdef __UNIX__
obj = src/dos/main.obj src/dos/timer.obj src/dos/ansi.obj src/game.obj src/scoredb.obj
inc = -Isrc -Isrc/dos
!else
obj = src\dos\main.obj src\dos\timer.obj src\dos\ansi.obj src\game.obj src\scoredb.obj
inc = -Isrc -Isrc\dos
!endif

bin = termtris.com

#opt = -otexan
#opt = -od

AS = nasm
CC = wcc
LD = wlink
ASFLAGS = -fobj
CFLAGS = -d3 $(opt) $(inc) $(def) -ms -s -zq -bt=com $(incpath)
LDFLAGS = option map $(libpath)

$(bin): $(obj)
	%write objects.lnk $(obj)
	%write ldflags.lnk $(LDFLAGS)
	$(LD) debug all name $@ system com file { @objects } @ldflags

.c: src;src/dos
.asm: src;src/dos

.c.obj: .autodepend
	$(CC) -fo=$@ $(CFLAGS) $[*

.asm.obj:
	nasm -f obj -o $@ $[*.asm

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

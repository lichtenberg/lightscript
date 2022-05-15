

OBJS = lsmain.o lightscript.tab.o lightscript.yy.o symtab.o parsefuncs.o lsplayback.o musicplayer.o

CFLAGS = -target x86_64-apple-macos10.13
#CFLAGS =

%.o : %.c
	clang $(CFLAGS) -c -o $@ $<

%.o : %.mm
	clang $(CFLAGS) -c -o $@ $<

lightscript : $(OBJS)
	clang $(CFLAGS) -o $@ $(OBJS) -framework Foundation -framework AVFoundation
	codesign -s mlichtenberg@me.com lightscript

lightscript.yy.c : lightscript.lex lightscript.tab.h
	flex -DECHO -o lightscript.yy.c lightscript.lex

lightscript.tab.c lightscript.tab.h : lightscript.y
	bison -d lightscript.y

lsmain.c : lightscript.h

symtab.c : lightscript.h

parsefuncs.c : lightscript.h

lsplayback.c : lightscript.h

clean :
	rm -f lightscript $(OBJS) lightscript.yy.c lightscript.tab.c lightscript.tab.h

zip :
	zip ../lightscript$(shell date "+%Y%m%d") lightscript *.cfg

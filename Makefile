

OBJS = lsmain.o lightscript.tab.o lightscript.yy.o symtab.o parsefuncs.o lsplayback.o musicplayer.o

%.o : %.c
	clang -c -o $@ $<

%.o : %.mm
	clang -c -o $@ $<

lightscript : $(OBJS)
	clang -o $@ $(OBJS) -framework Foundation -framework AVFoundation

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
	zip ../lightscript$(shell date "+%Y%m%d") lightscript *.ls *.cfg

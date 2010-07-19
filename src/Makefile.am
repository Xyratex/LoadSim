AM_YFLAGS = -d 
AM_CFLAGS = @CFLAGS@ -I ../include -DYYERROR_VERBOSE=1 -DDEBUG=1

noinst_PROGRAMS=mdsim

mdsim_SOURCES = parse-gram.y parse-lex.l md-sim.c vm_compile.c  encode.c md_client.c kernel.c
CLEAN_FILES= parse-gram.c parse-gram.h parse-lex.c
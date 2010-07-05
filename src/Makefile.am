AM_YFLAGS = -d 
AM_CFLAGS = @CFLAGS@ -I ../include -DYYERROR_VERBOSE=1 

noinst_PROGRAMS=mdsim

mdsim_SOURCES = parse-gram.y parse-lex.l md-sim.c vm_compile.c
CLEAN_FILES= parse-gram.c parse-gram.h parse-lex.c
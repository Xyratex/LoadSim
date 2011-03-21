AM_YFLAGS = -d 
AM_CFLAGS = -I ../include -D YYERROR_VERBOSE=1
AM_LFLAGS = @LEX_FL@

sbin_PROGRAMS=mdsim

mdsim_SOURCES = parse-gram.y parse-lex.l md-sim.c vm_compile.c encode.c md_client.c kernel.c \
		debug.h encode.h kernel.h list.h md_client.h parse-gram.h vm_compile.h \
		xml/ezxml.c

CONFIG_CLEAN_FILES = parse-gram.c parse-gram.h parse-lex.c


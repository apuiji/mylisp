HHS = \
	../myccutils/constr.hh \
	../myccutils/dl.hh \
	../myccutils/myiter.hh \
	../myccutils/mymap.hh \
	../myccutils/myset.hh \
	../myccutils/rbtree.hh \
	../myccutils/xyz.hh \
	ast.hh \
	ast_lexer.hh \
	ast_load.hh \
	ast_optimize.hh \
	ast_parse.hh \
	ast_preproc.hh \
	ast_token.hh \
	ast_trans.hh \
	ast_trans1.hh \
	ast_trans2.hh \
	compile.hh \
	coroutine.hh \
	direction.hh \
	eval.hh \
	gc.hh \
	gc_wb.hh \
	io.hh \
	lists.hh \
	maps.hh \
	object.hh \
	regexs.hh \
	rte.hh \
	strings.hh \
	value.hh
LIBS = myccutils
OBJS = \
	ast.o \
	ast_lexer.o \
	ast_load.o \
	ast_optimize.o \
	ast_parse.o \
	ast_preproc.o \
	ast_trans.o \
	ast_trans1.o \
	ast_trans2.o \
	compile.o \
	coroutine.o \
	eval.o \
	gc.o \
	io.o \
	lists.o \
	maps.o \
	object.o \
	regexs.o \
	rte.o \
	strings.o \
	value.o

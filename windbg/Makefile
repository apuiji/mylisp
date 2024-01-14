HHS = \
	../myccutils/xyz \
	ast \
	ast_lexer \
	ast_load \
	ast_optimize \
	ast_parse \
	ast_preproc \
	ast_token \
	ast_trans \
	ast_trans1 \
	ast_trans2 \
	compile \
	coroutine \
	direction \
	eval \
	gc \
	io_object \
	object \
	rte \
	value
OBJS = \
	ast \
	ast_lexer \
	ast_load \
	ast_optimize \
	ast_parse \
	ast_preproc \
	ast_trans \
	ast_trans1 \
	ast_trans2 \
	compile \
	coroutine \
	eval \
	gc \
	io_object \
	main \
	object \
	rte \
	value

mylisp: $(addsuffix .o, ${OBJS})
	clang++ -o $@ $^ $(addprefix -l, ${LIBS}) -g

%.o: ../%.cc $(addprefix ../, $(addsuffix .hh, ${HHS}))
	clang++ -o $@ $< -I .. -I../.. -std=c++2b -c -g

clean:
	echo>mylisp.exe
	del mylisp.exe *.o

.PHONY: clean

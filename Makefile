HHS = \
	../myccutils/xyz \
	ast \
	ast_include \
	ast_lexer \
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
	macro \
	object \
	rte \
	value
OBJS = \
	ast \
	ast_include \
	ast_lexer \
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
LIBS = stdc++fs

mylisp: $(addsuffix .o, ${OBJS})
	clang++ -o $@ $^ $(addprefix -l, ${LIBS})

%.o: %.cc $(addsuffix .hh, ${HHS})
	clang++ -o $@ $< -I .. -std=c++2b -O2 -c

clean:
	rm *.o

.PHONY: clean

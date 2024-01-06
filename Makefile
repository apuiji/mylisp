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
	macro \
	object \
	rte \
	value
OBJS = \
	ast_include \
	ast_lexer \
	ast_optimize \
	ast_parse \
	ast_preproc \
	ast_trans \
	ast_trans1 \
	ast_trans2 \
	compile \
	eval \
	gc \
	main \
	object \
	rte \
	value
LIBS = boost_coroutine

mylisp: $(addsuffix .o, ${OBJS})
	clang++ -o $@ $^ $(addprefix -l, ${LIBS})

%.o: %.cc $(addsuffix .hh, ${HHS})
	clang++ -o $@ $< -I .. -std=c++2b -O2 -c

clean:
	rm *.o

.PHONY: clean

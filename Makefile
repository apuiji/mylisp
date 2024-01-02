HHS := ../myccutils/xyz ast ast_include ast_lexer ast_optimize ast_parse ast_preproc ast_token ast_trans ast_trans1 ast_trans2 compile direction macro rte
OBJS := ast_include ast_lexer ast_optimize ast_parse ast_preproc ast_trans ast_trans1 ast_trans2 compile

mylisp: $(addsuffix .o, ${OBJS})
	echo 123

%.o: %.cc $(addsuffix .hh, ${HHS})
	clang++ -o $@ $< -I .. -std=c++2b -O2 -c

clean:
	rm *.o

.PHONY: clean

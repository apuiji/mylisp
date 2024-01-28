include ../Makefile

LIBS = dl stdc++fs

mylisp: $(addsuffix .o, ${OBJS})
	clang++ -o $@ $^ $(addprefix -l, ${LIBS}) -g

%.o: ../%.cc $(addprefix ../, $(addsuffix .hh, ${HHS}))
	clang++ -o $@ $< -I .. -I ../.. -std=c++2b -c -g

clean:
	touch mylisp
	rm mylisp *.o

.PHONY: clean

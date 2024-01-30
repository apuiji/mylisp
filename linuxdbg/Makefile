include ../Makefile

LIBS = dl myccutils stdc++fs

mylisp: $(addsuffix .o, ${OBJS})
	make -C ../../myccutils/linuxdbg
	clang++ -o $@ $^ -L ../../myccutils/linuxdbg $(addprefix -l, ${LIBS}) -g

%.o: ../%.cc $(addprefix ../, $(addsuffix .hh, ${HHS}))
	clang++ -o $@ $< -I .. -I ../.. -std=c++2b -c -g

clean:
	touch mylisp
	rm mylisp *.o

.PHONY: clean

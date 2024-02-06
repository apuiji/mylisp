include ../Makefile

PROD = libmylisp.so
LIBS = dl myccutils stdc++fs

${PROD}: $(addsuffix .o, ${OBJS})
	make -C ../../myccutils/linuxdbg
	clang++ -o $@ $^ -L ../../myccutils/linuxdbg $(addprefix -l, ${LIBS}) -shared -g

%.o: ../%.cc $(addprefix ../, $(addsuffix .hh, ${HHS}))
	clang++ -o $@ $< -I .. -I ../.. -fPIC -std=c++2b -c -g

clean:
	touch ${PROD}
	rm ${PROD} *.o

.PHONY: clean

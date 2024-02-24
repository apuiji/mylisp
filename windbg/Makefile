include ../Makefile

PROD = libmylisp.so
LIBS := myccutils

${PROD}: $(addsuffix .o, ${OBJS})
	make -C ../../myccutils/windbg
	clang++ -o $@ $^ -L ../../myccutils/windbg $(addprefix -l, ${LIBS}) -shared -g

%.o: ../%.cc $(addprefix ../, $(addsuffix .hh, ${HHS}))
	clang++ -o $@ $< -I .. -I ../.. -fPIC -std=c++2b -c -g

clean:
	echo > ${PROD}
	del ${PROD} *.o

.PHONY: clean

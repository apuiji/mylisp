include ../Makefile

DEPS1 := $(addprefix ../, $(addsuffix /windbg, ${DEPS}))
PROD = libmylisp.so

${PROD}: $(addsuffix .o, ${OBJS})
	$(addprefix make -C , $(addsuffix ;, ${DEPS1}))
	clang++ -o $@ $^ $(addprefix -L, ${DEPS1}) $(addprefix -l, ${LIBS}) -shared -g

%.o: ../%.cc $(addprefix ../, $(addsuffix .hh, ${HHS}))
	clang++ -o $@ $< -I .. -I ../.. -fPIC -std=c++2b -c -g

clean:
	echo > ${PROD}
	del ${PROD} *.o

.PHONY: clean

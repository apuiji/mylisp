include ../Makefile

DEPS1 := $(addprefix ../, $(addsuffix /linuxdbg, ${DEPS}))
PROD = libmylisp.so

${PROD}: $(addsuffix .o, ${OBJS})
	$(addprefix make -C , $(addsuffix ;, ${DEPS1}))
	clang++ -o $@ $^ $(addprefix -L, ${DEPS1}) $(addprefix -l, ${LIBS}) -shared -g -stdlib=libc++

%.o: ../%.cc $(addprefix ../, $(addsuffix .hh, ${HHS}))
	clang++ -o $@ $< -I .. -I ../.. -fPIC -std=c++2b -c -g -stdlib=libc++

clean:
	touch ${PROD}
	rm ${PROD} *.o

.PHONY: clean

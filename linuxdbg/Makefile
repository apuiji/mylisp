include ../Makefile

PROD = libmylisp.so

${PROD}: ${OBJS} libmyccutils.a
	clang++ -o $@ $^ -L . $(addprefix -l, ${LIBS}) -shared -g -stdlib=libc++

%.o: ../%.cc $(addprefix ../, ${HHS})
	clang++ -o $@ $< -I .. -I ../.. -fPIC -std=c++2b -c -g -stdlib=libc++

libmyccutils.a:
	make -C ../../myccutils/linuxdbg
	cp ../../myccutils/linuxdbg/$@ .

clean:
	touch ${PROD}
	touch libmyccutils.a
	rm ${PROD} *.o

.PHONY: clean

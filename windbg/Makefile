include ../Makefile

PROD = libmylisp.so

${PROD}: ${OBJS} libmyccutils.a
	clang++ -o $@ $^ -L . $(addprefix -l, ${LIBS}) -shared -g -stdlib=libc++

%.o: ../%.cc $(addprefix ../, ${HHS})
	clang++ -o $@ $< -I .. -I ../.. -fPIC -std=c++2b -c -g -stdlib=libc++

libmyccutils.a:
	make -C ../../myccutils/windbg
	copy ..\..\myccutils\windbg\$@ $@

clean:
	echo > ${PROD}
	echo > libmyccutils.a
	del ${PROD} *.o libmyccutils.a

.PHONY: clean

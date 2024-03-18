include ../Makefile

PROD = libmylisp.so

${PROD}: $(addsuffix .o, ${OBJS}) libmyccutils.a
	clang++ -o $@ $^ -L . $(addprefix -l, ${LIBS}) -shared -g

%.o: ../%.cc $(addprefix ../, $(addsuffix .hh, ${HHS}))
	clang++ -o $@ $< -I .. -I ../.. -fPIC -std=c++2b -c -g

libmyccutils.a:
	make -C ../../myccutils/windbg
	copy ..\..\myccutils\windbg\$@ $@

clean:
	echo > ${PROD}
	echo > libmyccutils.a
	del ${PROD} *.o libmyccutils.a

.PHONY: clean

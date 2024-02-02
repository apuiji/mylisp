include ../Makefile

LIBS := myccutils

mylisp.exe: $(addsuffix .o, ${OBJS})
	make -C ../../myccutils/windbg
	clang++ -o $@ $^ -L ../../myccutils/windbg $(addprefix -l, ${LIBS}) -g

%.o: ../%.cc $(addprefix ../, $(addsuffix .hh, ${HHS}))
	clang++ -o $@ $< -I .. -I ../.. -std=c++2b -c -g

clean:
	echo > mylisp.exe
	del mylisp.exe *.o

.PHONY: clean

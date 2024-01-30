include ../Makefile

LIBS := myccutils

mylisp.exe: $(addsuffix .o, ${OBJS}) MAKELIBS
	clang++ -o $@ $^ -L ../../myccutils/windbg $(addprefix -l, ${LIBS}) -g

%.o: ../%.cc $(addprefix ../, $(addsuffix .hh, ${HHS}))
	clang++ -o $@ $< -I .. -I../.. -std=c++2b -c -g

MAKELIBS:
	cd ../../myccutils/windbg
	make

clean:
	echo > mylisp.exe
	del mylisp.exe *.o

.PHONY: MAKELIBS clean

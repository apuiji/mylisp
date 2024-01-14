include ../Makefile

mylisp: $(addsuffix .o, ${OBJS})
	clang++ -o $@ $^ $(addprefix -l, ${LIBS}) -g

%.o: ../%.cc $(addprefix ../, $(addsuffix .hh, ${HHS}))
	clang++ -o $@ $< -I .. -I../.. -std=c++2b -c -g

clean:
	echo>mylisp.exe
	del mylisp.exe *.o

.PHONY: clean

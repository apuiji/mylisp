include Makefile

windbg/helloworld: $(addprefix windbg/, ${OBJS})
	echo helloworld

windbg/%.o: %.cc ${HHS}
	clang++ $< -c -g -I . -O2 -o $@ -std=c++2b -stdlib=libc++

clean:
	echo>windbg\a.o
	del windbg\*.o

.PHONY: clean

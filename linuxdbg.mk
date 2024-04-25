include Makefile

linuxdbg/helloworld: $(addprefix linuxdbg/, ${OBJS})
	echo helloworld

linuxdbg/%.o: %.cc ${HHS}
	clang++ $< -c -g -I . -O2 -o $@ -std=c++2b -stdlib=libc++

clean:
	touch linuxdbg/a.o
	rm linuxdbg/*.o

.PHONY: clean

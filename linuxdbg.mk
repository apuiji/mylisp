include Makefile

linuxdbg/libmylispc.a: $(addprefix linuxdbg/, ${OBJS})
	ar -rsv $@ $^

linuxdbg/%.o: %.cc ${HHS}
	clang++ $< -c -g -I . -O2 -o $@ -std=c++2b -stdlib=libc++

clean:
	touch linuxdbg/libmylispc.a
	touch linuxdbg/a.o
	rm linuxdbg/libmylispc.a linuxdbg/*.o

.PHONY: clean

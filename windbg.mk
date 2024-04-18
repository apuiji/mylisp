include Makefile

windbg/libmylispc.dll.a: $(addprefix windbg/, ${OBJS})
	ar -rsv $@ $^

windbg/%.o: %.cc ${HHS}
	clang++ $< -c -g -I . -O2 -o $@ -std=c++2b -stdlib=libc++

clean:
	echo>windbg\a.o
	echo>windbg\libmylispc.a
	del windbg\*.o
	del windbg\libmylispc.a

.PHONY: clean

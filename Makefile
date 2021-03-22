PREFIX  = /usr/local
LIBDIR  = $(PREFIX)/lib/berry/packages

CFLAGS  = -fPIC -I/usr/include/
LIBS    = -lsox

libsox.so: be-libsox.o
	$(CC) -shared $(CFLAGS) -o $@ be-libsox.o $(LIBS)

install:
	mkdir -p $(DESTDIR)$(LIBDIR)
	cp libsox.so $(DESTDIR)$(LIBDIR)

docs: libsox.so docs/config.ld
	ldoc -c docs/config.ld -d html -a .

clean:
	rm -rf *.o *.so

.PHONY: libsox.so

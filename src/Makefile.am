# Autoconf/automake file

# General compiler/linker flags
AM_CFLAGS = ${CFLAGS}
AM_LDFLAGS = ${LDFLAGS}

TARGETS = liblinthurber.a liblinthurber.so

all: $(TARGETS)

install:
	mkdir -p ${prefix}
	mkdir -p ${prefix}/lib
	mkdir -p ${prefix}/include
	cp liblinthurber.so ${prefix}/lib
	cp liblinthurber.a ${prefix}/lib
	cp linthurber.h ${prefix}/include

liblinthurber.a: linthurber_static.o 
	$(AR) rcs $@ $^

liblinthurber.so: linthurber.o
	$(CC) -shared $(AM_CFLAGS) -o liblinthurber.so $^ $(AM_LDFLAGS)

linthurber.o: linthurber.c
	$(CC) -fPIC -DDYNAMIC_LIBRARY -o $@ -c $^ $(AM_CFLAGS)
	
linthurber_static.o: linthurber.c
	$(CC) -o $@ -c $^ $(AM_CFLAGS)
	
clean:
	rm -rf $(TARGETS)
	rm -rf *.o 


# Autoconf/automake file

#bin_PROGRAMS = test_linthurber
bin_PROGRAMS = 

# General compiler/linker flags
AM_CFLAGS = ${CFLAGS} -I${UCVM_SRC_PATH}/src/ucvm
AM_LDFLAGS = ${LDFLAGS}

objects = test_api.o
TARGETS = $(bin_PROGRAMS)

all: $(bin_PROGRAMS)

install:
	mkdir -p ${prefix}/tests
#	cp test_linthurber ${prefix}/tests

test_linthurber$(EXEEXT): $(objects)
	$(CC) -o $@ $^ $(AM_CFLAGS) -L../src -llinthurber $(AM_LDFLAGS)

$(objects): %.o: %.c
	$(CC) -o $@ -c $^ $(AM_CFLAGS) -I../src/ $(AM_CFLAGS)

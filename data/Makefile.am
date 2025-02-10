# GNU Automake config


# Autoconf/automake file

# General compiler/linker flags
AM_CFLAGS = ${CFLAGS}
AM_FCFLAGS = ${FCFLAGS}
AM_LDFLAGS = ${LDFLAGS}

install:
if WITH_LINTHURBER_LARGEDATA_DIR
	$(MKDIR_P) ${prefix}/model
 	$(LN_S) -f $(LINTHURBER_LARGEDATA_DIR) ${prefix}/model
	echo "USING WITH_LINTHURBER_LARGEDATA_DIR in linthurber"
else
	mkdir -p ${prefix}/data
	cp ./config ${prefix}/data
	cp -R ./linthurber ${prefix}/data
	chmod og+r ${prefix}/data/config
	chmod -R og+r ${prefix}/data/linthurber
	chmod og+x ${prefix}/data/linthurber
endif

clean:
	echo "clean"


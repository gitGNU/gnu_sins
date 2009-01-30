all clean distclean maintainer-clean: src/Makefile
	{ cd src; $(MAKE) $@; }

src/Makefile: src/config.status
	{ cd src; ./config.status; }

src/config.status: src/configure
	{ cd src; ./configure; }

src/configure: src/autogen.sh
	{ cd src; ./autogen.sh; }

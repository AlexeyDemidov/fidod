AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = fidod.conf.in fidod.init.in

make-install-dirs:
	-if test '!' -d $(ETCDIR)/ ; then mkdir -p $(ETCDIR)/; fi

install-data-hook: make-install-dirs
	-@if test '!' -f $(ETCDIR)/$(conffile) ; then \
	echo Installing $(ETCDIR)/$(conffile) ; \
	$(INSTALL_DATA) $(srcdir)/fidod.conf $(ETCDIR)/$(conffile) ; \
	fi


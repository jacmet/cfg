bin_PROGRAMS = cfg-helper
bin_SCRIPTS = cfg
noinst_HEADERS = mtd-abi.h mtd-user.h
EXTRA_DIST = cfg.in
CLEANFILES = $(bin_SCRIPTS)

cfg: cfg.in
	sed -e 's,#PREFIX#,$(prefix),g' \
	    -e 's,#SYSCONF#,$(sysconfdir),g' < $< > $@
	chmod +x $@


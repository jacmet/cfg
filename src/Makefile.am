sbin_PROGRAMS = cfg-helper
sbin_SCRIPTS = cfg
EXTRA_DIST = cfg.in
CLEANFILES = $(sbin_SCRIPTS)

cfg: cfg.in
	sed -e 's,#PREFIX#,$(prefix),g' \
	    -e 's,#SYSCONF#,$(sysconfdir),g' < $< > $@
	chmod +x $@


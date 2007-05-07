bin_PROGRAMS = cfg-helper
dist_bin_SCRIPTS = cfg
noinst_HEADERS = mtd-abi.h mtd-user.h

cfg: cfg.in
	sed -e 's,#PREFIX#,$(prefix),g' \
	    -e 's,#SYSCONF#,$(sysconfdir),g' < $< > $@
	chmod +x $@


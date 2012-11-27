
ronindir = /usr/local/ronin

CC := $(CXX)
ASFLAGS := $(CXXFLAGS)

dist : SCUMMVM.BIN IP.BIN plugin_dist

plugin_dist : plugins
	@[ -z "$(PLUGINS)" ] || for p in $(or $(PLUGINS),none); do \
	  t="`basename \"$$p\" | LC_CTYPE=C tr '[:lower:]' '[:upper:]'`"; \
	  if /usr/bin/test "$$p" -ot "$$t"; then :; else \
	    echo sh-elf-strip -g -o "$$t" "$$p"; \
	    sh-elf-strip -g -o "$$t" "$$p"; \
	    $(srcdir)/backends/platform/dc/check_plugin_symbols "$$t"; \
          fi;\
	done

SCUMMVM.BIN : scummvm.bin
	scramble $< $@

scummvm.bin : scummvm.elf
	sh-elf-objcopy -S -R .stack -O binary $< $@

IP.BIN : ip.txt
	makeip $< $@

ip.txt : $(srcdir)/backends/platform/dc/ip.txt.in
	if [ x"$(VER_EXTRA)" = xgit ]; then \
	  ver="GIT"; \
	else ver="V$(VERSION)"; fi; \
	if expr "$$ver" : V...... >/dev/null; then \
	  ver="V$(VER_MAJOR).$(VER_MINOR).$(VER_PATCH)"; fi; \
	sed -e 's/[@]VERSION[@]/'"$$ver"/ -e 's/[@]DATE[@]/$(shell date '+%Y%m%d')/' < $< > $@


dcdist : dist
	mkdir -p dcdist/scummvm
	cp scummvm.elf SCUMMVM.BIN IP.BIN *.PLG dcdist/scummvm/

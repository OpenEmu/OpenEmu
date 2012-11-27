ifdef WRAP_MALLOC
	LDFLAGS += -Wl,--wrap,malloc -Wl,--wrap,free
endif

backends/platform/wince/PocketSCUMM.o: $(srcdir)/backends/platform/wince/PocketSCUMM.rc
	$(QUIET)$(MKDIR) $(*D)
	$(WINDRES) $(WINDRESFLAGS) -I$(srcdir)/backends/platform/wince $< $@

ifdef DYNAMIC_MODULES
plugins: backends/platform/wince/stub.o backends/platform/wince/PocketSCUMM.o
	$(CXX) backends/platform/wince/stub.o backends/platform/wince/PocketSCUMM.o -L. -lscummvm -o scummvm.exe
	
backends/platform/wince/stub.o: $(srcdir)/backends/platform/wince/stub.cpp
	$(CXX) -c $(srcdir)/backends/platform/wince/stub.cpp -o backends/platform/wince/stub.o
endif

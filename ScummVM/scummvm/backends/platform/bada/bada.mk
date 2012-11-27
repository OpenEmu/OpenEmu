# Bada specific modules are built under eclipse

$(EXECUTABLE): $(OBJS)
	rm -f $@
	ar Tru $@ $(OBJS)

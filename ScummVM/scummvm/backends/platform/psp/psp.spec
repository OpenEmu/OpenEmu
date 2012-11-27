%rename lib	old_lib
*lib:
%(old_lib) -lz -lstdc++ -lc -lm -lpspprof -lpspvfpu -lpspdebug -lpspgu -lpspge -lpspdisplay -lpspctrl -lpspsdk -lpsputility -lpspuser -lpsppower -lpsphprm  -lpsprtc -lpspaudio -lpspaudiocodec -lpspkernel -lpspnet_inet

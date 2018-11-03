
test.exe : test.c thread.h 
	dmc test
	objconv -fnasm test.obj

.phony: clean 
clean: 
	erase *.exe
	erase *.obj
	erase *.map

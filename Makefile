
all: 1 2 

.phony: 1 
.phony: 2 

1 : test.exe 
2 : test2.exe 

test.exe : test.c thread.h 
	dmc test
	objconv -fnasm test.obj
test2.exe : test2.c thread.h 
	dmc test2
	objconv -fnasm test2.obj

.phony: clean 
clean: 
	erase *.exe
	erase *.obj
	erase *.map

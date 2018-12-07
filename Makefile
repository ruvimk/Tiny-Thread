
all: 1 2 3 4

debug: CFLAGS += -D 
debug: 1 2 3 4 5

CC = dmc $(CFLAGS) 

.phony: 1 
.phony: 2 
.phony: 3 
.phony: 4 
.phony: 5 

1 : test.exe 
2 : test2.exe 
3 : test3.exe 
4 : test4.exe 
5 : test5.exe 

test.exe : test.c thread.h 
	$(CC) $(CFLAGS) test
	objconv -fnasm test.obj
test2.exe : test2.c thread.h 
	$(CC) $(CFLAGS) test2
	objconv -fnasm test2.obj
test3.exe : test3.c thread.h 
	$(CC) $(CFLAGS) test3
	objconv -fnasm test3.obj
test4.exe : test4.c thread.h 
	$(CC) $(CFLAGS) test4
	objconv -fnasm test4.obj
test5.exe : test5.c thread.h 
	$(CC) $(CFLAGS) test5
	objconv -fnasm test5.obj

.phony: clean 
clean: 
	rm -f *.exe
	rm -f *.obj
	rm -f *.map

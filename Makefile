
TARGETS	=	test_buffer test_lst test_cmd
COMOBJ	=	buffer.o \
		cmdline.o \
		memory.o \
		ptr_lst.o \
		str.o

CC	=	gcc
COPTS	=	-Wall -Wextra -Wpedantic
LOPTS	=
DEBUG	=	-g -DUSE_ASSERTS

%.o:%.c
	$(CC) $(COPTS) $(DEBUG) -c -o $@ $<

all: $(TARGETS)

buffer.o: buffer.c buffer.h myassert.h
cmdline.o: cmdline.c cmdline.h myassert.h
memory.o: memory.c memory.h myassert.h
ptr_lst.o: ptr_lst.c ptr_lst.h myassert.h
str.o: str.c str.h myassert.h

clean:
	-$(RM) $(TARGETS) $(COMOBJ) 

test_buffer: buffer.c memory.o
	$(CC) $(COPTS) $(DEBUG) -DTEST_BUFFER -o $@ $^

test_lst: ptr_lst.c memory.o
	$(CC) $(COPTS) $(DEBUG) -DTEST_PTR_LST -o $@ $^

test_cmd: cmdline.c memory.o buffer.o ptr_lst.o str.o
	$(CC) $(COPTS) $(DEBUG) -DTEST_CMDLINE -o $@ $^

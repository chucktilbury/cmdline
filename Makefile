
LIBRARY	=	libcmdline.a
TARGETS	=	test_cmd 
COMOBJ	=	buffer.o \
			cmdline.o \
			memory.o \
			ptr_lst.o \
			str.o

CC	=	gcc
COPTS	=	-Wall -Wextra -Wpedantic
LOPTS	=	-L./ -lcmdline
DEBUG	=	-g -DUSE_ASSERTS

%.o:%.c
	$(CC) $(COPTS) $(DEBUG) -c -o $@ $<

all: $(TARGETS)

$(TARGETS): test.c $(LIBRARY)
	$(CC) $(COPTS) $(DEBUG) -o $@ $< $(LOPTS)

buffer.o: buffer.c buffer.h myassert.h memory.o
cmdline.o: cmdline.c cmdline.h myassert.h memory.o
memory.o: memory.c memory.h myassert.h
ptr_lst.o: ptr_lst.c ptr_lst.h myassert.h memory.o
str.o: str.c str.h myassert.h memory.o

$(LIBRARY): $(COMOBJ)
	$(AR) rcs $@ $^

clean:
	-$(RM) $(TARGETS) $(COMOBJ) $(LIBRARY)

test_buffer: buffer.c memory.o
	$(CC) $(COPTS) $(DEBUG) -DTEST_BUFFER -o $@ $^

test_lst: ptr_lst.c memory.o
	$(CC) $(COPTS) $(DEBUG) -DTEST_PTR_LST -o $@ $^


# Target library
lib		:= libfs.a
objs 	:= fs.o disk.o
CC		:= gcc
CFLAGS	:= -Wall -Wextra -Werror -MMD


all: $(lib)

deps := $(patsubst %.o,%.d, $(objs))
-include $(deps)

%.o: %.c
	@echo "CC $@ $^"
	$(CC) $(CFLAGS) -c -o $@ $<
	
libfs.a: $(objs)
	@echo "CC $@ $^"
	ar rcs libfs.a $(objs)


clean:
	@echo "clean"
	rm -f $(targets) $(objs) $(deps)


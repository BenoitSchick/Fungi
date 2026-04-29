EXEC = ad777 
SRCS = $(wildcard src/*.c)
LDFLAGS =
CFLAGS=-Wno-unused-parameter

# Makefile common part
# CC=gcc
CC=arm-linux-gnueabihf-gcc
LD=arm-linux-gnueabihf-gcc
AR=ar
CFLAGS+=-pedantic -Wall -Wextra -g -c -mtune=cortex-a8 -Og -MD -std=gnu11
EXE=$(EXEC)
OBJDIR=.obj
OBJS+=$(addprefix $(OBJDIR)/, $(SRCS:.c=.o))

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@


all: $(OBJDIR)/ $(EXEC)


clean:
	rm -Rf .obj 
	rm -f $(EXEC) *~
	
$(EXEC): $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $@ 
			
$(OBJDIR)/:
	mkdir -p $(OBJDIR)
	mkdir -p $(OBJDIR)/src

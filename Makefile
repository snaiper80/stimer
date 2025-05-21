CFILES  := main.c stimer.c
PROG    := stimer
CFLAGS  := -Wall -Wextra -g -DDEBUG # Added -DDEBUG here
LDFLAGS := -lm # Added -lm here

# -MMD generates dependencies while compiling
CFLAGS += -MMD
CC 	   := gcc

OBJFILES := $(CFILES:.c=.o)
DEPFILES := $(CFILES:.c=.d)

$(PROG) : $(OBJFILES)
	$(LINK.o) $(LDFLAGS) -o $@ $^

clean :
	rm -f $(PROG) $(OBJFILES) $(DEPFILES)

memcheck: $(PROG)
	valgrind --leak-check=full --show-leak-kinds=all ./$(PROG)

-include $(DEPFILES)
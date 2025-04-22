TARGETS = task1 task2
CC = gcc
CFLAGS = -Wall -Wextra -lm
SRCS = task1.c task2.c
OBJS = $(SRCS:.c=.o)

all: $(TARGETS)

task1: task1.o
	$(CC) $< -o $@ $(CFLAGS)

task2: task2.o
	$(CC) $< -o $@ $(CFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGETS)

cleanall: clean
	rm -f $(TARGETS)

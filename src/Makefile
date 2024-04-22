CC := gcc
TARGET := proj2
CFLAGS := -std=gnu99 -Wall -Wextra -Werror -pedantic 
LDFLAGS := -lrt
SRCS := proj2.c
OBJS := $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS)
CC = gcc
CFLAGS = -D _DEFAULT_SOURCE -g -Wall
TARGET = testlib

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)
DEPS := $(SRCS:.c=.d)
INTS := $(wildcard *.lst *.obj)

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

sinclude $(DEPS)

%.d: %.c
	@set -e; \
	$(CC) $(CFLAGS) -MM $< \
	| sed 's,\($*\)\.o[ :]*,\1.o $@: ,g' > $@

.PHONY: clean
clean:
	rm -f $(OBJS) $(DEPS) $(INTS)
	rm -f $(TARGET)

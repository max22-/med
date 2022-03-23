CFLAGS = -DUSE_C_STDLIB

all: med

med: med.c
	gcc $(CFLAGS) med.c -o med -std=c89 -Wall

.PHONY: run clean

run: med
	./med

clean:
	rm -f med

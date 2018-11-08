SRC=$(wildcard *.c)
DST=$(patsubst %.c,%,$(SRC))

default: $(DST)

%: %.c
	gcc -O3 -o $@ $^

clean:
	rm -rf $(DST)

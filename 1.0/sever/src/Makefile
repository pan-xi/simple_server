OBJS:=$(wildcard *.c)
server:$(OBJS)
	gcc $^ -o ../bin/$@ -I../include -D DEBUG
clean:
	rm server
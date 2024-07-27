OBJS:=user.o netlink.o ubus.o config.o utils.o main.o
EXEC:=oafd
all: $(OBJS)
	$(CC) -o $(EXEC) $(OBJS) $(LIBS)
clean:
	rm $(EXEC) *.o

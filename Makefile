CC := mpic++
CXXFLAGS := -c -Wl,--no-as-needed -std=c++11 -lpthread -I. -I$(MPIHOME)/include  -g
LDFLAGS := -Wl,--no-as-needed -std=c++11  -lpthread -g

OBJS :=  BTime.o Tick.o MPIFileSegment.o Logger.o LogMessage.o Scrub.o

Scrub : $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS)  -o Scrub

%.o: %.c
	$(CC) $(CXXFLAGS) -c -o $@ $<

clean :
	rm  scrub $(OBJS)

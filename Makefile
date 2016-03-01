CC := mpic++
CXXFLAGS := -c -Wl,--no-as-needed -std=c++11 -lpthread -I. -I$(MPIHOME)/include  -g
LDFLAGS := -Wl,--no-as-needed -std=c++11  -lpthread -g

OBJS :=  BTime.o Tick.o MPIFileSegment.o Logger.o LogMessage.o  RunningStats.o

all: Scrub Normal

Scrub : $(OBJS) Scrub.o
	$(CC) $(OBJS) Scrub.o $(LDFLAGS)  -o Scrub

Normal: BTime.o Tick.o MPIFileSegment.o Logger.o LogMessage.o RunningStats.o Normal.o
	$(CC) $(OBJS) Normal.o $(LDFLAGS)  -o Normal

%.o: %.c
	$(CC) $(CXXFLAGS) -c -o $@ $<

clean :
	rm  Scrub Normal $(OBJS)

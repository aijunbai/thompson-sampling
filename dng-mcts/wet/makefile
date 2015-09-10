CXX	=	g++
CCFLAGS	=	-Wall -O3 -msse -I../engine
EXTRA	=	-std=c++0x
OBJS	=	main.o random.o
TARGET	=	wet

$(TARGET):	$(OBJS)
		$(CXX) $(CCFLAGS) $(EXTRA) -o $(TARGET) $(OBJS)

clean:
		rm -f $(OBJS) $(TARGET) *~ core

.cc.o:
		$(CXX) $(CCFLAGS) $(EXTRA) -c $<


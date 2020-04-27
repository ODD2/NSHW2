CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

LIBS = -lssl -lcrypto

TARGET = NSHW2_server NSHW2_client

COMMON_OBJS = ssl_helper.o

all: $(TARGET)

NSHW2_server: NSHW2_server.o $(COMMON_OBJS)
	$(CXX) -o $@ $^ $(LIBS)

NSHW2_client: NSHW2_client.o $(COMMON_OBJS)
	$(CXX) -o $@ $^ $(LIBS)

#$(TARGET):	$(OBJS)
#	$(CXX) -o $(TARGET) $(OBJS) $(LIBS) -lssl -lcrypto

clean:
	rm -f *.o $(TARGET)

CXXFLAGS=$(shell pkg-config --cflags acx)
LDFLAGS=$(shell pkg-config --libs acx)
all:
	$(CXX) main.cpp -o consumer $(CXXFLAGS) $(LDFLAGS)

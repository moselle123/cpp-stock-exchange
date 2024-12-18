CXX = g++

CXXFLAGS = -O0 -g3 -std=c++17 -Wall -Wextra -pedantic

all: main

main: main.cpp orderlist.o
	$(CXX) $(CXXFLAGS) main.cpp orderlist.o -o main

orderlist.o: orderlist.cpp orderlist.h
	$(CXX) $(CXXFLAGS) -c orderlist.cpp -o orderlist.o

clean:
	rm -f *~ *.o main main.exe *.stackdump

deepclean: clean
	rm -f *.stackdump

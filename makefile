CXX = g++

CXXFLAGS = -O0 -g3 -std=c++17 -Wall -Wextra -pedantic

all: main

main: main.cpp OrderList.o
	$(CXX) $(CXXFLAGS) main.cpp OrderList.o -o main

OrderList.o: OrderList.cpp OrderList.h
	$(CXX) $(CXXFLAGS) -c OrderList.cpp -o OrderList.o

deepclean:
	rm -f *~ *.o main main.exe *.stackdump

clean: clean
	rm -f *~ *.o *.stackdump

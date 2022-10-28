demo: demo.cpp include/netpbm.hpp
	g++ -O3 -Iinclude -Wall -Werror -pedantic -std=c++17 -o demo demo.cpp

clean:
	rm -rf demo

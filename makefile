all: udp_server udp_client
udp_helper: udp_helper.cpp udp_helper.h
	g++ -c -std=c++14 udp_helper.cpp -lpthread -I.
udp_server: udp_server.cpp udp_helper
	g++ -o udp_server -std=c++11 udp_server.cpp -lpthread udp_helper.o	
udp_client: udp_server.cpp udp_helper
	g++ -o udp_client -std=c++11 udp_client.cpp -lpthread udp_helper.o	
clean:
	rm -rf *.o udp_server udp_client

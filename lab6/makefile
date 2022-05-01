all:
	g++ main.cpp -o main -lzmq -pthread
	g++ child.cpp -o child -lzmq -pthread
	g++ server.cpp -o server -lzmq
	g++ client.cpp -o client -lzmq
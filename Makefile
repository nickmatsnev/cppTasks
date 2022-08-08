all: first second third

first_support:
	g++ firstTask/processForFirst.cpp -o first_support
	./first_support
	rm -f first_support

first:
	g++ firstTask/first.cpp -o first
	./first first_support 6
	rm -f first

second:
	g++ secondTask/second.cpp -o second
	./second secondTask/source secondTask/replica 10 secondTask/log.txt
	rm -f second

third_server:
	g++ thirdTask/server.cpp -o server
	./server
	rm -f server

third_client:
	g++ thirdTask/client.cpp -o client
	./client
	rm -f client

third_wrong:
	g++ thirdTask/wrong.cpp -o wrong
	./wrong
	rm -f wrong

clean:
	rm -f first
	rm -f second
	rm -f server
	rm -f client
	rm -f wrong

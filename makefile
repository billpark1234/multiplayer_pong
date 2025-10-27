compile:
	g++ -c ./client.cpp
	g++ -c ./server.cpp
	g++ -O3 client.o -o client -lsfml-network -lsfml-system -lsfml-graphics -lsfml-window
	g++ -O3 server.o -o server -lsfml-network -lsfml-system -lsfml-graphics

run: compile
	./app

clean:
	rm -f *.o app client server
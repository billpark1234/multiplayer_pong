compile:
	g++ -c ./main.cpp
	g++ -c ./client.cpp
	g++ -c ./server.cpp
	g++ -O3 main.o -o app -lsfml-graphics -lsfml-window -lsfml-system
	g++ -O3 client.o -o client -lsfml-network -lsfml-system
	g++ -O3 server.o -o server -lsfml-network -lsfml-system

run: compile
	./app

clean:
	rm -f *.o app client server
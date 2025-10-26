compile:
	g++ -c ./main.cpp
	g++ -O3 main.o -o app -lsfml-graphics -lsfml-window -lsfml-system

run: compile
	./app

clean:
	rm -f *.o app
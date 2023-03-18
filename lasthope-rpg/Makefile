FLAGS = -pedantic-errors -std=c++11

systemf.o: systemf.cpp systemf.h
	g++ $(FLAGS) -c systemf.cpp
        
hope.o: hope.cpp
	g++ $(FLAGS) -c hope.cpp

chooseCharacter.o: chooseCharacter.cpp chooseCharacter.h
	g++ $(FLAGS) -c chooseCharacter.cpp

main: systemf.o hope.o chooseCharacter.o
	g++ $(FLAGS) systemf.o hope.o -o main chooseCharacter.o
        
clean:
	rm -f main hope.o systemf.o  chooseCharacter.o

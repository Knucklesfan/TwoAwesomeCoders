OBJS	= main.o font.o knuxfanscreen.o background.o titlescreen.o
HEADER	= 
OUT	= SDLTetris
CC	 = g++
FLAGS	 = -g -c -Wall -Iinclude/ -DCLIENT
LFLAGS	 = -lSDL2 -lSDL2_mixer -lavcodec -lavutil -lavformat -fsanitize=address

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

main.o: src/main.cpp
	$(CC) $(FLAGS) src/main.cpp 
background.o: src/background.cpp
	$(CC) $(FLAGS) src/background.cpp 
knuxfanscreen.o: src/knuxfanscreen.cpp
	$(CC) $(FLAGS) src/knuxfanscreen.cpp 
titlescreen.o: src/titlescreen.cpp
	$(CC) $(FLAGS) src/titlescreen.cpp 

font.o: src/font.cpp
	$(CC) $(FLAGS) src/font.cpp 
clean:

	rm -f $(OBJS) $(OUT) 

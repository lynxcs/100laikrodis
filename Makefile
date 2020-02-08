SDL_LIB = -L/usr/lib -L./lib -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
SDL_INCLUDE = -I/usr/include

CXXFLAGS = -Iinclude $(SDL_INCLUDE)
LDFLAGS = $(SDL_LIB)

clock: main.cpp
	g++ main.cpp $(CXXFLAGS) $(LDFLAGS) -o a.out

# Files
S_FILES=demo.cpp

# Output
EXEC=demo

# Build settings
CC=g++
# SDL options
CC_SDL=-lSDL2 -lSDL2_gfx -lSDL2_image


all: demo

demo: $(S_FILES)
	$(CC) $(S_FILES) -o $(EXEC) $(CC_SDL)

clean:
	rm -f demo *.o

CC = gcc
CFLAGS = -g -std=c11 -Iinclude
LDFLAGS = -Llib -lglfw3dll -lopengl32 -lgdi32
SRC = src/main.c src/glad.c
OUT = cutable.exe

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $(OUT)

clean:
	del $(OUT)

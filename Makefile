CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -pedantic -Iinclude
SRC := src/main.c src/assembler.c src/executor.c src/parser.c

ifeq ($(OS),Windows_NT)
TARGET := neander_cli.exe
RM := del /Q
else
TARGET := neander_cli
RM := rm -f
endif

.PHONY: all clean run-example

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run-example: $(TARGET)
	./$(TARGET) asmrun "exemplo_soma.asm" --dec

clean:
	-$(RM) $(TARGET)

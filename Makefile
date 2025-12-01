CC = clang
CFLAGS = -Wall -Wextra -std=c11

TARGET = sequencer_demo
OBJS = main.o sequencer.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c sequencer.h
	$(CC) $(CFLAGS) -c main.c

sequencer.o: sequencer.c sequencer.h
	$(CC) $(CFLAGS) -c sequencer.c

clean:
	rm -f $(TARGET) $(OBJS)
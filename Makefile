CC = gcc
CFLAGS = -Wall -Wextra -pthread
SRC_DIR = hw1
TARGET = $(SRC_DIR)/all_tests


SRCS = $(wildcard $(SRC_DIR)/*.c)

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(SRC_DIR)/*.o $(TARGET)
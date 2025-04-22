
CC = clang
CFLAGS = -Wall -Wextra -pedantic -ggdb
# CFLAGS += -O2

SOURCE  := ./Source
BUILD   := ./Build


all: clean $(BUILD)/main


$(BUILD)/main: $(BUILD)/main.o $(BUILD)/tokenizer.o $(BUILD)/String_View.o $(BUILD)/arena.o            | build
	$(CC) $(CFLAGS) -o $(BUILD)/main   $(BUILD)/main.o $(BUILD)/tokenizer.o $(BUILD)/String_View.o $(BUILD)/arena.o


# ------------------ *.o's ------------------

$(BUILD)/main.o: $(SOURCE)/main.c                                        | build
	$(CC) $(CFLAGS) -c -o $(BUILD)/main.o $(SOURCE)/main.c

$(BUILD)/tokenizer.o: $(SOURCE)/tokenizer.c                              | build
	$(CC) $(CFLAGS) -c -o $(BUILD)/tokenizer.o $(SOURCE)/tokenizer.c

$(BUILD)/String_View.o: $(SOURCE)/String_View.h                          | build
	$(CC) $(CFLAGS) -x c -DSTRING_VIEW_IMPLEMENTATION -c -o $(BUILD)/String_View.o $(SOURCE)/String_View.h

$(BUILD)/arena.o: $(SOURCE)/arena.h                                      | build
	$(CC) $(CFLAGS) -x c -DARENA_IMPLEMENTATION -c -o $(BUILD)/arena.o $(SOURCE)/arena.h



build:
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)


CC = clang
CFLAGS = -Wall -Wextra -pedantic -Werror -ggdb
# CFLAGS += -O2

SOURCE  := ./Source
BUILD   := ./Build


all: clean $(BUILD)/main


$(BUILD)/main: $(BUILD)/main.o $(BUILD)/Report_Error.o $(BUILD)/tokenizer.o $(BUILD)/AST.o $(BUILD)/Compile.o $(BUILD)/String_View.o $(BUILD)/arena.o $(BUILD)/context.o          | build
	$(CC) $(CFLAGS) -o $(BUILD)/main   $(BUILD)/main.o $(BUILD)/Report_Error.o $(BUILD)/tokenizer.o $(BUILD)/AST.o $(BUILD)/Compile.o $(BUILD)/String_View.o $(BUILD)/arena.o $(BUILD)/context.o


# ------------------ *.o's ------------------

$(BUILD)/main.o: $(SOURCE)/main.c                                        | build
	$(CC) $(CFLAGS) -c -o $(BUILD)/main.o $(SOURCE)/main.c

$(BUILD)/Report_Error.o: $(SOURCE)/Report_Error.c                              | build
	$(CC) $(CFLAGS) -c -o $(BUILD)/Report_Error.o $(SOURCE)/Report_Error.c

$(BUILD)/tokenizer.o: $(SOURCE)/tokenizer.c                              | build
	$(CC) $(CFLAGS) -c -o $(BUILD)/tokenizer.o $(SOURCE)/tokenizer.c

$(BUILD)/AST.o: $(SOURCE)/AST.c                              | build
	$(CC) $(CFLAGS) -c -o $(BUILD)/AST.o $(SOURCE)/AST.c

$(BUILD)/Compile.o: $(SOURCE)/Compile.c                              | build
	$(CC) $(CFLAGS) -c -o $(BUILD)/Compile.o $(SOURCE)/Compile.c


# ------------------ single header files ------------------

$(BUILD)/String_View.o: $(SOURCE)/String_View.h                          | build
	$(CC) $(CFLAGS) -x c -DSTRING_VIEW_IMPLEMENTATION -c -o $(BUILD)/String_View.o $(SOURCE)/String_View.h

$(BUILD)/arena.o: $(SOURCE)/arena.h                                      | build
	$(CC) $(CFLAGS) -x c -DARENA_IMPLEMENTATION -c -o $(BUILD)/arena.o $(SOURCE)/arena.h

$(BUILD)/context.o: $(SOURCE)/context.h                                      | build
	$(CC) $(CFLAGS) -x c -DCONTEXT_IMPLEMENTATION -c -o $(BUILD)/context.o $(SOURCE)/context.h



build:
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD) ./lanthi_build

CC      ?= gcc
CFLAGS  ?= -std=c11 -Wall -Wextra -pedantic -O2
SRC_DIR := src
BUILD   := build

APP  := $(BUILD)/states
TEST := $(BUILD)/test_states_list

.PHONY: all run test sanitize mutation original clean

all: $(APP)

$(BUILD):
	mkdir -p $(BUILD)

$(APP): $(SRC_DIR)/main.c $(SRC_DIR)/states_list.c $(SRC_DIR)/states_list.h | $(BUILD)
	$(CC) $(CFLAGS) $(SRC_DIR)/main.c $(SRC_DIR)/states_list.c -o $@

$(TEST): tests/test_states_list.c $(SRC_DIR)/states_list.c $(SRC_DIR)/states_list.h | $(BUILD)
	$(CC) $(CFLAGS) tests/test_states_list.c $(SRC_DIR)/states_list.c -o $@

run: $(APP)
	./$(APP)

test: $(TEST)
	./$(TEST)

# Rebuild tests and a scripted demo with AddressSanitizer + LeakSanitizer.
sanitize: | $(BUILD)
	$(CC) -std=c11 -Wall -Wextra -g -fsanitize=address,undefined \
		tests/test_states_list.c $(SRC_DIR)/states_list.c -o $(BUILD)/test_asan
	./$(BUILD)/test_asan
	$(CC) -std=c11 -Wall -Wextra -g -fsanitize=address,undefined \
		$(SRC_DIR)/main.c $(SRC_DIR)/states_list.c -o $(BUILD)/states_asan
	NO_COLOR=1 ./$(BUILD)/states_asan < tests/demo_input.txt > /dev/null

# Build the code exactly as submitted (for comparison only).
original: | $(BUILD)
	$(CC) -w original/submitted_version.c -o $(BUILD)/original_submitted

# Check that the tests catch deliberately introduced bugs.
mutation:
	bash scripts/mutation_check.sh

clean:
	rm -rf $(BUILD)

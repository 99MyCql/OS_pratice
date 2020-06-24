BUILD_DIR := ./build

CC = gcc
CFLAGS = -l pthread

.PHONY: clean run

# $(app) is given in command line like: make build app=mysys
$(BUILD_DIR)/$(app): $(app).c
	mkdir -p $(BUILD_DIR)
	$(CC) $(app).c -o $(BUILD_DIR)/$(app) $(CFLAGS)

run: $(BUILD_DIR)/$(app)
	$(BUILD_DIR)/$(app)

clean:
	rm -rf $(BUILD_DIR)/*

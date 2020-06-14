BUILD_DIR := ./build

CC = gcc

.PHONY: clean build

build: $(app).c
    mkdir -p $(BUILD_DIR)
    $(CC) $(app).c -o $(BUILD_DIR)/$(app)

clean:
    rm -rf $(BUILD_DIR)/*

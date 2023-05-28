CC := gcc
CFLAGS = -W -Wall -Wextra -pedantic -g -ggdb -O2
TARGET := ftp-client
RELEASE_DIR := release
DEBUG_DIR := debug
ARCHIVE_NAME := 050541_Kokhanovsky_H.E._Capstone_Project

.PHONY: all release debug clean

all: release

release: CFLAGS += -O2
release: $(RELEASE_DIR)/$(TARGET) archive

debug: CFLAGS += -O0
debug: $(DEBUG_DIR)/$(TARGET)

$(RELEASE_DIR)/$(TARGET): ftp-client.c
	mkdir -p $(RELEASE_DIR)
	$(CC) $(CFLAGS) -o $@ $<

$(DEBUG_DIR)/$(TARGET): ftp-client.c
	mkdir -p $(DEBUG_DIR)
	$(CC) $(CFLAGS) -o $@ $<

archive:
	mkdir -p $(RELEASE_DIR)/Kokhanovsky_H.E/course_work
	cp ftp-client.c $(RELEASE_DIR)/Kokhanovsky_H.E/course_work
	cp -r Documentation $(RELEASE_DIR)/Kokhanovsky_H.E/course_work

tar:
	tar -czvf $(ARCHIVE_NAME).tar.gz $(RELEASE_DIR)

clean:
	rm -rf $(RELEASE_DIR) $(DEBUG_DIR) $(ARCHIVE_NAME).tar.gz

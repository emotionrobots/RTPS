BIN_DIR = bin
LIB_DIR = lib
INC_DIR = include 
CLIENT_DIR = src/rtps_client
SERVER_DIR = src/rtps_server
COMMON_DIR = src/rtps_common

CC = gcc
AR = ar rcs
CFLAGS = -Wall -Wno-sign-compare -Wextra -O2 -I. -I$(INC_DIR)
LDFLAGS = 
LIBS = -lm -lc -lcjson -lSDL2 -lSDL2_gfx


# Source files for libraries
COMMON_LIB_SRCS = $(COMMON_DIR)/rtps_common.c $(COMMON_DIR)/circular_buffer.c


# Static library targets
COMMON_LIB = $(LIB_DIR)/librtps_common.a


# Object files for libraries
COMMON_LIB_OBJS = $(COMMON_LIB_SRCS:.c=.o)


# Source files for binaries
CLIENT_BIN_SRCS = $(CLIENT_DIR)/rtps_client.c 
SERVER_BIN_SRCS = $(SERVER_DIR)/rtps_server.c


# Binary targets
CLIENT_BIN = $(BIN_DIR)/rtps_client
SERVER_BIN = $(BIN_DIR)/rtps_server


# Object files for binaries
CLIENT_BIN_OBJS = $(CLIENT_BIN_SRCS:.c=.o)
SERVER_BIN_OBJS = $(SERVER_BIN_SRCS:.c=.o)


# All targets
.PHONY: all clean
all: $(COMMON_LIB) $(CLIENT_BIN) $(SERVER_BIN)


# Build static libraries
$(COMMON_LIB): $(COMMON_LIB_OBJS)
	$(AR) $@ $^


# Build binaries
$(CLIENT_BIN): $(CLIENT_BIN_OBJS) $(COMMON_LIB)
	$(CC) -o $@ $(CLIENT_BIN_OBJS) $(COMMON_LIB) $(LIBS)

$(SERVER_BIN): $(SERVER_BIN_OBJS) $(COMMON_LIB)
	$(CC) -o $@ $(SERVER_BIN_OBJS) $(COMMON_LIB) $(LIBS)


# Compile source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


# Clean target
clean:
	rm -f $(CLIENT_BIN_OBJS) $(CLIENT_BIN)
	rm -f $(SERVER_BIN_OBJS) $(SERVER_BIN)
	rm -f $(COMMON_LIB) $(COMMON_LIB_OBJS)


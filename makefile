BIN = bin
LIB = lib
CLIENT_DIR = rtps_client
SERVER_DIR = rtps_server

CC = gcc
AR = ar rcs
CFLAGS = -Wall -Wextra -O2 -I$(SERVER_DIR)
LDFLAGS = -lm -lc -lcjson -lSDL2 -lSDL2_gfx


# Source files for libraries
CLIENT_LIB_SRCS = $(CLIENT_DIR)/client_lib.c


# Static library targets
CLIENT_LIB = $(LIB)/librtps_client.a


# Object files for libraries
CLIENT_LIB_OBJS = $(CLIENT_LIB_SRCS:.c=.o)


# Source files for binaries
CLIENT_BIN_SRCS = $(CLIENT_DIR)/client_main.c 
SERVER_BIN_SRCS = $(SERVER_DIR)/rtps_server.c


# Binary targets
CLIENT_BIN = ${BIN}/rtps_client
SERVER_BIN = ${BIN}/rtps_server


# Object files for binaries
CLIENT_BIN_OBJS = $(CLIENT_BIN_SRCS:.c=.o)
SERVER_BIN_OBJS = $(SERVER_BIN_SRCS:.c=.o)


# All targets
.PHONY: all clean
all: $(CLIENT_LIB) $(CLIENT_BIN) $(SERVER_BIN)


# Build static libraries
$(CLIENT_LIB): $(CLIENT_LIB_OBJS)
	$(AR) $@ $^


# Build binaries
$(CLIENT_BIN): $(CLIENT_BIN_OBJS) $(CLIENT_LIB) $(CLIENT_BIN_SRCS) 
	$(CC) -o $@ $(CLIENT_BIN_OBJS) $(CLIENT_LIB) $(LDFLAGS)

$(SERVER_BIN): $(SERVER_BIN_OBJS)
	$(CC) -o $@ $(SERVER_BIN_OBJS) $(LDFLAGS)


# Compile source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


# Clean target
clean:
	rm -f $(CLIENT_LIB_OBJS) $(CLIENT_BIN_OBJS) $(CLIENT_LIB) $(CLIENT_BIN)
	rm -f $(SERVER_BIN_OBJS) $(SERVER_BIN)


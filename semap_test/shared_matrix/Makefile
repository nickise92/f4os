SERVER=bin/server
CLIENT=bin/client
CFLAGS=-Wall -std=c99
INCLUDES=-I./lib

SRCS_SERVER=src/server.c
SRCS_CLIENT=src/client.c

all: $(SERVER) $(CLIENT)

server: $(SERVER)

client: $(CLIENT)

$(SERVER): $(SRCS_SERVER)
	@echo "Compiling Server..."
	gcc $(CFLAGS) $(INCLUDES) $(SRCS_SERVER) -o $(SERVER)
	@echo "Done."

$(CLIENT): $(SRCS_CLIENT)
	@echo "Compiling Client..."
	gcc $(CFLAGS) $(INCLUDES) $(SRCS_CLIENT) -o $(CLIENT)
	@echo "Done."

.PHONY: clean

clean:
	@echo "Removing executable file(s)..."
	@rm -f bin/*
	@echo "Done."


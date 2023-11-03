all: bin/F4Server bin/F4Client


bin/F4Server: src/server.c
	@echo "Compiling server..."
	gcc src/server.c -o bin/F4Server
	@echo "Done."

bin/F4Client: src/client.c 
	@echo "Compiling client..."
	gcc src/client.c -o bin/F4Client 
	@echo "Done."

clean: 
	@echo "Removing all file(s)..."
	rm bin/*
	@echo "Done."
	
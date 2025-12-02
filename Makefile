# Executable names
SERVER_TARGET = chat_server
CLIENT_TARGET = chat_client

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -Wpedantic -O2 -pthread -I.
LDFLAGS = -pthread

# Source files
SERVER_SRCS = server_main.cpp server/server.cpp server/client_connection.cpp
CLIENT_SRCS = client_main.cpp client/client_app.cpp

SERVER_OBJS = $(SERVER_SRCS:.cpp=.o)
CLIENT_OBJS = $(CLIENT_SRCS:.cpp=.o)

# Find all header files
HEADERS = $(shell find . -name "*.hpp" -o -name "*.h")

# Default target - build both
all: $(SERVER_TARGET) $(CLIENT_TARGET)

# Server binary
$(SERVER_TARGET): $(SERVER_OBJS)
	@echo "Linking $(SERVER_TARGET)..."
	$(CXX) $(LDFLAGS) -o $@ $^
	@echo "Server build complete!"

# Client binary
$(CLIENT_TARGET): $(CLIENT_OBJS)
	@echo "Linking $(CLIENT_TARGET)..."
	$(CXX) $(LDFLAGS) -o $@ $^
	@echo "Client build complete!"

# Compile .cpp -> .o
%.o: %.cpp $(HEADERS)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build only server
server: $(SERVER_TARGET)

# Build only client
client: $(CLIENT_TARGET)

# Run server
run-server: $(SERVER_TARGET)
	./$(SERVER_TARGET)

# Run client
run-client: $(CLIENT_TARGET)
	./$(CLIENT_TARGET)

# Clean build artifacts
clean:
	@echo "Cleaning..."
	rm -f $(SERVER_OBJS) $(CLIENT_OBJS) $(SERVER_TARGET) $(CLIENT_TARGET)
	@echo "Clean complete!"

.PHONY: all server client run-server run-client clean
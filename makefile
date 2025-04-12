BUILD = ./build
SRC = ./src
LIBS = ./libs
INCLUDE = ./include 

SOURCE	= $(wildcard $(SRC)/*.cpp)
LSOURCE	= $(wildcard $(LIBS)/*/*.cpp)
OBJS	= $(patsubst $(SRC)/%.cpp, $(BUILD)/%.o, $(SOURCE))
LOBJS	= $(patsubst $(LIBS)/%.cpp, $(BUILD)/%.o, $(LSOURCE))

OUT	= $(BUILD)/solution.out
FLAGS	= -Wall -DIXWEBSOCKET_USE_TLS -DIXWEBSOCKET_USE_SECURE_TRANSPORT -DIXWEBSOCKET_USE_OPEN_SSL -g
LFLAGS	= 
CC	= clang++

all: $(OUT)

$(BUILD)/solution.out: $(OBJS) $(LOBJS)
	$(CC) -g $(LOBJS) $(OBJS) -o $(OUT) $(LFLAGS) -lssl -lcrypto

$(BUILD)/%.o: $(SRC)/%.cpp $(INCLUDE)
	$(CC) $(FLAGS) $(INCLUDE:%=-I %) -c $< -std=c++23 -o $@

$(BUILD)/%.o: $(LIBS)/%.cpp $(LIBS)
	$(CC) $(FLAGS) $(LIBS:%=-I %) -c $< -std=c++23 -o $@

clean:
	rm -f $(OBJS) $(LOBJS) $(OUT)
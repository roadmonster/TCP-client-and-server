SERVER = http_svr


all: $(SERVER)

$(SERVER): $(SERVER).cpp
	g++ -std=c++17 main.cpp -lstdc++fs -o $@ $^


clean:
	rm -f *.o $(SERVER)

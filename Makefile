all:app

app: main.cpp
	g++ -o app main.cpp -lssl -lcrypto -lncurses

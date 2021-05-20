all: client.cpp server.cpp protocol.pb.h protocol.pb.cc errors.h
	make clean
	make client
	make server

clean:
	rm client
	rm server

client: client.cpp protocol.pb.h protocol.pb.cc errors.h
	g++ client.cpp errors.h protocol.pb.h protocol.pb.cc `pkg-config --cflags --libs protobuf` -o client


server: server.cpp protocol.pb.h protocol.pb.cc errors.h
	g++ server.cpp errors.h protocol.pb.h protocol.pb.cc `pkg-config --cflags --libs protobuf` -o server
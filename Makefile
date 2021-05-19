all: client.cpp server.cpp protocol.pb.h protocol.pb.cc
	g++ client.cpp protocol.pb.h protocol.pb.cc `pkg-config --cflags --libs protobuf` -o client
	# g++ server.cpp -o server
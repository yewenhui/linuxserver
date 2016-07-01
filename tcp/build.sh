#!/bin/bash

# PATH=/usr/local/bin:/usr/bin:/bin:/usr/local/sbin:/usr/sbin:/sbin:/usr/local/go/bin:/usr/local/jdk1.7.0_45/bin:/home/leonye/bin
# export PATH

SRCS="Acceptor.cpp InetAddress.cpp Socket.cpp TcpStream.cpp"

g++ -std=c++0x -Wall -Wextra -g -O2 $SRCS Tcp_Test.cpp -o tcp_unittest -lboost_program_options
#g++-4.7 -std=c++0x -Wall -Wextra -g -O2 $SRCS Tcp_Test.cpp -o tcp_unittest
# g++ -std=c++0x -Wall -Wextra -g -O2 Acceptor.cpp InetAddress.cpp Socket.cpp TcpStream.cpp Tcp_Test.cpp -o tcp_unittest -lboost_program_options

exit 0

CXX = g++
CFLAGS = -std=c++17 -O2 -Wall -g 

TARGET = server
OBJS = ../code/coder/*.cpp ../code/pool/*.cpp ../code/timer/*.cpp \
       ../code/http/*.cpp ../code/server/*.cpp ../code/log/*.cpp \
       ../code/buffer/*.cpp ../code/json/*.cpp ../code/main.cpp

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ../bin/$(TARGET)  -pthread -lmysqlclient

clean:
	rm -rf ../bin/$(OBJS) $(TARGET)





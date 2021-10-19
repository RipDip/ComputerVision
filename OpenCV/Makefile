# the compiler: gcc for C program, define as g++ for C++
  CC = g++
  VERSION = -std=c++11
  # compiler flags:
  #  -g     - this flag adds debugging information to the executable file
  #  -Wall  - this flag is used to turn on most compiler warnings
  LIBS  = `pkg-config --cflags --libs opencv`
  CFLAGS = -g -Wall
  # The build target
  TARGET = main
  all: $(TARGET).o
  $(TARGET): $(TARGET).cpp
		@echo "\nCode wird kompiliert..."
		$(CC) $(CFLAGS) $(VERSION) -o $(TARGET) $(TARGET).cpp $(LIBS)

  clean:
	   $(RM) $(TARGET)

  run: $(TARGET)
	  ./$(TARGET)

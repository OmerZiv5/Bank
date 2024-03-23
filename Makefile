# Makefile for the bank program
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Werror -pedantic-errors -DNDEBUG -pthread
#-fsanitize=thread
CXXLINK = $(CXX)
OBJS = main.o
RM = rm -f
# Creating the executable
Bank: $(OBJS)
	$(CXXLINK) -o Bank $(OBJS) -pthread
#-fsanitize=thread
# Creating the object files
main.o: ATMfunctions.cpp bank.cpp classes.h main.cpp
# Cleaning old files before new make
clean:
	$(RM) $(TARGET) *.o *~ "#"* core.* Bank

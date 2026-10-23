CXX = g++
CXXFLAGS = -Wall -std=c++11

myshell: command.cc tokenizer.cc command.h tokenizer.h
	$(CXX) $(CXXFLAGS) command.cc tokenizer.cc -o myshell

clean:
	rm -f myshell

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = zombie_sim
SRCS = main.cpp Virus.cpp ZombieBehavior.cpp Region.cpp Simulation.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

# Dependencies
main.o: main.cpp Virus.h ZombieBehavior.h Region.h Simulation.h
Virus.o: Virus.cpp Virus.h
ZombieBehavior.o: ZombieBehavior.cpp ZombieBehavior.h
Region.o: Region.cpp Region.h
Simulation.o: Simulation.cpp Simulation.h Virus.h ZombieBehavior.h Region.h

.PHONY: all clean

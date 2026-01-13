CXX = g++
CXXFLAGS = -std=c++17 -pthread -Wall

all: port_captain ship_captain passenger

port_captain: port_captain.cpp PortCaptain.cpp IPCManager.cpp IPCManager.hpp common.hpp
	$(CXX) $(CXXFLAGS) port_captain.cpp PortCaptain.cpp IPCManager.cpp -o port_captain

ship_captain: ship_captain.cpp ShipCaptain.cpp ShipCaptain.hpp common.hpp
	$(CXX) $(CXXFLAGS) ship_captain.cpp ShipCaptain.cpp -o ship_captain

passenger: passenger.cpp pas.cpp pas.hpp common.hpp
	$(CXX) $(CXXFLAGS) passenger.cpp pas.cpp -o passenger

clean:
	rm -f port_captain ship_captain passenger rejs
CXX = g++
CXXFLAGS = -std=c++17 -pthread -Wall -I. -Iinterfaces -Iinfrastructure -Idomain

INFRA_SOURCES = infrastructure/BridgeState.cpp \
                infrastructure/ShipState.cpp \
                infrastructure/MessageService.cpp \
                IPCManager.cpp

DOMAIN_SOURCES = domain/Passenger.cpp \
                 domain/ShipCaptain.cpp \
                 domain/PortCaptain.cpp

all: port_captain ship_captain passenger

port_captain: port_captain.cpp $(DOMAIN_SOURCES) $(INFRA_SOURCES)
	$(CXX) $(CXXFLAGS) port_captain.cpp $(DOMAIN_SOURCES) $(INFRA_SOURCES) -o port_captain

ship_captain: ship_captain.cpp $(DOMAIN_SOURCES) $(INFRA_SOURCES)
	$(CXX) $(CXXFLAGS) ship_captain.cpp $(DOMAIN_SOURCES) $(INFRA_SOURCES) -o ship_captain

passenger: passenger.cpp $(DOMAIN_SOURCES) $(INFRA_SOURCES)
	$(CXX) $(CXXFLAGS) passenger.cpp $(DOMAIN_SOURCES) $(INFRA_SOURCES) -o passenger

clean:
	rm -f port_captain ship_captain passenger rejs *.o
	rm -f infrastructure/*.o domain/*.o
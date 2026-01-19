#ifndef PASSENGER_HPP
#define PASSENGER_HPP

#include "IBridgeAccess.hpp"
#include "IPassengerShipAccess.hpp"
#include "IMessageService.hpp"
#include "common.hpp"
#include <memory>

class Passenger {
public:
    Passenger(int id,
              std::shared_ptr<IBridgeAccess> bridge,
              std::shared_ptr<IPassengerShipAccess> ship,
              std::shared_ptr<IMessageService> messaging);
    
    void run();

private:
    bool waitForBoardingAllowed();
    bool tryToBoard();
    void handleUnloading();
    
    int passenger_id_;
    std::shared_ptr<IBridgeAccess> bridge_;
    std::shared_ptr<IPassengerShipAccess> ship_;
    std::shared_ptr<IMessageService> messaging_;
};

#endif // PASSENGER_HPP
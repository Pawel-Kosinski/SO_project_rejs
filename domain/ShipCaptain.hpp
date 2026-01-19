#ifndef SHIP_CAPTAIN_HPP
#define SHIP_CAPTAIN_HPP

#include "IShipCaptainControl.hpp"
#include "IMessageService.hpp"
#include "common.hpp"
#include <memory>
#include <atomic>

class ShipCaptain {
public:
    ShipCaptain(std::shared_ptr<IShipCaptainControl> ship,
                std::shared_ptr<IMessageService> messaging);
    
    void run();

private:
    void performVoyage();
    void waitForBoardingCompletion();
    void waitForBridgeToClear();
    void waitForShipToEmpty();
    
    int voyages_;
    std::shared_ptr<IShipCaptainControl> ship_;
    std::shared_ptr<IMessageService> messaging_;
};

#endif // SHIP_CAPTAIN_HPP
#ifndef PORT_CAPTAIN_HPP
#define PORT_CAPTAIN_HPP

#include "IPortCaptainShipControl.hpp"
#include "IMessageService.hpp"
#include "common.hpp"
#include <memory>

class PortCaptain {
public:
    PortCaptain(std::shared_ptr<IPortCaptainShipControl> ship,
                std::shared_ptr<IMessageService> messaging);
    
    void run();
    bool hasVoyagesEnded() const { return voyages_ended_; }

private:
    void handleStartBoardingMessage();
    void handleStartUnloadingMessage();
    
    std::shared_ptr<IPortCaptainShipControl> ship_;
    std::shared_ptr<IMessageService> messaging_;
    bool voyages_ended_;
};

#endif // PORT_CAPTAIN_HPP
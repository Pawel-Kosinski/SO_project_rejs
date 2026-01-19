#include "PortCaptain.hpp"

PortCaptain::PortCaptain(std::shared_ptr<IPortCaptainShipControl> ship,
                         std::shared_ptr<IMessageService> messaging)
    : ship_(ship), messaging_(messaging), voyages_ended_(false) {}

void PortCaptain::handleStartBoardingMessage() {
    std::cout << RED << "Kapitan Portu: Otrzymano sygnał rozpoczęcia załadunku.\n" << RESET;
    sleep_ms(std::rand() % 2000 + 2000);
    
    ship_->allowBoarding();
    std::cout << RED << "Kapitan Portu: Wysłano komunikat do pasażerów o rozpoczęciu załadunku.\n" << RESET;
}

void PortCaptain::handleStartUnloadingMessage() {
    std::cout << RED << "Kapitan Portu: Otrzymano sygnał rozpoczęcia rozładunku.\n" << RESET;
    
    messaging_->sendUnloadingAllowedMessages(N);
    std::cout << RED << "Kapitan Portu: Wysłano komunikat do pasażerów o rozpoczęciu rozładunku.\n" << RESET;
    
    if (ship_->getVoyageNumber() >= R) {
        std::cout << RED << "Kapitan Portu: Osiągnięto maksymalną liczbę rejsów.\n" << RESET;
        voyages_ended_ = true;
    }
}

void PortCaptain::run() {
    std::cout << RED << "Kapitan Portu: Rozpoczynam pracę.\n" << RESET;
    
    while (!ship_->isTerminated() && !voyages_ended_) {
        msgbuf_custom msg = messaging_->waitForMessage();
        
        if (msg.mtype == MSG_TYPE_START_BOARDING) {
            handleStartBoardingMessage();
        } else if (msg.mtype == MSG_TYPE_START_UNLOADING) {
            handleStartUnloadingMessage();
        }
    }
}
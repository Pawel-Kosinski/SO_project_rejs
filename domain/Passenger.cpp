// domain/Passenger.cpp
#include "Passenger.hpp"

Passenger::Passenger(int id,
                     std::shared_ptr<IBridgeAccess> bridge,
                     std::shared_ptr<IPassengerShipAccess> ship,
                     std::shared_ptr<IMessageService> messaging)
    : passenger_id_(id), bridge_(bridge), ship_(ship), messaging_(messaging) {}

bool Passenger::waitForBoardingAllowed() {
    sleep_ms(std::rand() % 5000);
    while (true) {
        if (ship_->canBoard()) {
            sleep_ms(1500 + (std::rand() % 4000));
            bridge_->enter(passenger_id_);
            return true; 
        }
        
        if (ship_->getVoyageNumber() > R) {
            return false; 
        }
        
        sleep(1);
    }
}

bool Passenger::tryToBoard() {
    sleep_ms(std::rand() % 5000);
    if (ship_->canBoard() && ship_->hasSpace()) {
        sleep_ms(3000);
        bridge_->exit(passenger_id_);
        ship_->board(passenger_id_);
        return true;  
    } else {
        bridge_->exit(passenger_id_);
        sleep_ms(3000); 
        return false;  
    }
}

void Passenger::handleUnloading() {
    msgbuf_custom msg = messaging_->waitForUnloadingMessage();
    
    if (strcmp(msg.mtext, "Unloading is now allowed.") == 0 or strcmp(msg.mtext, "Start unloading") == 0 or strcmp(msg.mtext, "Abort voyages") == 0) {
        sleep_ms(1000 + (std::rand() % 5000));
        ship_->disembark(passenger_id_);
        sleep_ms(std::rand() % 4000);
        bridge_->enter(passenger_id_);
        sleep_ms(1000 + (std::rand() % 3000));
        bridge_->exit(passenger_id_);
    } 
}

void Passenger::run() {
    std::cout << GREEN << "Pasażer " << passenger_id_ << " przybył do portu.\n" << RESET;
    sleep_ms(1000 + (std::rand() % 2000));
    
    while (true) {
        if (!waitForBoardingAllowed()) {
            break;  
        }
        
        if (tryToBoard()) {
            sleep_ms(1000 + (std::rand() % 1000));
            handleUnloading();
        } else {
            continue;
        }
        
        if (ship_->getVoyageNumber() >= R) {
            break;
        }
    }
    
    std::cout << GREEN << "Pasażer " << passenger_id_ << " idzie do domu.\n" << RESET;
}
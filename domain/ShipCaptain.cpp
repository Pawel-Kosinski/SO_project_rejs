#include "ShipCaptain.hpp"

//extern std::atomic<bool> g_alarm_fired;

ShipCaptain::ShipCaptain(std::shared_ptr<IShipCaptainControl> ship,
                         std::shared_ptr<IMessageService> messaging)
    : voyages_(0), ship_(ship), messaging_(messaging) {}

void ShipCaptain::waitForBoardingCompletion() {
    alarm(T1_SEC);  // Ustaw budzik
    
    while (true) {
        if (ship_->getFlag()) {
            std::cout << BLUE << "Kapitan Statku: Czas na załadunek minął. Kończę załadunek.\n" << RESET;
            break;
        }

        if (ship_->isLoadingFinished()) {
            std::cout << BLUE << "Kapitan Statku: Załadunek zakończony przez Kapitana Portu.\n" << RESET;
            alarm(0);  
            break;
        }

        if (ship_->isFull()) {
            std::cout << BLUE << "Kapitan Statku: Statek pełny. Przedwczesne zakończenie załadunku.\n" << RESET;
            alarm(0);
            break;
        }
        
        if (ship_->isTerminated()) {
            std::cout << BLUE << "Kapitan Statku: Sygnał o zakończeniu rejsów.\n" << RESET;
            alarm(0);
            break;
        }
        
        sleep(1);
    }
}

void ShipCaptain::waitForBridgeToClear() {
    if (ship_->getPassengersOnBridge() > 0) {
        std::cout << BLUE << "Kapitan Statku: Czekam, aż pasażerowie opuszczą mostek.\n" << RESET;
        while (ship_->getPassengersOnBridge() > 0) {
            sleep(1);
        }
    }
}

void ShipCaptain::waitForShipToEmpty() {
    while (ship_->getPassengersOnBoard() > 0) {
        sleep(1);
    }
    std::cout << BLUE << "Kapitan Statku: Wszyscy pasażerowie opuścili statek.\n" << RESET;
}

void ShipCaptain::performVoyage() {
    voyages_++;
    ship_->setVoyageNumber(voyages_);
    
    ship_->startBoarding();
    std::cout << BLUE << "Kapitan Statku: Rozpoczęcie załadunku do rejsu " << voyages_ << ".\n" << RESET;
    messaging_->sendStartBoardingMessage();
    
    waitForBoardingCompletion();
    
    ship_->finishBoarding();

    if (!ship_->isTerminated())
    {
    waitForBridgeToClear();

    ship_->startVoyage();
    std::cout << BLUE << "Kapitan Statku: Rejs " << voyages_ << " w trakcie.\n" << RESET;
    sleep(T2_SEC);
    
    ship_->finishVoyage();
    std::cout << BLUE << "Kapitan Statku: Rejs " << voyages_ << " zakończony. Pasażerowie mogą opuścić statek.\n" << RESET;
    messaging_->sendStartUnloadingMessage();
    ship_->startUnloading();
    }
    
    waitForShipToEmpty();
    
    waitForBridgeToClear();
    
    std::cout << BLUE << "Kapitan Statku: Rozładunek zakończony dla rejsu " << voyages_ << ".\n" << RESET;
}

void ShipCaptain::run() {
    std::cout << BLUE << "Kapitan Statku: Rozpoczynam pracę.\n" << RESET;
    
    while (voyages_ < R && !ship_->isTerminated()) {
        performVoyage();
    }
    
    voyages_++;
    ship_->setVoyageNumber(voyages_);
    
    std::cout << BLUE << "Kapitan Statku: Kończę pracę.\n" << RESET;
}
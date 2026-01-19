#ifndef SHIP_STATE_HPP
#define SHIP_STATE_HPP

#include "IPassengerShipAccess.hpp"
#include "IShipCaptainControl.hpp"
#include "IPortCaptainShipControl.hpp"
#include "common.hpp"

class ShipState : public IPassengerShipAccess,
                  public IShipCaptainControl,
                  public IPortCaptainShipControl {
public:
    ShipState(int semid, SharedData* shared_data);
    
    // Implementacja IPassengerShipAccess
    bool canBoard() const override;
    bool hasSpace() const override;
    void board(int passenger_id) override;
    void disembark(int passenger_id) override;
    int getVoyageNumber() const override;
    
    // Implementacja IShipCaptainControl
    void startBoarding() override;
    void finishBoarding() override;
    void startVoyage() override;
    void finishVoyage() override;
    void startUnloading() override;
    int getPassengersOnBoard() const override;
    int getPassengersOnBridge() const override;
    bool isFull() const override;
    bool isTerminated() const override;
    void setVoyageNumber(int voyage_num) override;
    void setFlag() override;
    bool getFlag() override;
    bool isLoadingFinished() override;
    
    // Implementacja IPortCaptainShipControl
    void allowBoarding() override;
    void forceFinishBoarding() override;
    void initiateEmergencyStop() override;

private:
    void sem_p();
    void sem_v();
    
    int semid_;
    SharedData* shared_data_;
};

#endif // SHIP_STATE_HPP
#ifndef BRIDGE_STATE_HPP
#define BRIDGE_STATE_HPP

#include "IBridgeAccess.hpp"
#include "common.hpp"

class BridgeState : public IBridgeAccess {
public:
    BridgeState(int semid, SharedData* shared_data);
    
    void enter(int passenger_id) override;
    void exit(int passenger_id) override;
    int getPassengersOnBridge() const override;

private:
    void sem_p();
    void sem_v();
    
    int semid_;
    SharedData* shared_data_;
};

#endif // BRIDGE_STATE_HPP
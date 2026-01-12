#include "common.hpp"

class ShipCaptain {
public:
    ShipCaptain(int semid, int msgid, SharedData *sd);

    void sendMessage(long mtype, const char *text);
    void startBoarding();
    void waitWhileBoarding();
    void finishBoarding();
    void startVoyage();
    void finishVoyage();
    void shipUnload();
    void bridgeUnload();
    void end();
    int voyages = 0;

private:        
    int semid;
    int msgid;
    SharedData *shared_data;
};
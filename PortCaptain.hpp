#include "common.hpp"

class PortCaptain {
public:
    PortCaptain(int semid, int msgid, SharedData *sd);

    msgbuf_custom waitForMessage();
    void startBoarding();
    void startUnloading();
    void endVoyages();

    int semid;
    int msgid;
    SharedData *shared_data;
    bool voyages_ended = false;
};
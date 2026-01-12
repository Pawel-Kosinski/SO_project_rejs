#ifndef PAS_HPP
#define PAS_HPP

#include "common.hpp"

class Passenger {
public:
    Passenger(int id, int semid, int msgid, SharedData *sd);

    bool isBoardingAllowed(); // true = idź dalej, false = koniec pracy
    bool tryToBoard();        // true = wszedłem, false = wyrzucili mnie

    void tryToUnload();
    void voyageAbort();

private:
    void enter_bridge();
    void exit_bridge();
    void enter_ship();
    void exit_ship();
    
    // Helpery do semaforów (prywatne)
    void sem_p(int semnum);
    void sem_v(int semnum);

    int passenger_id;
    int semid;
    int msgid;
    SharedData *sd;
};

#endif
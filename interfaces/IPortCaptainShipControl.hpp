// interfaces/IPortCaptainShipControl.hpp
#ifndef IPORT_CAPTAIN_SHIP_CONTROL_HPP
#define IPORT_CAPTAIN_SHIP_CONTROL_HPP

class IPortCaptainShipControl {
public:
    virtual ~IPortCaptainShipControl() = default;
    
    // Kontrola załadunku przez kapitana portu
    virtual void allowBoarding() = 0;
    virtual void forceFinishBoarding() = 0;
    
    // Awaryjne zatrzymanie
    virtual void initiateEmergencyStop() = 0;
    
    // Sprawdzanie stanu
    virtual int getVoyageNumber() const = 0;
    virtual bool isTerminated() const = 0;
};

#endif // IPORT_CAPTAIN_SHIP_CONTROL_HPP
#ifndef ISHIP_CAPTAIN_CONTROL_HPP
#define ISHIP_CAPTAIN_CONTROL_HPP

class IShipCaptainControl {
public:
    virtual ~IShipCaptainControl() = default;
    
    // Kontrola załadunku
    virtual void startBoarding() = 0;
    virtual void finishBoarding() = 0;
    
    // Kontrola rejsu
    virtual void startVoyage() = 0;
    virtual void finishVoyage() = 0;
    
    // Kontrola rozładunku
    virtual void startUnloading() = 0;
    
    // Sprawdzanie stanu
    virtual int getPassengersOnBoard() const = 0;
    virtual int getPassengersOnBridge() const = 0;
    virtual bool isFull() const = 0;
    virtual bool isTerminated() const = 0;
    virtual void setFlag() = 0;
    virtual bool getFlag() = 0;
    virtual bool isLoadingFinished() = 0;
    
    // Numer rejsu
    virtual void setVoyageNumber(int voyage_num) = 0;
};

#endif // ISHIP_CAPTAIN_CONTROL_HPP
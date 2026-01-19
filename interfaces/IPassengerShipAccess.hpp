#ifndef IPASSENGER_SHIP_ACCESS_HPP
#define IPASSENGER_SHIP_ACCESS_HPP

class IPassengerShipAccess {
public:
    virtual ~IPassengerShipAccess() = default;
    
    // Sprawdzanie stanu
    virtual bool canBoard() const = 0;
    virtual bool hasSpace() const = 0;
    
    // Akcje pasażera
    virtual void board(int passenger_id) = 0;
    virtual void disembark(int passenger_id) = 0;
    
    // Sprawdzanie końca rejsów
    virtual int getVoyageNumber() const = 0;
};

#endif // IPASSENGER_SHIP_ACCESS_HPP
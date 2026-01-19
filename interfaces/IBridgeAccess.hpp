#ifndef IBRIDGE_ACCESS_HPP
#define IBRIDGE_ACCESS_HPP

class IBridgeAccess {
public:
    virtual ~IBridgeAccess() = default;
    
    virtual void enter(int passenger_id) = 0;
    virtual void exit(int passenger_id) = 0;
    virtual int getPassengersOnBridge() const = 0;
};

#endif // IBRIDGE_ACCESS_HPP
#ifndef IMESSAGE_SERVICE_HPP
#define IMESSAGE_SERVICE_HPP

#include "common.hpp"

class IMessageService {
public:
    virtual ~IMessageService() = default;
    
    // Dla kapitana statku
    virtual void sendStartBoardingMessage() = 0;
    virtual void sendStartUnloadingMessage() = 0;
    
    // Dla kapitana portu
    virtual msgbuf_custom waitForMessage() = 0;
    virtual void sendUnloadingAllowedMessages(int count) = 0;
    virtual void sendAbortMessages(int count) = 0;
    
    // Dla pasażerów
    virtual msgbuf_custom waitForUnloadingMessage() = 0;
    virtual bool isBoardingAllowed() const = 0;
};

#endif // IMESSAGE_SERVICE_HPP
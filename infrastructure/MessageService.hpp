#ifndef MESSAGE_SERVICE_HPP
#define MESSAGE_SERVICE_HPP

#include "IMessageService.hpp"
#include "common.hpp"

class MessageService : public IMessageService {
public:
    MessageService(int msgid, SharedData* shared_data);

    void sendStartBoardingMessage() override;
    void sendStartUnloadingMessage() override;
    
    msgbuf_custom waitForMessage() override;
    void sendUnloadingAllowedMessages(int count) override;
    void sendAbortMessages(int count) override;
    
    msgbuf_custom waitForUnloadingMessage() override;
    bool isBoardingAllowed() const override;

private:
    void sendMessage(long mtype, const char* text);
    
    int msgid_;
    SharedData* shared_data_;
};

#endif // MESSAGE_SERVICE_HPP
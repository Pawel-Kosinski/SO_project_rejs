#include "IPCManager.hpp"
#include "domain/ShipCaptain.hpp"
#include "infrastructure/ShipState.hpp"
#include "infrastructure/MessageService.hpp"
#include <memory>
#include <atomic>

std::shared_ptr<IShipCaptainControl> g_ship_captain_control = nullptr;

void alarmHandler(int sig) {
    g_ship_captain_control->setFlag();
    std::cout << BLUE << "Kapitan Statku: Czas na załadunek upłynął.\n" << RESET;
}

int main() {
    signal(SIGALRM, alarmHandler);
    
    //  1. Podłącz się do istniejących zasobów 
    IPCManager ipc_manager;
    
    try {
        ipc_manager.attach();
    } catch (const std::exception& e) {
        std::cerr << "Błąd podłączenia IPC: " << e.what() << std::endl;
        std::cerr << "Upewnij się, że port_captain jest uruchomiony!\n";
        return EXIT_FAILURE;
    }
    
    //  2. Stwórz infrastrukturę (adaptery) 
    auto ship_state = std::make_shared<ShipState>(
        ipc_manager.getSemaphoreId(),
        ipc_manager.getSharedData()
    );
    
    auto messaging = std::make_shared<MessageService>(
        ipc_manager.getMessageQueueId(),
        ipc_manager.getSharedData()
    );
    
    //  3. Rzutuj na interfejs kapitana statku 
    std::shared_ptr<IShipCaptainControl> captain_ship_control = ship_state;
    
    // Ustaw globalny wskaźnik dla alarm handlera
    g_ship_captain_control = captain_ship_control;
    
    //  4. Uruchom logikę Kapitana Statku
    ShipCaptain ship_captain(captain_ship_control, messaging);
    ship_captain.run();
    
    return 0;
}
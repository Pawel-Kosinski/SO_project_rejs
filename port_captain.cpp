#include "IPCManager.hpp"
#include "domain/PortCaptain.hpp"
#include "infrastructure/ShipState.hpp"
#include "infrastructure/MessageService.hpp"
#include <memory>

// Globalny wskaźnik dla handlerów sygnałów
std::shared_ptr<IPortCaptainShipControl> g_ship_control = nullptr;
std::shared_ptr<IMessageService> g_messaging = nullptr;

void handle_sighup(int sig) {
    if (!g_ship_control) return;
    std::cout << RED << "Kapitan Portu: SIGHUP - zakończam przedwcześnie załadunek.\n" << RESET;
    g_ship_control->forceFinishBoarding();
}

void handle_sigabrt(int sig) {
    if (!g_ship_control || !g_messaging) return;
    
    std::cout << RED << "SIGABRT received: Przerywam rejsy na dany dzień.\n" << RESET;
    
    // Zainicjuj awaryjne zatrzymanie
    g_ship_control->initiateEmergencyStop();
    
    // Wyślij komunikaty abort do pasażerów
    g_messaging->sendAbortMessages(N);
}

int main() {
    std::srand(std::time(nullptr));
    
    IPCManager ipc_manager;
    
    try {
        ipc_manager.initialize();
    } catch (const std::exception& e) {
        std::cerr << "Błąd inicjalizacji IPC: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    // ========== 2. Stwórz infrastrukturę (adaptery) ==========
    auto ship_state = std::make_shared<ShipState>(
        ipc_manager.getSemaphoreId(),
        ipc_manager.getSharedData()
    );
    
    auto messaging = std::make_shared<MessageService>(
        ipc_manager.getMessageQueueId(),
        ipc_manager.getSharedData()
    );
    
    // ========== 3. Rzutuj na interfejs kapitana portu ==========
    std::shared_ptr<IPortCaptainShipControl> port_ship_control = ship_state;
    
    // Ustaw globalne wskaźniki dla handlerów sygnałów
    g_ship_control = port_ship_control;
    g_messaging = messaging;
    
    // ========== 4. Rejestracja sygnałów ==========
    struct sigaction sa_hup;
    sa_hup.sa_handler = handle_sighup;
    sa_hup.sa_flags = SA_RESTART;
    sigemptyset(&sa_hup.sa_mask);
    if (sigaction(SIGHUP, &sa_hup, nullptr) == -1) {
        perror("sigaction SIGHUP");
        exit(EXIT_FAILURE);
    }
    
    struct sigaction sa_abrt;
    sa_abrt.sa_handler = handle_sigabrt;
    sa_abrt.sa_flags = SA_RESTART;
    sigemptyset(&sa_abrt.sa_mask);
    if (sigaction(SIGABRT, &sa_abrt, nullptr) == -1) {
        perror("sigaction SIGABRT");
        exit(EXIT_FAILURE);
    }
    
    // ========== 5. Uruchom logikę Kapitana Portu ==========
    PortCaptain port_captain(port_ship_control, messaging);
    port_captain.run();
    
    // ========== 6. Poczekaj aż wszystkie procesy się zakończą ==========
    SharedData* shared_data = ipc_manager.getSharedData();
    while (true) {
        pthread_mutex_lock(&shared_data->mutex);
        int p = shared_data->passengers;
        pthread_mutex_unlock(&shared_data->mutex);
        
        if (p == 0) break;
        sleep(1);
    }
    
    // ========== 7. Cleanup zasobów ==========
    ipc_manager.cleanup();
    
    std::cout << RED << "Kapitan Portu: Koniec dnia, zamykam port.\n" << RESET;
    return 0;
}
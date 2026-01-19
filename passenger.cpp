#include "IPCManager.hpp"
#include "domain/Passenger.hpp"
#include "infrastructure/BridgeState.hpp"
#include "infrastructure/ShipState.hpp"
#include "infrastructure/MessageService.hpp"
#include <memory>
#include <vector>
#include <thread>

struct PassengerArgs {
    int passenger_id;
    std::shared_ptr<IBridgeAccess> bridge;
    std::shared_ptr<IPassengerShipAccess> ship;
    std::shared_ptr<IMessageService> messaging;
};

void PassengerLogic(PassengerArgs args) {
    Passenger passenger(args.passenger_id, args.bridge, args.ship, args.messaging);
    passenger.run();
}

int main() {
    std::srand(std::time(nullptr) ^ getpid());
    
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
    auto bridge_state = std::make_shared<BridgeState>(
        ipc_manager.getSemaphoreId(),
        ipc_manager.getSharedData()
    );
    
    auto ship_state = std::make_shared<ShipState>(
        ipc_manager.getSemaphoreId(),
        ipc_manager.getSharedData()
    );
    
    auto messaging = std::make_shared<MessageService>(
        ipc_manager.getMessageQueueId(),
        ipc_manager.getSharedData()
    );
    
    //  3. Rzutuj na interfejsy pasażera 
    std::shared_ptr<IBridgeAccess> bridge = bridge_state;
    std::shared_ptr<IPassengerShipAccess> passenger_ship = ship_state;
    
    //  4. Uruchom wątki pasażerów 
    std::vector<std::thread> threads;
    threads.reserve(NUM_PASSENGERS);
    
    std::cout << GREEN << "Generowanie pasażerów...\n" << RESET;
    
    SharedData* shared_data = ipc_manager.getSharedData();
    
    for (int i = 0; i < NUM_PASSENGERS; i++) {
        PassengerArgs args = {i + 1, bridge, passenger_ship, messaging};
        threads.emplace_back(PassengerLogic, args);
        sleep_ms(200 + (std::rand() % 300));
        
        pthread_mutex_lock(&shared_data->mutex);
        shared_data->passengers++;
        int terminate_now = shared_data->terminate;
        pthread_mutex_unlock(&shared_data->mutex);
        
        if (terminate_now == 1) break;
    }
    
    // Czekaj na zakończenie wszystkich wątków
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }
    
    // Wyzeruj licznik pasażerów
    pthread_mutex_lock(&shared_data->mutex);
    shared_data->passengers = 0;
    pthread_mutex_unlock(&shared_data->mutex);
    
    std::cout << GREEN << "Wszyscy pasażerowie zakończyli pracę.\n" << RESET;
    
    return 0;
}
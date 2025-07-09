/*
** EPITECH PROJECT, 2025
** Zappy [WSL: Ubuntu]
** File description:
** NetworkThread
*/

#ifndef NETWORKTHREAD_HPP_
#define NETWORKTHREAD_HPP_

#include <thread>
#include <memory>
#include <atomic>
#include <string>
#include "zappy/network/utils/NetworkManager.hpp"
#include "zappy/game/world/SharedGameState.hpp"

namespace zappy::network::utils {
class NetworkThread {
private:
    std::unique_ptr<std::thread> _thread;
    std::unique_ptr<zappy::network::utils::NetworkManager> _network_manager;
    std::shared_ptr<zappy::game::world::SharedGameState> _game_state;
    std::atomic<bool> _running{false};
    
    // Configuration de connexion
    std::string _host;
    int _port;
    
    // Gestion des erreurs
    std::atomic<bool> _connection_failed{false};
    std::string _last_error;

public:
    NetworkThread(const std::string& host, int port, 
        std::shared_ptr<zappy::game::world::SharedGameState> game_state);
    ~NetworkThread();
    
    // Gestion du thread
    bool start();
    void stop();
    void join();
    bool isRunning() const { return _running.load(); }
    
    // État de la connexion
    bool isConnected() const;
    bool hasConnectionFailed() const { return _connection_failed.load(); }
    std::string getLastError() const { return _last_error; }
    
    // Interface pour envoyer des commandes
    void requestInitialData();
    void requestMapSize();
    void requestMapContent();
    void requestTeamNames();
    void requestPlayerInfo(int player_id);
    void setTimeUnit(int time_unit);
    
private:
    // Fonction principale du thread
    void networkThread();
    
    // Handle network events
    bool handleNetworkEvents();
    
    // Gestion des erreurs
    void setError(const std::string& error);
    void clearError();
};
}
#endif /* !NETWORKTHREAD_HPP_ */
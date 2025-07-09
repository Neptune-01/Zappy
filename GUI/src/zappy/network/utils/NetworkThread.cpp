/*
** EPITECH PROJECT, 2025
** Zappy [WSL: Ubuntu]
** File description:
** NetworkThread
*/

#include "zappy/network/utils/NetworkThread.hpp"
#include <iostream>
#include <chrono>
#include <thread>

zappy::network::utils::NetworkThread::NetworkThread(const std::string& host, int port, 
    std::shared_ptr<zappy::game::world::SharedGameState> game_state)
    : _game_state(game_state), _host(host), _port(port) {
    
    _network_manager = std::make_unique<NetworkManager>(host, port, game_state);
}

zappy::network::utils::NetworkThread::~NetworkThread() {
    stop();
    join();
}

bool zappy::network::utils::NetworkThread::start() {
    if (_running.load()) {
        std::cerr << "[NetworkThread] Already running" << std::endl;
        return false;
    }
    
    clearError();
    
    try {
        _thread = std::make_unique<std::thread>(&zappy::network::utils::NetworkThread::networkThread, this);
        _running = true;
        
#ifndef NDEBUG
        std::cout << "[NetworkThread] Started network thread" << std::endl;
#endif
        return true;
    } catch (const std::exception& e) {
        setError("Failed to start network thread: " + std::string(e.what()));
        return false;
    }
}

void zappy::network::utils::NetworkThread::stop() {
    if (!_running.load()) {
        return;
    }
    
#ifndef NDEBUG
    std::cout << "[NetworkThread] Stopping network thread" << std::endl;
#endif
    
    _running = false;
    
    if (_network_manager) {
        _network_manager->requestStop();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

void zappy::network::utils::NetworkThread::join() {
    if (_thread && _thread->joinable()) {
        _thread->join();
        _thread.reset();
#ifndef NDEBUG
        std::cout << "[NetworkThread] Network thread joined" << std::endl;
#endif
    }
}

void zappy::network::utils::NetworkThread::networkThread() {
#ifndef NDEBUG
    std::cout << "[NetworkThread] Network thread started" << std::endl;
#endif
    
    try {
        // Phase 1: Connexion au serveur
        if (!_network_manager->connect()) {
            setError("Failed to connect to server " + _host + ":" + std::to_string(_port));
            _connection_failed = true;
            _running = false;
            return;
        }
        
#ifndef NDEBUG
        std::cout << "[NetworkThread] Connected to server" << std::endl;
#endif
        
        // Phase 2: Authentification comme client graphique
        if (!_network_manager->authenticateAsGraphic()) {
            setError("Failed to authenticate as GRAPHIC client");
            _connection_failed = true;
            _running = false;
            _network_manager->disconnect();
            return;
        }
        
#ifndef NDEBUG
        std::cout << "[NetworkThread] Authenticated as GRAPHIC client" << std::endl;
#endif
        
        // Phase 3: Attendre un peu où être sûr de recevoir les datas initiales
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        // Phase 4: Boucle principale avec timer pour la carte
        auto last_map_request = std::chrono::steady_clock::now();
        
        while (_running.load() && _network_manager->isConnected()) {
            if (!handleNetworkEvents()) {
                break;
            }
            
            auto now = std::chrono::steady_clock::now();
            auto time_since_last_request = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_map_request);
            
            if (time_since_last_request.count() >= 1000) {
                requestMapContent();
                last_map_request = now;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
    } catch (const std::exception& e) {
        setError("Network thread exception: " + std::string(e.what()));
        _connection_failed = true;
    }
    
    // Nettoyage
    if (_network_manager) {
        _network_manager->disconnect();
    }
    
    _running = false;
    _game_state->setConnected(false);
    
#ifndef NDEBUG
    std::cout << "[NetworkThread] Network thread ended" << std::endl;
#endif
}

void zappy::network::utils::NetworkThread::requestMapContent() {
    if (_network_manager && _network_manager->isConnected()) {
        _network_manager->requestMapContent();
    }
}

bool zappy::network::utils::NetworkThread::handleNetworkEvents() {
    for (int i = 0; i < 10 && _running.load(); ++i) {
        if (!_network_manager->isConnected()) {
            return false;
        }

        if (!_network_manager->processEvents()) {
            return false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    return true;
}

void zappy::network::utils::NetworkThread::setError(const std::string& error) {
    _last_error = error;
    std::cerr << "[NetworkThread ERROR] " << error << std::endl;
}

void zappy::network::utils::NetworkThread::clearError() {
    _last_error.clear();
    _connection_failed = false;
}
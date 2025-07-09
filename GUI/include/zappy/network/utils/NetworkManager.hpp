/*
** EPITECH PROJECT, 2025
** Zappy [WSL: Ubuntu]
** File description:
** NetworkManager
*/

#ifndef NETWORKMANAGER_HPP_
#define NETWORKMANAGER_HPP_

#include <string>
#include <thread>
#include <memory>
#include <poll.h>
#include <queue>
#include <mutex>
#include <atomic>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "zappy/game/world/SharedGameState.hpp"
#include "zappy/network/protocol/CommandParser.hpp"

namespace zappy::network::utils {
class NetworkManager {
private:
    // Configuration de connexion
    std::string _host;
    int _port;
    
    // Socket et polling
    int _socket_fd = -1;
    struct pollfd _poll_fds[1];
    
    // Buffers de communication
    std::string _receive_buffer;
    std::queue<std::string> _send_queue;
    std::mutex _send_mutex;
    
    // Gestion d'état
    bool _connected = false;
    bool _authenticated = false;
    std::atomic<bool> _should_stop{false};
    
    // Shared state avec le thread graphique
    std::shared_ptr<zappy::game::world::SharedGameState> _game_state;
    std::unique_ptr<zappy::network::protocol::CommandParser> _command_parser;
    
    // Configuration de timeout
    static constexpr int POLL_TIMEOUT_MS = 100;
    static constexpr int CONNECTION_TIMEOUT_S = 10;
    static constexpr size_t MAX_BUFFER_SIZE = 65536;

public:
    NetworkManager(const std::string& host, int port, 
        std::shared_ptr<zappy::game::world::SharedGameState> game_state);
    ~NetworkManager();
    
    // Gestion de la connexion
    bool connect();
    void disconnect();
    bool isConnected() const { return _connected; }
    
    // Arrêt forcé de la boucle réseau
    void requestStop() { _should_stop = true; }
    
    // Authentification client graphique
    bool authenticateAsGraphic();

    // Process network events (non-blocking)
    bool processEvents();
    
    // Envoi de commandes
    void sendCommand(const std::string& command);
    void requestMapSize();
    void requestMapContent();
    void requestTeamNames();
    void requestPlayerPosition(int player_id);
    void requestPlayerInventory(int player_id);
    void requestTimeUnit();
    void setTimeUnit(int time_unit);
    
private:
    // Gestion des sockets
    bool createSocket();
    bool connectSocket();
    void closeSocket();
    
    // Communication
    bool handlePollEvents();
    bool receiveData();
    bool sendPendingData();
    void processReceivedData();
    void processCompleteMessage(const std::string& message);
    void requestInitialGameState();
    void processPendingCommands();
    void processCommandParserPendingCommands();
    
    // Parsing et traitement
    std::vector<std::string> splitMessage(const std::string& message);
    
    // Utilitaires
    bool waitForConnection();
    std::string extractNextMessage();
    void logError(const std::string& message) const;
    void logDebug(const std::string& message) const;
};
}
#endif /* !NETWORKMANAGER_HPP_ */
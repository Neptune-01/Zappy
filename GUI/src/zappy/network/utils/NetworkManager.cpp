/*
** EPITECH PROJECT, 2025
** Zappy [WSL: Ubuntu]
** File description:
** NetworkManager
*/

#include "zappy/network/utils/NetworkManager.hpp"

zappy::network::utils::NetworkManager::NetworkManager(const std::string& host, int port, 
    std::shared_ptr<zappy::game::world::SharedGameState> game_state)
    : _host(host), _port(port), _game_state(game_state) {
    
    _command_parser = std::make_unique<zappy::network::protocol::CommandParser>(game_state);
    
    // Initialiser la structure pollfd
    _poll_fds[0].fd = -1;
    _poll_fds[0].events = POLLIN | POLLOUT;
    _poll_fds[0].revents = 0;
}

zappy::network::utils::NetworkManager::~NetworkManager() {
    disconnect();
}

bool zappy::network::utils::NetworkManager::connect() {
#ifndef NDEBUG
    logDebug("Attempting to connect to " + _host + ":" + std::to_string(_port));
#endif
    
    if (!createSocket()) {
        return false;
    }
    
    if (!connectSocket()) {
        closeSocket();
        return false;
    }
    
    _connected = true;
    _poll_fds[0].fd = _socket_fd;
    
#ifndef NDEBUG
    logDebug("Successfully connected to server");
#endif
    return true;
}

void zappy::network::utils::NetworkManager::disconnect() {
    if (_connected) {
#ifndef NDEBUG
        logDebug("Disconnecting from server");
#endif
        _connected = false;
        _authenticated = false;
        closeSocket();
        _game_state->setConnected(false);
    }
}

bool zappy::network::utils::NetworkManager::authenticateAsGraphic() {
    if (!_connected) {
        logError("Cannot authenticate: not connected");
        return false;
    }
    
    sendCommand("GRAPHIC");
    
    _authenticated = true;
    _game_state->setConnected(true);
    
    
#ifndef NDEBUG
    logDebug("Authenticated as GRAPHIC client");
#endif
    requestInitialGameState();
    return true;
}

void zappy::network::utils::NetworkManager::processPendingCommands() {
    auto pending_commands = _game_state->getAndClearPendingCommands();

    std::lock_guard<std::mutex> lock(_send_mutex);
    for (const auto& cmd : pending_commands) {
        std::string message = cmd.command + "\n";
        _send_queue.push(message);
        logDebug("Queued pending command: " + cmd.command);
    }
}

void zappy::network::utils::NetworkManager::processCommandParserPendingCommands() {
    if (!_command_parser) {
        return;
    }
    
    auto pending_commands = _command_parser->getAndClearPendingCommands();

    std::lock_guard<std::mutex> lock(_send_mutex);
    for (const auto& cmd : pending_commands) {
        std::string message = cmd + "\n";
        _send_queue.push(message);
        logDebug("Queued pending command from parser: " + cmd);
    }
}

bool zappy::network::utils::NetworkManager::processEvents() {
    if (!_connected || _should_stop.load()) {
        return false;
    }
    
    if (!handlePollEvents()) {
        return false;
    }
    processPendingCommands();
    processReceivedData();
    processCommandParserPendingCommands();
    
    // Envoyer les données en attente
    if (!sendPendingData()) {
        return false;
    }
    
    return true;
}


bool zappy::network::utils::NetworkManager::createSocket() {
    _socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket_fd == -1) {
        logError("Failed to create socket: " + std::string(strerror(errno)));
        return false;
    }
    
    int flags = fcntl(_socket_fd, F_GETFL, 0);
    if (flags == -1 || fcntl(_socket_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        logError("Failed to set non-blocking mode: " + std::string(strerror(errno)));
        close(_socket_fd);
        _socket_fd = -1;
        return false;
    }
    
    return true;
}

bool zappy::network::utils::NetworkManager::connectSocket() {
    struct sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(_port);
    
    // Résoudre le hostname
    struct hostent* host_entry = gethostbyname(_host.c_str());
    if (!host_entry) {
        logError("Failed to resolve hostname: " + _host);
        return false;
    }
    
    server_addr.sin_addr = *((struct in_addr*)host_entry->h_addr);
    
    // Tentative de connexion
    int result = ::connect(_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    
    if (result == -1) {
        if (errno == EINPROGRESS) {
            // Connexion en cours, attendre avec poll
            return waitForConnection();
        } else {
            logError("Connection failed: " + std::string(strerror(errno)));
            return false;
        }
    }
    
    return true;
}

bool zappy::network::utils::NetworkManager::waitForConnection() {
    struct pollfd conn_poll;
    conn_poll.fd = _socket_fd;
    conn_poll.events = POLLOUT;
    
    int result = poll(&conn_poll, 1, CONNECTION_TIMEOUT_S * 1000);
    
    if (result == -1) {
        logError("Poll failed during connection: " + std::string(strerror(errno)));
        return false;
    }
    
    if (result == 0) {
        logError("Connection timeout");
        return false;
    }
    
    // Vérifier si la connexion a réussi
    int error;
    socklen_t len = sizeof(error);
    if (getsockopt(_socket_fd, SOL_SOCKET, SO_ERROR, &error, &len) == -1) {
        logError("Failed to get socket error: " + std::string(strerror(errno)));
        return false;
    }
    
    if (error != 0) {
        logError("Connection failed: " + std::string(strerror(error)));
        return false;
    }
    
    return true;
}

void zappy::network::utils::NetworkManager::closeSocket() {
    if (_socket_fd != -1) {
        close(_socket_fd);
        _socket_fd = -1;
        _poll_fds[0].fd = -1;
    }
    _receive_buffer.clear();
    
    // Vider la queue d'envoi
    std::lock_guard<std::mutex> lock(_send_mutex);
    while (!_send_queue.empty()) {
        _send_queue.pop();
    }
}

bool zappy::network::utils::NetworkManager::handlePollEvents() {
    int result = poll(_poll_fds, 1, POLL_TIMEOUT_MS);
    
    if (result == -1) {
        if (errno == EINTR) {
            return true; // Interruption, continuer
        }
        logError("Poll failed: " + std::string(strerror(errno)));
        return false;
    }
    
    if (result == 0) {
        return true; // Timeout, continuer
    }
    
    // Vérifier les événements
    if (_poll_fds[0].revents & POLLERR || _poll_fds[0].revents & POLLHUP || _poll_fds[0].revents & POLLNVAL) {
        logError("Socket error detected");
        return false;
    }
    
    if (_poll_fds[0].revents & POLLIN) {
        if (!receiveData()) {
            return false;
        }
    }
    
    return true;
}

bool zappy::network::utils::NetworkManager::receiveData() {
    char buffer[4096];
    ssize_t bytes_received = recv(_socket_fd, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_received == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return true; // Pas de données disponibles
        }
        logError("Receive failed: " + std::string(strerror(errno)));
        return false;
    }
    
    if (bytes_received == 0) {
#ifndef NDEBUG
        logDebug("Server closed connection");
#endif
        return false;
    }
    
    buffer[bytes_received] = '\0';
    _receive_buffer += buffer;
    
    // Vérifier la taille du buffer
    if (_receive_buffer.size() > MAX_BUFFER_SIZE) {
        logError("Receive buffer overflow");
        _receive_buffer.clear();
        return false;
    }
    
    return true;
}

bool zappy::network::utils::NetworkManager::sendPendingData() {
    std::lock_guard<std::mutex> lock(_send_mutex);
    
    while (!_send_queue.empty()) {
        const std::string& message = _send_queue.front();
        
        ssize_t bytes_sent = send(_socket_fd, message.c_str(), message.length(), MSG_NOSIGNAL);
        std::cout << "[NetworkManager] send : " << message << std::endl;
        if (bytes_sent == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            logError("Send failed: " + std::string(strerror(errno)));
            return false;
        }
        
        if (bytes_sent == static_cast<ssize_t>(message.length())) {
            _send_queue.pop();
        } else {
            std::string remaining = message.substr(bytes_sent);
            _send_queue.front() = remaining;
            break;
        }
    }
    
    return true;
}

void zappy::network::utils::NetworkManager::processReceivedData() {
    std::string message;
    while (!(message = extractNextMessage()).empty()) {
        processCompleteMessage(message);
    }
}

std::string zappy::network::utils::NetworkManager::extractNextMessage() {
    size_t newline_pos = _receive_buffer.find('\n');
    if (newline_pos == std::string::npos) {
        return ""; // Message incomplet
    }
    
    std::string message = _receive_buffer.substr(0, newline_pos);
    _receive_buffer.erase(0, newline_pos + 1);
    
    if (!message.empty() && message.back() == '\r') {
        message.pop_back();
    }
    
    return message;
}

void zappy::network::utils::NetworkManager::processCompleteMessage(const std::string& message) {
    if (message.empty()) {
        return;
    }
    
#ifndef NDEBUG
    logDebug("Received: " + message);
#endif
    
    // Séparer la commande et les paramètres
    std::vector<std::string> parts = splitMessage(message);
    if (parts.empty()) {
        return;
    }
    
    std::string command = parts[0];
    parts.erase(parts.begin());
    
    // Parser la commande
    _command_parser->parseCommand(command, parts);
}

std::vector<std::string> zappy::network::utils::NetworkManager::splitMessage(const std::string& message) {
    std::vector<std::string> parts;
    std::stringstream ss(message);
    std::string part;
    
    while (ss >> part) {
        parts.push_back(part);
    }
    
    return parts;
}

void zappy::network::utils::NetworkManager::sendCommand(const std::string& command) {
    if (!_connected) {
        logError("Cannot send command: not connected");
        return;
    }
    
    std::string message = command + "\n";
    
    std::lock_guard<std::mutex> lock(_send_mutex);
    _send_queue.push(message);
    
#ifndef NDEBUG
    logDebug("Queued command: " + command);
#endif
}

void zappy::network::utils::NetworkManager::requestMapSize() {
    sendCommand("msz");
}

void zappy::network::utils::NetworkManager::requestMapContent() {
    sendCommand("mct");
}

void zappy::network::utils::NetworkManager::requestTeamNames() {
    sendCommand("tna");
}

void zappy::network::utils::NetworkManager::requestPlayerPosition(int player_id) {
    sendCommand("ppo #" + std::to_string(player_id));
}

void zappy::network::utils::NetworkManager::requestPlayerInventory(int player_id) {
    sendCommand("pin #" + std::to_string(player_id));
}

void zappy::network::utils::NetworkManager::requestTimeUnit() {
    sendCommand("sgt");
}

void zappy::network::utils::NetworkManager::requestInitialGameState() {
#ifndef NDEBUG
    logDebug("Requesting initial game state");
#endif
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    requestMapSize();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    requestMapContent();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    requestTeamNames();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    requestTimeUnit();
}

void zappy::network::utils::NetworkManager::setTimeUnit(int time_unit) {
    sendCommand("sst " + std::to_string(time_unit));
}

void zappy::network::utils::NetworkManager::logError(const std::string& message) const {
    std::cerr << "[NetworkManager ERROR] " << message << std::endl;
}

void zappy::network::utils::NetworkManager::logDebug(const std::string& message) const {
(void)message;
    #ifndef NDEBUG
    //std::cout << "[NetworkManager] " << message << std::endl;
#else
#endif
}
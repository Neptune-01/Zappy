/*
** EPITECH PROJECT, 2025
** Zappy [WSL: Ubuntu]
** File description:
** SharedGameState
*/

#ifndef SHAREDGAMESTATE_HPP_
#define SHAREDGAMESTATE_HPP_

#include <vector>
#include <map>
#include <string>
#include <mutex>
#include <atomic>
#include <queue>
#include <memory>
#include <chrono>
#include "zappy/game/world/Tile.hpp"
#include "zappy/game/entities/Player.hpp"

namespace zappy::game::world {

// Événements du jeu
struct GameEvent {
    enum Type {
        PLAYER_MOVED,
        PLAYER_LEVELED,
        PLAYER_DIED,
        RESOURCE_CHANGED,
        INCANTATION_STARTED,
        INCANTATION_ENDED,
        EGG_LAID,
        BROADCAST_RECEIVED
    };
    
    Type type;
    int player_id = -1;
    int x = -1;
    int y = -1;
    std::string message;
    std::map<std::string, int> data;
};

// Commandes réseau à envoyer
struct NetworkCommand {
    std::string command;
    int priority = 0;
    std::string response_expected;
};

struct ZappyBroadcast {
    int player_id = 0;           // ID du joueur qui a envoyé le broadcast
    std::string message = "";    // Contenu du message
    std::string team_name = "";  // Nom de l'équipe du joueur
    int x = 0;                   // Position X du joueur au moment du broadcast
    int y = 0;                   // Position Y du joueur au moment du broadcast
    std::chrono::steady_clock::time_point timestamp; // Timestamp du broadcast
    
    ZappyBroadcast() : timestamp(std::chrono::steady_clock::now()) {}
    
    ZappyBroadcast(int id, const std::string& msg, int pos_x = 0, int pos_y = 0) 
        : player_id(id), message(msg), x(pos_x), y(pos_y), 
          timestamp(std::chrono::steady_clock::now()) {}
};

class SharedGameState {
public:
    SharedGameState() = default;
    ~SharedGameState() = default;
    
    // Map management
    void setMapSize(int width, int height);
    void updateTile(int x, int y, const std::vector<int>& resources);
    void updateAllTiles(const std::vector<std::vector<ZappyTile>>& tiles);
    
    // Player management  
    void addPlayer(int id, int x, int y, int orientation, int level, const std::string& team);
    void updatePlayerPosition(int id, int x, int y, int orientation);
    void updatePlayerLevel(int id, int level);
    void updatePlayerInventory(int id, int x, int y, const std::vector<int>& inventory);
    void removePlayer(int id);

    // Broadcast management
    void addBroadcast(int player_id, const std::string& message);
    void addBroadcast(const ZappyBroadcast& broadcast);
    std::vector<ZappyBroadcast> getBroadcasts() const;
    std::vector<ZappyBroadcast> getRecentBroadcasts(int count = 10) const;
    void clearBroadcasts();
    size_t getBroadcastCount() const;
    
    // Resource management
    void updatePlayerResourceDelta(int player_id, int resource_type, int delta);
    void updateTileResourceDelta(int x, int y, int resource_type, int delta);
    
    // Team management
    void setTeamNames(const std::vector<std::string>& teams);
    
    // Event system
    void addEvent(const GameEvent& event);
    std::vector<GameEvent> getAndClearEvents();
    
    // Network commands
    void addPendingCommand(const NetworkCommand& command);
    std::vector<NetworkCommand> getAndClearPendingCommands();
    
    // Getters (thread-safe)
    std::vector<std::vector<ZappyTile>> getWorldMap() const;
    ZappyTile getTile(int x, int y) const;
    std::vector<zappy::game::entities::ZappyPlayer> getPlayers() const;
    zappy::game::entities::ZappyPlayer getPlayerById(int id) const;
    std::vector<std::string> getTeamNames() const;
    
    // Game state
    void setTimeUnit(int time_unit);
    int getTimeUnit() const;
    void setWinner(const std::string& team);
    std::string getWinner() const;
    
    // Connection state
    void setConnected(bool connected) { _connected = connected; }
    bool isConnected() const { return _connected.load(); }
    
    // Map dimensions
    int getMapWidth() const { return _map_width; }
    int getMapHeight() const { return _map_height; }
    
    // Statistics
    int getPlayerCount() const;
    
    // Debug
    void dumpState() const;
    
private:
    mutable std::mutex _mutex;
    
    // Map data
    int _map_width = 0;
    int _map_height = 0;
    std::vector<std::vector<ZappyTile>> _world_map;
    
    // Players
    std::map<int, zappy::game::entities::ZappyPlayer> _players;
    
    // Teams
    std::vector<std::string> _team_names;

    // Broadcasts
    std::vector<ZappyBroadcast> _broadcasts;
    static constexpr size_t MAX_BROADCASTS = 100;
    
    // Game state
    int _time_unit = 100;
    std::string _last_winner;
    
    // Connection state
    std::atomic<bool> _connected{false};
    
    // Events and commands
    std::queue<GameEvent> _events;
    std::queue<NetworkCommand> _pending_commands;
};

}

#endif /* !SHAREDGAMESTATE_HPP_ */
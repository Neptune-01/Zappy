/*
** EPITECH PROJECT, 2025
** Zappy [WSL: Ubuntu]
** File description:
** SharedGameState
*/

#include "zappy/game/world/SharedGameState.hpp"
#include <iostream>
#include <sstream>

void zappy::game::world::SharedGameState::setMapSize(int width, int height) {
    std::lock_guard<std::mutex> lock(_mutex);
    
    _map_width = width;
    _map_height = height;
    
    // Redimensionner la carte
    _world_map.clear();
    _world_map.resize(height);
    for (int y = 0; y < height; ++y) {
        _world_map[y].resize(width);
        for (int x = 0; x < width; ++x) {
            _world_map[y][x] = ZappyTile(x, y);
        }
    }
    
    //std::cout << "[SharedGameState] Map size set to " << width << "x" << height << std::endl;
}

void zappy::game::world::SharedGameState::updateTile(int x, int y, const std::vector<int>& resources) {
    std::lock_guard<std::mutex> lock(_mutex);
    
    if (x < 0 || x >= _map_width || y < 0 || y >= _map_height) {
        std::cerr << "[SharedGameState] Invalid tile coordinates: (" << x << ", " << y << ")" << std::endl;
        return;
    }
    
    if (resources.size() != 7) {
        std::cerr << "[SharedGameState] Invalid resource count: " << resources.size() << std::endl;
        return;
    }
    
    ZappyTile& tile = _world_map[y][x];
    tile.food = resources[0];
    tile.linemate = resources[1];
    tile.deraumere = resources[2];
    tile.sibur = resources[3];
    tile.mendiane = resources[4];
    tile.phiras = resources[5];
    tile.thystame = resources[6];
    
    // Créer un événement de changement de ressource
    GameEvent event;
    event.type = GameEvent::RESOURCE_CHANGED;
    event.x = x;
    event.y = y;
    _events.push(event);
}

void zappy::game::world::SharedGameState::updateAllTiles(const std::vector<std::vector<ZappyTile>>& tiles) {
    std::lock_guard<std::mutex> lock(_mutex);
    _world_map = tiles;
    
    // Créer un événement global de mise à jour
    GameEvent event;
    event.type = GameEvent::RESOURCE_CHANGED;
    event.x = -1; // Indique une mise à jour globale
    event.y = -1;
    _events.push(event);
}

void zappy::game::world::SharedGameState::addPlayer(int id, int x, int y, int orientation, int level, const std::string& team) {
    std::lock_guard<std::mutex> lock(_mutex);
    
    zappy::game::entities::ZappyPlayer player;
    player.id = id;
    player.x = x;
    player.y = y;
    player.orientation = orientation;
    player.level = level;
    player.team = team;
    
    // Assigner une couleur basée sur l'équipe
    if (team == "team1") player.color = sf::Color::Red;
    else if (team == "team2") player.color = sf::Color::Blue;
    else if (team == "team3") player.color = sf::Color::Green;
    else if (team == "team4") player.color = sf::Color::Yellow;
    else player.color = sf::Color::White;
    
    _players[id] = player;
    
    //std::cout << "[SharedGameState] Player " << id << " added at (" << x << ", " << y 
//<< ") team: " << team << std::endl;
}

void zappy::game::world::SharedGameState::updatePlayerPosition(int id, int x, int y, int orientation) {
    std::lock_guard<std::mutex> lock(_mutex);
    
    auto it = _players.find(id);
    if (it == _players.end()) {
        std::cerr << "[SharedGameState] Player " << id << " not found for position update" << std::endl;
        return;
    }
    
    it->second.x = x;
    it->second.y = y;
    it->second.orientation = orientation;
    
    // Créer un événement de mouvement
    GameEvent event;
    event.type = GameEvent::PLAYER_MOVED;
    event.player_id = id;
    event.x = x;
    event.y = y;
    _events.push(event);
}

void zappy::game::world::SharedGameState::updatePlayerLevel(int id, int level) {
    std::lock_guard<std::mutex> lock(_mutex);
    
    auto it = _players.find(id);
    if (it == _players.end()) {
        std::cerr << "[SharedGameState] Player " << id << " not found for level update" << std::endl;
        return;
    }
    
    it->second.level = level;
    
    // Créer un événement de level up
    GameEvent event;
    event.type = GameEvent::PLAYER_LEVELED;
    event.player_id = id;
    event.data["level"] = level;
    _events.push(event);
}

void zappy::game::world::SharedGameState::updatePlayerInventory(int id, int x, int y, const std::vector<int>& inventory) {
    std::lock_guard<std::mutex> lock(_mutex);
    
    auto it = _players.find(id);
    if (it == _players.end()) {
        std::cerr << "[SharedGameState] Player " << id << " not found for inventory update" << std::endl;
        return;
    }
    
    // Mettre à jour la position aussi (protocole pin)
    it->second.x = x;
    it->second.y = y;
    
    // Mettre à jour l'inventaire du joueur
    // Format: q0 q1 q2 q3 q4 q5 q6 (food, linemate, deraumere, sibur, mendiane, phiras, thystame)
    if (inventory.size() >= 7) {
        it->second.inventory_food = inventory[0];
        it->second.inventory_linemate = inventory[1];
        it->second.inventory_deraumere = inventory[2];
        it->second.inventory_sibur = inventory[3];
        it->second.inventory_mendiane = inventory[4];
        it->second.inventory_phiras = inventory[5];
        it->second.inventory_thystame = inventory[6];
        
        //std::cout << "[SharedGameState] Player " << id << " inventory updated: "
//    << "food=" << inventory[0] << " linemate=" << inventory[1] 
//    << " deraumere=" << inventory[2] << " sibur=" << inventory[3]
//    << " mendiane=" << inventory[4] << " phiras=" << inventory[5]
//    << " thystame=" << inventory[6] << std::endl;
    } else {
//        std::cerr << "[SharedGameState] Invalid inventory size for player " << id 
//    << ": " << inventory.size() << std::endl;
    }
}

void zappy::game::world::SharedGameState::removePlayer(int id) {
    std::lock_guard<std::mutex> lock(_mutex);
    
    auto it = _players.find(id);
    if (it == _players.end()) {
        std::cerr << "[SharedGameState] Player " << id << " not found for removal" << std::endl;
        return;
    }
    
    _players.erase(it);
    
    // Créer un événement de mort
    GameEvent event;
    event.type = GameEvent::PLAYER_DIED;
    event.player_id = id;
    _events.push(event);
    
    //std::cout << "[SharedGameState] Player " << id << " removed" << std::endl;
}

void zappy::game::world::SharedGameState::setTeamNames(const std::vector<std::string>& teams) {
    std::lock_guard<std::mutex> lock(_mutex);
    _team_names = teams;
    
    std::cout << "[SharedGameState] Teams: ";
    for (const auto& team : teams) {
        std::cout << team << " ";
    }
    std::cout << std::endl;
}

void zappy::game::world::SharedGameState::addEvent(const GameEvent& event) {
    std::lock_guard<std::mutex> lock(_mutex);
    _events.push(event);
}

std::vector<zappy::game::world::GameEvent> zappy::game::world::SharedGameState::getAndClearEvents() {
    std::lock_guard<std::mutex> lock(_mutex);
    
    std::vector<GameEvent> events;
    while (!_events.empty()) {
        events.push_back(_events.front());
        _events.pop();
    }
    return events;
}

void zappy::game::world::SharedGameState::addPendingCommand(const NetworkCommand& command) {
    std::lock_guard<std::mutex> lock(_mutex);
    _pending_commands.push(command);
    std::cout << "pending command : " << command.command.c_str() << std::endl;
}

std::vector<zappy::game::world::NetworkCommand> zappy::game::world::SharedGameState::getAndClearPendingCommands() {
    std::lock_guard<std::mutex> lock(_mutex);
    
    std::vector<NetworkCommand> commands;
    while (!_pending_commands.empty()) {
        commands.push_back(_pending_commands.front());
        _pending_commands.pop();
    }
    return commands;
}

std::vector<std::vector<zappy::game::world::ZappyTile>> zappy::game::world::SharedGameState::getWorldMap() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _world_map;
}

zappy::game::world::ZappyTile zappy::game::world::SharedGameState::getTile(int x, int y) const {
    std::lock_guard<std::mutex> lock(_mutex);
    if (x < 0 || x >= _map_width || y < 0 || y >= _map_height) {
        throw std::out_of_range("Tile coordinates out of bounds");
    }
    return _world_map[y][x];  // Safe copy
}

std::vector<zappy::game::entities::ZappyPlayer> zappy::game::world::SharedGameState::getPlayers() const {
    std::lock_guard<std::mutex> lock(_mutex);
    
    std::vector<zappy::game::entities::ZappyPlayer> players;
    for (const auto& pair : _players) {
        players.push_back(pair.second);
    }
    return players;
}

zappy::game::entities::ZappyPlayer zappy::game::world::SharedGameState::getPlayerById(int id) const {
    std::lock_guard<std::mutex> lock(_mutex);

    auto it = _players.find(id);
    if (it == _players.end()) {
        std::cerr << "[SharedGameState] getPlayerById: Player " << id << " not found" << std::endl;
        return zappy::game::entities::ZappyPlayer();
    }
    return it->second;
}

std::vector<std::string> zappy::game::world::SharedGameState::getTeamNames() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _team_names;
}

void zappy::game::world::SharedGameState::setTimeUnit(int time_unit) {
    std::lock_guard<std::mutex> lock(_mutex);
    _time_unit = time_unit;
}

int zappy::game::world::SharedGameState::getTimeUnit() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _time_unit;
}

void zappy::game::world::SharedGameState::setWinner(const std::string& team) {
    std::lock_guard<std::mutex> lock(_mutex);
    _last_winner = team;
    
    //std::cout << "[SharedGameState] Game ended! Winner: " << team << std::endl;
}

std::string zappy::game::world::SharedGameState::getWinner() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _last_winner;
}

int zappy::game::world::SharedGameState::getPlayerCount() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _players.size();
}

void zappy::game::world::SharedGameState::dumpState() const {
    std::lock_guard<std::mutex> lock(_mutex);
    
    //std::cout << "=== GAME STATE DUMP ===" << std::endl;
    //std::cout << "Map: " << _map_width << "x" << _map_height << std::endl;
    //std::cout << "Players: " << _players.size() << std::endl;
    //std::cout << "Teams: " << _team_names.size() << std::endl;
    //std::cout << "Connected: " << _connected.load() << std::endl;
    //std::cout << "Time unit: " << _time_unit << std::endl;
    //std::cout << "======================" << std::endl;
}

void zappy::game::world::SharedGameState::updatePlayerResourceDelta(int player_id, int resource_type, int delta) {
    std::lock_guard<std::mutex> lock(_mutex);
    
    auto it = _players.find(player_id);
    if (it == _players.end()) {
        std::cerr << "[SharedGameState] Player " << player_id << " not found for resource delta" << std::endl;
        return;
    }
    
    // Apply delta to the appropriate resource
    switch (resource_type) {
        case 0: // food
            it->second.inventory_food = std::max(0, it->second.inventory_food + delta);
            break;
        case 1: // linemate
            it->second.inventory_linemate = std::max(0, it->second.inventory_linemate + delta);
            break;
        case 2: // deraumere
            it->second.inventory_deraumere = std::max(0, it->second.inventory_deraumere + delta);
            break;
        case 3: // sibur
            it->second.inventory_sibur = std::max(0, it->second.inventory_sibur + delta);
            break;
        case 4: // mendiane
            it->second.inventory_mendiane = std::max(0, it->second.inventory_mendiane + delta);
            break;
        case 5: // phiras
            it->second.inventory_phiras = std::max(0, it->second.inventory_phiras + delta);
            break;
        case 6: // thystame
            it->second.inventory_thystame = std::max(0, it->second.inventory_thystame + delta);
            break;
        default:
            std::cerr << "[SharedGameState] Invalid resource type: " << resource_type << std::endl;
            return;
    }
    
    //std::cout << "[SharedGameState] Player " << player_id << " resource " << resource_type 
//<< " changed by " << delta << std::endl;
}

void zappy::game::world::SharedGameState::updateTileResourceDelta(int x, int y, int resource_type, int delta) {
    std::lock_guard<std::mutex> lock(_mutex);
    
    if (x < 0 || x >= _map_width || y < 0 || y >= _map_height) {
//        std::cerr << "[SharedGameState] Invalid tile coordinates for resource delta: (" 
//    << x << ", " << y << ")" << std::endl;
        return;
    }
    
    ZappyTile& tile = _world_map[y][x];
    
    // Apply delta to the appropriate resource
    switch (resource_type) {
        case 0: // food
            tile.food = std::max(0, tile.food + delta);
            break;
        case 1: // linemate
            tile.linemate = std::max(0, tile.linemate + delta);
            break;
        case 2: // deraumere
            tile.deraumere = std::max(0, tile.deraumere + delta);
            break;
        case 3: // sibur
            tile.sibur = std::max(0, tile.sibur + delta);
            break;
        case 4: // mendiane
            tile.mendiane = std::max(0, tile.mendiane + delta);
            break;
        case 5: // phiras
            tile.phiras = std::max(0, tile.phiras + delta);
            break;
        case 6: // thystame
            tile.thystame = std::max(0, tile.thystame + delta);
            break;
        default:
            std::cerr << "[SharedGameState] Invalid resource type: " << resource_type << std::endl;
            return;
    }
    
    //std::cout << "[SharedGameState] Tile (" << x << ", " << y << ") resource " << resource_type 
//<< " changed by " << delta << std::endl;
}

void zappy::game::world::SharedGameState::addBroadcast(int player_id, const std::string& message) {
    std::lock_guard<std::mutex> lock(_mutex);
    
    // Créer le broadcast
    ZappyBroadcast broadcast(player_id, message);
    
    auto it = _players.find(player_id);
    if (it != _players.end()) {
        broadcast.x = it->second.x;
        broadcast.y = it->second.y;
        broadcast.team_name = it->second.team;
    }
    
    // Ajouter le broadcast à la liste
    _broadcasts.push_back(broadcast);
    
    // Maintenir la limite de broadcasts stockés
    if (_broadcasts.size() > MAX_BROADCASTS) {
        _broadcasts.erase(_broadcasts.begin(), 
                         _broadcasts.begin() + (_broadcasts.size() - MAX_BROADCASTS));
    }
}

void zappy::game::world::SharedGameState::addBroadcast(const ZappyBroadcast& broadcast) {
    std::lock_guard<std::mutex> lock(_mutex);
    
    _broadcasts.push_back(broadcast);
    
    // Maintenir la limite
    if (_broadcasts.size() > MAX_BROADCASTS) {
        _broadcasts.erase(_broadcasts.begin(), 
                         _broadcasts.begin() + (_broadcasts.size() - MAX_BROADCASTS));
    }
}

std::vector<zappy::game::world::ZappyBroadcast> zappy::game::world::SharedGameState::getBroadcasts() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _broadcasts;
}

std::vector<zappy::game::world::ZappyBroadcast> zappy::game::world::SharedGameState::getRecentBroadcasts(int count) const {
    std::lock_guard<std::mutex> lock(_mutex);
    
    if (_broadcasts.empty()) {
        return {};
    }
    
    if (count <= 0 || static_cast<size_t>(count) >= _broadcasts.size()) {
        return _broadcasts;
    }
    
    // Retourner les 'count' derniers broadcasts
    auto start_it = _broadcasts.end() - count;
    return std::vector<ZappyBroadcast>(start_it, _broadcasts.end());
}

void zappy::game::world::SharedGameState::clearBroadcasts() {
    std::lock_guard<std::mutex> lock(_mutex);
    _broadcasts.clear();
}

size_t zappy::game::world::SharedGameState::getBroadcastCount() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _broadcasts.size();
}
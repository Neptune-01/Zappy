/*
** EPITECH PROJECT, 2025
** Zappy [WSL: Ubuntu]
** File description:
** CommandParser - Complete Implementation
*/

#include "zappy/network/protocol/CommandParser.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

zappy::network::protocol::CommandParser::CommandParser(std::shared_ptr<zappy::game::world::SharedGameState> game_state)
    : _game_state(game_state) {
    setupCommandHandlers();
}

void zappy::network::protocol::CommandParser::setupCommandHandlers() {
    // Commandes principales du protocole GUI
    _command_handlers["msz"] = [this](const auto& params) { handleMapSize(params); };
    _command_handlers["bct"] = [this](const auto& params) { handleTileContent(params); };
    _command_handlers["tna"] = [this](const auto& params) { handleTeamNames(params); };
    
    // Gestion des joueurs
    _command_handlers["pnw"] = [this](const auto& params) { handleNewPlayer(params); };
    _command_handlers["ppo"] = [this](const auto& params) { handlePlayerPosition(params); };
    _command_handlers["plv"] = [this](const auto& params) { handlePlayerLevel(params); };
    _command_handlers["pin"] = [this](const auto& params) { handlePlayerInventory(params); };
    _command_handlers["pex"] = [this](const auto& params) { handlePlayerExpulsion(params); };
    _command_handlers["pbc"] = [this](const auto& params) { handlePlayerBroadcast(params); };
    _command_handlers["pdi"] = [this](const auto& params) { handlePlayerDeath(params); };
    
    // Gestion des incantations
    _command_handlers["pic"] = [this](const auto& params) { handleIncantationStart(params); };
    _command_handlers["pie"] = [this](const auto& params) { handleIncantationEnd(params); };
    
    // Gestion des œufs
    _command_handlers["pfk"] = [this](const auto& params) { handleEggLaying(params); };
    _command_handlers["enw"] = [this](const auto& params) { handleNewEgg(params); };
    _command_handlers["ebo"] = [this](const auto& params) { handleEggConnection(params); };
    _command_handlers["edi"] = [this](const auto& params) { handleEggDeath(params); };
    
    // Gestion des ressources
    _command_handlers["pdr"] = [this](const auto& params) { handleResourceDrop(params); };
    _command_handlers["pgt"] = [this](const auto& params) { handleResourceTake(params); };
    
    // Gestion du jeu
    _command_handlers["sgt"] = [this](const auto& params) { handleTimeUnit(params); };
    _command_handlers["sst"] = [this](const auto& params) { handleTimeUnitSet(params); };
    _command_handlers["seg"] = [this](const auto& params) { handleEndGame(params); };
    _command_handlers["smg"] = [this](const auto& params) { handleServerMessage(params); };
    
    // Gestion des erreurs
    _command_handlers["suc"] = [this](const auto& params) { handleUnknownCommand(params); };
    _command_handlers["sbp"] = [this](const auto& params) { handleBadParameter(params); };
}

void zappy::network::protocol::CommandParser::parseCommand(const std::string& command, const std::vector<std::string>& parameters) {
    logCommand(command, parameters);
    
    auto it = _command_handlers.find(command);
    if (it != _command_handlers.end()) {
        try {
            it->second(parameters);
        } catch (const std::exception& e) {
            logError("Exception while handling command '" + command + "': " + e.what());
        }
    } else {
        logWarning("Unknown command: " + command);
    }
}

bool zappy::network::protocol::CommandParser::isValidCommand(const std::string& command) const {
    return _command_handlers.find(command) != _command_handlers.end();
}

// Handlers pour les commandes principales

void zappy::network::protocol::CommandParser::handleMapSize(const std::vector<std::string>& params) {
    if (!validateParameters(params, 2, "msz")) return;
    
    int width = parseInteger(params[0], "map width");
    int height = parseInteger(params[1], "map height");
    
    if (width <= 0 || height <= 0) {
        logError("Invalid map dimensions: " + std::to_string(width) + "x" + std::to_string(height));
        return;
    }
    
    _game_state->setMapSize(width, height);
}

void zappy::network::protocol::CommandParser::handleTileContent(const std::vector<std::string>& params) {
    if (!validateParameters(params, 9, "bct")) return;
    
    int x = parseInteger(params[0], "tile x");
    int y = parseInteger(params[1], "tile y");
    
    if (!validateCoordinates(x, y)) return;
    
    std::vector<int> resources = parseResourceList(params, 2);
    if (resources.size() != 7) {
        logError("Invalid resource count in bct command");
        return;
    }
    
    _game_state->updateTile(x, y, resources);
}

void zappy::network::protocol::CommandParser::handleTeamNames(const std::vector<std::string>& params) {
    static bool teams_finalized = false;
    
    if (params.empty() || teams_finalized) {
        return;
    }
    
    auto teams = _game_state->getTeamNames();
    size_t original_size = teams.size();
    
    for (const auto& param : params) {
        std::string team_name = parseString(param);
        
        if (!team_name.empty() && 
            std::find(teams.begin(), teams.end(), team_name) == teams.end()) {
            teams.push_back(team_name);
        }
    }
    
    if (teams.size() == original_size && !teams.empty()) {
        if (!teams_finalized) {
            _game_state->setTeamNames(teams);
            teams_finalized = true;
        }
    } else if (teams.size() > original_size) {
        _game_state->setTeamNames(teams);
    }
}

void zappy::network::protocol::CommandParser::handleNewPlayer(const std::vector<std::string>& params) {
    if (!validateParameters(params, 6, "pnw")) return;
    
    // Format: pnw #n X Y O L N
    std::string player_str = params[0];
    if (player_str.empty() || player_str[0] != '#') {
        logError("Invalid player ID format: " + player_str);
        return;
    }
    
    int player_id = parseInteger(player_str.substr(1), "player ID");
    int x = parseInteger(params[1], "player x");
    int y = parseInteger(params[2], "player y");
    int orientation = parseInteger(params[3], "player orientation");
    int level = parseInteger(params[4], "player level");
    std::string team = parseString(params[5]);
    
    if (!validateCoordinates(x, y) || !validateOrientation(orientation)) return;
    
    _game_state->addPlayer(player_id, x, y, orientation, level, team);
}

void zappy::network::protocol::CommandParser::handlePlayerPosition(const std::vector<std::string>& params) {
    if (!validateParameters(params, 4, "ppo")) return;
    
    // Format: ppo #n X Y O
    std::string player_str = params[0];
    if (player_str.empty() || player_str[0] != '#') {
        logError("Invalid player ID format: " + player_str);
        return;
    }
    
    int player_id = parseInteger(player_str.substr(1), "player ID");
    int x = parseInteger(params[1], "player x");
    int y = parseInteger(params[2], "player y");
    int orientation = parseInteger(params[3], "player orientation");
    
    if (!validateCoordinates(x, y) || !validateOrientation(orientation)) return;
    
    _game_state->updatePlayerPosition(player_id, x, y, orientation);
}

void zappy::network::protocol::CommandParser::handlePlayerLevel(const std::vector<std::string>& params) {
    if (!validateParameters(params, 2, "plv")) return;
    
    // Format: plv #n L
    std::string player_str = params[0];
    if (player_str.empty() || player_str[0] != '#') {
        logError("Invalid player ID format: " + player_str);
        return;
    }
    
    int player_id = parseInteger(player_str.substr(1), "player ID");
    int level = parseInteger(params[1], "player level");
    
    if (level < 1 || level > 8) {
        logError("Invalid player level: " + std::to_string(level));
        return;
    }
    
    _game_state->updatePlayerLevel(player_id, level);
}

void zappy::network::protocol::CommandParser::handlePlayerInventory(const std::vector<std::string>& params) {
    if (!validateParameters(params, 10, "pin")) return;
    
    // Format: pin #n X Y q0 q1 q2 q3 q4 q5 q6
    std::string player_str = params[0];
    if (player_str.empty() || player_str[0] != '#') {
        logError("Invalid player ID format: " + player_str);
        return;
    }
    
    int player_id = parseInteger(player_str.substr(1), "player ID");
    int x = parseInteger(params[1], "player x");
    int y = parseInteger(params[2], "player y");
    
    if (!validateCoordinates(x, y)) return;
    
    std::vector<int> inventory = parseResourceList(params, 3);
    if (inventory.size() != 7) {
        logError("Invalid inventory count in pin command");
        return;
    }
    
    _game_state->updatePlayerInventory(player_id, x, y, inventory);
    
    // Créer un événement d'inventaire mis à jour
    zappy::game::world::GameEvent event;
    event.type = zappy::game::world::GameEvent::RESOURCE_CHANGED;
    event.player_id = player_id;
    event.x = x;
    event.y = y;
    event.message = "inventory_updated";
    _game_state->addEvent(event);
}

void zappy::network::protocol::CommandParser::handlePlayerDeath(const std::vector<std::string>& params) {
    if (!validateParameters(params, 1, "pdi")) return;
    
    // Format: pdi #n
    std::string player_str = params[0];
    if (player_str.empty() || player_str[0] != '#') {
        logError("Invalid player ID format: " + player_str);
        return;
    }
    
    int player_id = parseInteger(player_str.substr(1), "player ID");
    _game_state->removePlayer(player_id);
}

void zappy::network::protocol::CommandParser::handleTimeUnit(const std::vector<std::string>& params) {
    if (!validateParameters(params, 1, "sgt")) return;
    
    int time_unit = parseInteger(params[0], "time unit");
    _game_state->setTimeUnit(time_unit);
}

void zappy::network::protocol::CommandParser::handleEndGame(const std::vector<std::string>& params) {
    if (!validateParameters(params, 1, "seg")) return;
    
    std::string winner = parseString(params[0]);
    _game_state->setWinner(winner);
}

// Handlers pour les commandes complètes

void zappy::network::protocol::CommandParser::handlePlayerExpulsion(const std::vector<std::string>& params) {
    if (!validateParameters(params, 1, "pex")) return;
    
    // Format: pex #n
    std::string player_str = params[0];
    if (player_str.empty() || player_str[0] != '#') {
        logError("Invalid player ID format: " + player_str);
        return;
    }
    
    int player_id = parseInteger(player_str.substr(1), "player ID");
    
    // Créer un événement d'expulsion
    zappy::game::world::GameEvent event;
    event.type = zappy::game::world::GameEvent::PLAYER_MOVED; // Réutiliser le type existant
    event.player_id = player_id;
    event.message = "expelled";
    _game_state->addEvent(event);
    
    logCommand("pex", params);
}

void zappy::network::protocol::CommandParser::handlePlayerBroadcast(const std::vector<std::string>& params) {
    if (!validateParameters(params, 2, "pbc")) return;
    
    // Format: pbc #n M
    std::string player_str = params[0];
    if (player_str.empty() || player_str[0] != '#') {
        logError("Invalid player ID format: " + player_str);
        return;
    }
    
    int player_id = parseInteger(player_str.substr(1), "player ID");
    std::string message = parseString(params[1]);
    _game_state->addBroadcast(player_id, message);
    
    // Créer un événement de broadcast
    zappy::game::world::GameEvent event;
    event.type = zappy::game::world::GameEvent::BROADCAST_RECEIVED;
    event.player_id = player_id;
    event.message = message;
    _game_state->addEvent(event);
    
    logCommand("pbc", params);
}

void zappy::network::protocol::CommandParser::handleIncantationStart(const std::vector<std::string>& params) {
    if (!validateParameters(params, 4, "pic")) return;
    
    // Format: pic X Y L #n #n ...
    int x = parseInteger(params[0], "incantation x");
    int y = parseInteger(params[1], "incantation y");
    int level = parseInteger(params[2], "incantation level");
    
    if (!validateCoordinates(x, y)) return;
    
    // Parser les joueurs participants
    std::vector<int> players = parsePlayerList(params, 3);
    
    // Créer un événement d'incantation
    zappy::game::world::GameEvent event;
    event.type = zappy::game::world::GameEvent::INCANTATION_STARTED;
    event.x = x;
    event.y = y;
    event.data["level"] = level;
    event.data["player_count"] = static_cast<int>(players.size());
    
    if (!players.empty()) {
        event.player_id = players[0]; // Premier joueur comme référence
    }
    
    _game_state->addEvent(event);
    logCommand("pic", params);
}

void zappy::network::protocol::CommandParser::handleIncantationEnd(const std::vector<std::string>& params) {
    if (!validateParameters(params, 3, "pie")) return;
    
    // Format: pie X Y R
    int x = parseInteger(params[0], "incantation x");
    int y = parseInteger(params[1], "incantation y");
    int result = parseInteger(params[2], "incantation result");
    
    if (!validateCoordinates(x, y)) return;
    
    // Créer un événement de fin d'incantation
    zappy::game::world::GameEvent event;
    event.type = zappy::game::world::GameEvent::INCANTATION_ENDED;
    event.x = x;
    event.y = y;
    event.data["result"] = result;
    event.message = (result == 1) ? "success" : "failure";
    
    _game_state->addEvent(event);
    logCommand("pie", params);
}

void zappy::network::protocol::CommandParser::handleEggLaying(const std::vector<std::string>& params) {
    if (!validateParameters(params, 1, "pfk")) return;
    
    // Format: pfk #n
    std::string player_str = params[0];
    if (player_str.empty() || player_str[0] != '#') {
        logError("Invalid player ID format: " + player_str);
        return;
    }
    
    int player_id = parseInteger(player_str.substr(1), "player ID");
    
    // Créer un événement de ponte d'œuf
    zappy::game::world::GameEvent event;
    event.type = zappy::game::world::GameEvent::EGG_LAID;
    event.player_id = player_id;
    
    _game_state->addEvent(event);
    logCommand("pfk", params);
}

void zappy::network::protocol::CommandParser::handleNewEgg(const std::vector<std::string>& params) {
    if (!validateParameters(params, 4, "enw")) return;
    
    // Format: enw #e #n X Y
    std::string egg_str = params[0];
    std::string player_str = params[1];
    
    if (egg_str.empty() || egg_str[0] != '#') {
        logError("Invalid egg ID format: " + egg_str);
        return;
    }
    
    if (player_str.empty() || player_str[0] != '#') {
        logError("Invalid player ID format: " + player_str);
        return;
    }
    
    int egg_id = parseInteger(egg_str.substr(1), "egg ID");
    int player_id = parseInteger(player_str.substr(1), "player ID");
    int x = parseInteger(params[2], "egg x");
    int y = parseInteger(params[3], "egg y");
    
    if (!validateCoordinates(x, y)) return;
    
    // Créer un événement de nouvel œuf
    zappy::game::world::GameEvent event;
    event.type = zappy::game::world::GameEvent::EGG_LAID;
    event.player_id = player_id;
    event.x = x;
    event.y = y;
    event.data["egg_id"] = egg_id;
    
    _game_state->addEvent(event);
    logCommand("enw", params);
}

void zappy::network::protocol::CommandParser::handleEggConnection(const std::vector<std::string>& params) {
    if (!validateParameters(params, 1, "ebo")) return;
    
    // Format: ebo #e
    std::string egg_str = params[0];
    if (egg_str.empty() || egg_str[0] != '#') {
        logError("Invalid egg ID format: " + egg_str);
        return;
    }
    
    int egg_id = parseInteger(egg_str.substr(1), "egg ID");
    
    // Créer un événement de connexion d'œuf
    zappy::game::world::GameEvent event;
    event.type = zappy::game::world::GameEvent::EGG_LAID; // Réutiliser pour l'instant
    event.player_id = -1; // Pas de joueur spécifique
    event.data["egg_id"] = egg_id;
    event.message = "egg_connected";
    
    _game_state->addEvent(event);
    logCommand("ebo", params);
}

void zappy::network::protocol::CommandParser::handleEggDeath(const std::vector<std::string>& params) {
    if (!validateParameters(params, 1, "edi")) return;
    
    // Format: edi #e
    std::string egg_str = params[0];
    if (egg_str.empty() || egg_str[0] != '#') {
        logError("Invalid egg ID format: " + egg_str);
        return;
    }
    
    int egg_id = parseInteger(egg_str.substr(1), "egg ID");
    
    // Créer un événement de mort d'œuf
    zappy::game::world::GameEvent event;
    event.type = zappy::game::world::GameEvent::PLAYER_DIED; // Réutiliser pour l'instant
    event.player_id = -1; // Pas de joueur spécifique
    event.data["egg_id"] = egg_id;
    event.message = "egg_died";
    
    _game_state->addEvent(event);
    logCommand("edi", params);
}

void zappy::network::protocol::CommandParser::handleResourceDrop(const std::vector<std::string>& params) {
    if (!validateParameters(params, 2, "pdr")) return;
    
    // Format: pdr #n i
    std::string player_str = params[0];
    if (player_str.empty() || player_str[0] != '#') {
        logError("Invalid player ID format: " + player_str);
        return;
    }
    
    int player_id = parseInteger(player_str.substr(1), "player ID");
    int resource_type = parseInteger(params[1], "resource type");
    
    if (resource_type < 0 || resource_type > 6) {
        logError("Invalid resource type: " + std::to_string(resource_type));
        return;
    }
    
    // Get player to know which tile to update
    auto player = _game_state->getPlayerById(player_id);
    if (player.id != 0 || player_id == 0) {
        // Add pending commands for updates
        addPendingCommand("bct " + std::to_string(player.x) + " " + std::to_string(player.y));
        addPendingCommand("pin #" + std::to_string(player_id));
    }
    
    // Create a simple event
    zappy::game::world::GameEvent event;
    event.type = zappy::game::world::GameEvent::RESOURCE_CHANGED;
    event.player_id = player_id;
    event.data["resource_type"] = resource_type;
    event.data["action"] = 0; // 0 = drop, 1 = take
    event.message = "resource_dropped";
    
    _game_state->addEvent(event);
    logCommand("pdr", params);
}

void zappy::network::protocol::CommandParser::handleResourceTake(const std::vector<std::string>& params) {
    if (!validateParameters(params, 2, "pgt")) return;
    
    // Format: pgt #n i
    std::string player_str = params[0];
    if (player_str.empty() || player_str[0] != '#') {
        logError("Invalid player ID format: " + player_str);
        return;
    }
    
    int player_id = parseInteger(player_str.substr(1), "player ID");
    int resource_type = parseInteger(params[1], "resource type");
    
    if (resource_type < 0 || resource_type > 6) {
        logError("Invalid resource type: " + std::to_string(resource_type));
        return;
    }
    
    // Get player to know which tile to update
    auto player = _game_state->getPlayerById(player_id);
    if (player.id != 0 || player_id == 0) {
        // Add pending commands for updates
        addPendingCommand("bct " + std::to_string(player.x) + " " + std::to_string(player.y));
        addPendingCommand("pin #" + std::to_string(player_id));
    }
    
    // Create a simple event
    zappy::game::world::GameEvent event;
    event.type = zappy::game::world::GameEvent::RESOURCE_CHANGED;
    event.player_id = player_id;
    event.data["resource_type"] = resource_type;
    event.data["action"] = 1; // 0 = drop, 1 = take
    event.message = "resource_taken";
    
    _game_state->addEvent(event);
    logCommand("pgt", params);
}

void zappy::network::protocol::CommandParser::handleTimeUnitSet(const std::vector<std::string>& params) {
    if (!validateParameters(params, 1, "sst")) return;
    
    int time_unit = parseInteger(params[0], "time unit");
    _game_state->setTimeUnit(time_unit);
    
    logCommand("sst", params);
}

void zappy::network::protocol::CommandParser::handleServerMessage(const std::vector<std::string>& params) {
    if (!validateParameters(params, 1, "smg")) return;
    
    std::string message = parseString(params[0]);
    _game_state->addBroadcast(-1, message);
    
    // Créer un événement pour afficher le message du serveur
    zappy::game::world::GameEvent event;
    event.type = zappy::game::world::GameEvent::BROADCAST_RECEIVED;
    event.player_id = -1; // Indique que c'est un message du serveur
    event.message = message;
    
    _game_state->addEvent(event);
    logCommand("smg", params);
}

void zappy::network::protocol::CommandParser::handleUnknownCommand(const std::vector<std::string>& params) {
    (void)params; // Suppress unused parameter warning
    logWarning("Server reported unknown command");
}

void zappy::network::protocol::CommandParser::handleBadParameter(const std::vector<std::string>& params) {
    (void)params; // Suppress unused parameter warning
    logWarning("Server reported bad parameter");
}

// Utilitaires de parsing

int zappy::network::protocol::CommandParser::parseInteger(const std::string& str, const std::string& context) const {
    try {
        return std::stoi(str);
    } catch (const std::exception& e) {
        logError("Failed to parse integer '" + str + "' for " + context);
        return 0;
    }
}

std::string zappy::network::protocol::CommandParser::parseString(const std::string& str) const {
    return str;
}

std::vector<int> zappy::network::protocol::CommandParser::parseResourceList(const std::vector<std::string>& params, size_t start_index) const {
    std::vector<int> resources;
    
    for (size_t i = start_index; i < start_index + 7 && i < params.size(); ++i) {
        resources.push_back(parseInteger(params[i], "resource"));
    }
    
    return resources;
}

std::vector<int> zappy::network::protocol::CommandParser::parsePlayerList(const std::vector<std::string>& params, size_t start_index) const {
    std::vector<int> players;
    
    for (size_t i = start_index; i < params.size(); ++i) {
        std::string player_str = params[i];
        if (!player_str.empty() && player_str[0] == '#') {
            players.push_back(parseInteger(player_str.substr(1), "player ID"));
        }
    }
    
    return players;
}

// Validation

bool zappy::network::protocol::CommandParser::validateParameters(const std::vector<std::string>& params, size_t expected_count, const std::string& command) const {
    if (params.size() < expected_count) {
        logError("Command '" + command + "' expects " + std::to_string(expected_count) + 
                " parameters, got " + std::to_string(params.size()));
        return false;
    }
    return true;
}

bool zappy::network::protocol::CommandParser::validateCoordinates(int x, int y) const {
    if (x < 0 || y < 0) {
        logError("Invalid coordinates: (" + std::to_string(x) + ", " + std::to_string(y) + ")");
        return false;
    }
    
    // Valider contre la taille de la carte si elle est connue
    int map_width = _game_state->getMapWidth();
    int map_height = _game_state->getMapHeight();
    
    if (map_width > 0 && map_height > 0) {
        if (x >= map_width || y >= map_height) {
            logError("Coordinates out of bounds: (" + std::to_string(x) + ", " + std::to_string(y) + 
                    ") for map " + std::to_string(map_width) + "x" + std::to_string(map_height));
            return false;
        }
    }
    
    return true;
}

bool zappy::network::protocol::CommandParser::validateOrientation(int orientation) const {
    if (orientation < 1 || orientation > 4) {
        logError("Invalid orientation: " + std::to_string(orientation) + " (expected 1-4)");
        return false;
    }
    return true;
}

bool zappy::network::protocol::CommandParser::validatePlayerId(int player_id) const {
    if (player_id < 0) {
        logError("Invalid player ID: " + std::to_string(player_id));
        return false;
    }
    return true;
}

// Logging

void zappy::network::protocol::CommandParser::logCommand(const std::string& command, const std::vector<std::string>& params) const {
    std::ostringstream oss;
    //oss << "[CommandParser] " << command;
    //for (const auto& param : params) {
    //    oss << " " << param;
    //}
    //std::cout << oss.str() << std::endl;
    (void)command;
    (void)params;
}

void zappy::network::protocol::CommandParser::logError(const std::string& message) const {
    #ifndef NDEBUG
    std::cerr << "[CommandParser ERROR] " << message << std::endl;
    #endif
    (void)message;
}

void zappy::network::protocol::CommandParser::logWarning(const std::string& message) const {
    #ifndef NDEBUG
    //std::cerr << "[CommandParser WARNING] " << message << std::endl;
    #endif
    (void)message;
}

void zappy::network::protocol::CommandParser::addPendingCommand(const std::string& command) {
    std::lock_guard<std::mutex> lock(_pending_mutex);
    _pending_commands.push_back(command);
    #ifndef NDEBUG
    std::cout << "[CommandParser] Added pending command: " << command << std::endl;
    #endif
}

std::vector<std::string> zappy::network::protocol::CommandParser::getAndClearPendingCommands() {
    std::lock_guard<std::mutex> lock(_pending_mutex);
    std::vector<std::string> commands = _pending_commands;
    _pending_commands.clear();
    return commands;
}

bool zappy::network::protocol::CommandParser::hasPendingCommands() const {
    std::lock_guard<std::mutex> lock(_pending_mutex);
    return !_pending_commands.empty();
}
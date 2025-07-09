/*
** EPITECH PROJECT, 2025
** Zappy [WSL: Ubuntu]
** File description:
** CommandParser
*/

#ifndef COMMANDPARSER_HPP_
#define COMMANDPARSER_HPP_

#include <string>
#include <vector>
#include <functional>
#include <map>
#include <memory>
#include "zappy/game/world/SharedGameState.hpp"

namespace zappy::network::protocol {
class CommandParser {
public:
    explicit CommandParser(std::shared_ptr<zappy::game::world::SharedGameState> game_state);
    ~CommandParser() = default;
    
    // Parsing principal
    void parseCommand(const std::string& command, const std::vector<std::string>& parameters);
    bool isValidCommand(const std::string& command) const;

    // Pending commands management
    void addPendingCommand(const std::string& command);
    std::vector<std::string> getAndClearPendingCommands();
    bool hasPendingCommands() const;
    
private:
    // Initialisation des handlers
    void setupCommandHandlers();
    
    // Handlers pour les commandes du protocole GUI
    void handleMapSize(const std::vector<std::string>& params);              // msz X Y
    void handleTileContent(const std::vector<std::string>& params);          // bct X Y q0 q1 q2 q3 q4 q5 q6
    void handleMapContent(const std::vector<std::string>& params);           // mct (suivi de multiples bct)
    void handleTeamNames(const std::vector<std::string>& params);            // tna N
    
    // Gestion des joueurs
    void handleNewPlayer(const std::vector<std::string>& params);            // pnw #n X Y O L N
    void handlePlayerPosition(const std::vector<std::string>& params);       // ppo #n X Y O
    void handlePlayerLevel(const std::vector<std::string>& params);          // plv #n L
    void handlePlayerInventory(const std::vector<std::string>& params);      // pin #n X Y q0 q1 q2 q3 q4 q5 q6
    void handlePlayerExpulsion(const std::vector<std::string>& params);      // pex #n
    void handlePlayerBroadcast(const std::vector<std::string>& params);      // pbc #n M
    void handlePlayerDeath(const std::vector<std::string>& params);          // pdi #n
    
    // Gestion des incantations
    void handleIncantationStart(const std::vector<std::string>& params);     // pic X Y L #n #n ...
    void handleIncantationEnd(const std::vector<std::string>& params);       // pie X Y R
    
    // Gestion des œufs
    void handleEggLaying(const std::vector<std::string>& params);            // pfk #n
    void handleNewEgg(const std::vector<std::string>& params);               // enw #e #n X Y
    void handleEggConnection(const std::vector<std::string>& params);        // ebo #e
    void handleEggDeath(const std::vector<std::string>& params);             // edi #e
    
    // Gestion des ressources
    void handleResourceDrop(const std::vector<std::string>& params);         // pdr #n i
    void handleResourceTake(const std::vector<std::string>& params);         // pgt #n i
    
    // Gestion du jeu
    void handleTimeUnit(const std::vector<std::string>& params);             // sgt T
    void handleTimeUnitSet(const std::vector<std::string>& params);          // sst T
    void handleEndGame(const std::vector<std::string>& params);              // seg N
    void handleServerMessage(const std::vector<std::string>& params);        // smg M
    
    // Gestion des erreurs
    void handleUnknownCommand(const std::vector<std::string>& params);       // suc
    void handleBadParameter(const std::vector<std::string>& params);         // sbp
    
    // Utilitaires de parsing
    int parseInteger(const std::string& str, const std::string& context = "") const;
    std::string parseString(const std::string& str) const;
    std::vector<int> parseResourceList(const std::vector<std::string>& params, 
        size_t start_index) const;
    std::vector<int> parsePlayerList(const std::vector<std::string>& params, 
        size_t start_index) const;
    
    // Validation
    bool validateParameters(const std::vector<std::string>& params, 
        size_t expected_count, 
        const std::string& command) const;
    bool validateCoordinates(int x, int y) const;
    bool validatePlayerId(int player_id) const;
    bool validateOrientation(int orientation) const;
    
    // Logging et debug
    void logCommand(const std::string& command, 
        const std::vector<std::string>& params) const;
    void logError(const std::string& message) const;
    void logWarning(const std::string& message) const;

    std::shared_ptr<zappy::game::world::SharedGameState> _game_state;
    // Map des handlers de commandes
    std::map<std::string, std::function<void(const std::vector<std::string>&)>> _command_handlers;

    std::vector<std::string> _pending_commands;
    mutable std::mutex _pending_mutex;
};
}
#endif /* !COMMANDPARSER_HPP_ */
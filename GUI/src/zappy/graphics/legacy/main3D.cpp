/*
** EPITECH PROJECT, 2025
** GUI [WSL: Ubuntu]
** File description:
** main3D
*/


#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>
#include <string>
#include <string_view>
#include <stdexcept>

#include "zappy/core/Application.hpp"
#include "zappy/network/utils/NetworkThread.hpp"
#include "zappy/game/world/SharedGameState.hpp"

std::atomic<bool> g_shutdown_requested{false};

void signal_handler(int signal) {
    std::cout << "\n🛑 Shutdown requested (signal " << signal << ")\n";
    g_shutdown_requested = true;
}

struct ProgramConfig {
    std::string host = "localhost";
    int port = 4242;
    bool help = false;
};

void print_usage(const char* program_name) {
    std::cout << "USAGE: " << program_name << " [OPTIONS]\n\n"
    << "Network mode:\n"
    << "  " << program_name << " -p port -h machine\n\n"
    << "OPTIONS:\n"
    << "  -p port          port number (default: 4242)\n"
    << "  -h machine       hostname of the server (default: localhost)\n"
    << "EXAMPLE:\n"
    << "  " << program_name << " -p 4242 -h localhost\n";
}

bool parse_arguments(int argc, char* argv[], ProgramConfig& config) {
    for (int i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];

        auto requires_argument = [&](int i) -> bool {
            return i + 1 < argc;
        };

        try {
            if (arg == "-p") {
                if (!requires_argument(i)) throw std::invalid_argument("-p requires a port number");
                config.port = std::stoi(argv[++i]);
                if (config.port <= 0 || config.port > 65535)
                    throw std::out_of_range("Invalid port number");
            } else if (arg == "-h") {
                if (!requires_argument(i)) throw std::invalid_argument("-h requires a hostname");
                config.host = argv[++i];
            } else if (arg == "--help" || arg == "help") {
                config.help = true;
                return true;
            } else {
                throw std::invalid_argument("Unknown option: " + std::string(arg));
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            return false;
        }
    }
    return true;
}

int run_network_mode(const ProgramConfig& config) {
    std::cout << "🌐 Starting Zappy GUI\n"
    << "   Host: " << config.host << "\n"
    << "   Port: " << config.port << "\n";

    auto shared_state = std::make_shared<zappy::game::world::SharedGameState>();
    auto app = std::make_unique<zappy::core::Application>(shared_state);

    if (!app->initialize()) {
        std::cerr << "❌ Failed to initialize application\n";
        return 84;
    }

    auto network_thread = std::make_unique<zappy::network::utils::NetworkThread>(
        config.host, config.port, shared_state);

    if (!network_thread->start()) {
        std::cerr << "❌ Failed to start network thread\n";
        return 84;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    if (network_thread->hasConnectionFailed()) {
        std::cerr << "❌ Network connection failed: " << network_thread->getLastError() << "\n";
        network_thread->stop();
        network_thread->join();
        return 84;
    }

    std::cout << "✅ Network connection established\n";

    // Run GUI main thread
    app->run();
    
    network_thread->stop();
    
    auto start_time = std::chrono::steady_clock::now();
    const auto timeout = std::chrono::seconds(2);
    
    while (network_thread->isRunning()) {
        if (std::chrono::steady_clock::now() - start_time > timeout) {
            std::cout << "Network thread did not stop gracefully, forcing shutdown\n";
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    network_thread->join();

    std::cout << "Shutdown complete\n";
    return 0;
}

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    ProgramConfig config;
    if (!parse_arguments(argc, argv, config)) {
        print_usage(argv[0]);
        return 84;
    }

    if (config.help) {
        print_usage(argv[0]);
        return 0;
    }

    try {
        return run_network_mode(config);
    } catch (const std::exception& e) {
        std::cerr << "❌ Fatal error: " << e.what() << "\n";
        return 84;
    } catch (...) {
        std::cerr << "❌ Unknown fatal error occurred\n";
        return 84;
    }
}

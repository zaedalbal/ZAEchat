#include "server/server.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <string>

int main(int argc, char* argv[])
{
    try
    {
        unsigned short port = DEFAULT_SERVER_PORT;

        // Если указан аргумент, используем его как порт
        if(argc > 1)
        {
            try
            {
                int port_arg = std::stoi(argv[1]);
                if(port_arg < 1024 || port_arg > 65535)
                {
                    std::cerr << "Error: Port must be between 1024 and 65535" << std::endl;
                    std::cerr << "Usage: " << argv[0] << " [port]" << std::endl;
                    return 1;
                }
                port = static_cast<unsigned short>(port_arg);
            }
            catch(const std::exception& e)
            {
                std::cerr << "Error: Invalid port number" << std::endl;
                std::cerr << "Usage: " << argv[0] << " [port]" << std::endl;
                return 1;
            }
        }

        boost::asio::io_context context;
        auto srv = std::make_shared<server>(context, port);

        srv->start();
        std::cout << "========================================" << std::endl;
        std::cout << "Chat server started successfully" << std::endl;
        std::cout << "Port: " << srv->get_port() << std::endl;
        std::cout << "Listening on: 0.0.0.0:" << srv->get_port() << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Waiting for connections..." << std::endl;

        context.run();
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Fatal error: " << ex.what() << std::endl;
        return 1; 
    }
    return 0;
}
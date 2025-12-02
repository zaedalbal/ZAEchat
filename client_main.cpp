#include "client/client_app.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

int main(int argc, char* argv[])
{
    try
    {
        std::string host = "127.0.0.1";
        std::string port = std::to_string(DEFAULT_SERVER_PORT);

        if(argc > 1)
        {
            host = argv[1];
        }
        if(argc > 2)
        {
            port = argv[2];
        }

        boost::asio::io_context io_context;
        auto app = std::make_shared<client_app>(io_context);
        
        std::cout << "Connecting to " << host << ":" << port << std::endl;
        app->connect(host, port);
        
        // ВАЖНО: Сначала обрабатываем никнейм СИНХРОННО
        app->send_nickname_and_start();
        
        std::thread io_thread([&io_context](){io_context.run();});
        
        // теперь обрабатываем обычные сообщения
        app->handle_user_input();
        
        if(io_thread.joinable())
        {
            io_thread.join();
        }
        std::cout << "Stopped" << std::endl;
    }
    catch(const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
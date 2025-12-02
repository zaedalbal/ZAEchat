#pragma once

#include <boost/asio.hpp>
#include "../defines.hpp"
#include <array>
#include <string>
#include <memory>
#include <atomic>

class client_app : public std::enable_shared_from_this<client_app>
{
    public:
        explicit client_app(boost::asio::io_context& io_context);

        void connect(const std::string& host, const std::string& port);

        void send_message(const std::string& message);

        void start_reading_from_server();
        
        void handle_user_input();

        void send_nickname_and_start();

    private:
        boost::asio::io_context& io_context_;
        boost::asio::ip::tcp::socket socket_;
        std::array<char, MESSAGE_BUFFER_SIZE> read_buffer_;
};
#pragma once

#include <boost/asio.hpp>
#include "../defines.hpp"
#include <array>
#include <string>
#include <memory>
#include <atomic>

class client_app : public std::enable_shared_from_this<client_app> // наследование от enable_shared чтобы хранить указатель на самого себя
                                                                   // (без этого во время какой либо асинхронной операции можно потерять указатель
                                                                   // и получить segmentation fault)
{
    public:
        explicit client_app(boost::asio::io_context& io_context); // конструктор

        void connect(const std::string& host, const std::string& port); // подключение к серверу

        void send_message(const std::string& message); // отправка сообщения

        void start_reading_from_server(); // чтение с сервера
        
        void handle_user_input(); // обработка ввода из консоли

        void send_nickname_and_start(); // отправка никнейма при первом подключении

    private:
        boost::asio::io_context& io_context_; // контекст boost asio
        boost::asio::ip::tcp::socket socket_; // сокет для связи с серваком
        std::array<char, MESSAGE_BUFFER_SIZE> read_buffer_; // буфер для чтения
};
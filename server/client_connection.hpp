#pragma once
#include <boost/asio.hpp>
#include "../defines.hpp"
#include <array>
#include <memory>
#include <string>

class server;

class client : public std::enable_shared_from_this<client> // наследование от enable_shared чтобы хранить указатель на самого себя
                                                           // (без этого во время какой либо асинхронной операции можно потерять указатель
                                                           // и получить segmentation fault)
{
    public:
        client(boost::asio::ip::tcp::socket sock); // конструктор

        void start(); // метод с которого запускается клиент

        void send_message(const std::string& str); // отправка данных клиенту

        std::string get_nickname();

        std::weak_ptr<server> server_ref; // указатель на сервер
    private:
        void disconnect(); // выход с сервера

        void start_reading(); // отправка данных серверу

        void request_client_nickname();

        void set_client_nickname(std::string nickname); // изменение имени пользователя

    private:
        boost::asio::ip::tcp::socket socket_; // сокет клиента

        std::string client_nickname_; // имя пользователя

        std::array<char, MESSAGE_BUFFER_SIZE> read_buffer_; // буфер для чтения
};
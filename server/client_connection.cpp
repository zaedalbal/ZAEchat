#include "client_connection.hpp"
#include "server.hpp"
#include <iostream>

client::client(boost::asio::ip::tcp::socket sock) : socket_(std::move(sock)), client_nickname_("Unnamed"){};

void client::start()
{
    request_client_nickname();
}

void client::disconnect()
{
    boost::system::error_code ec;
    socket_.close(ec);
    if(auto srv = server_ref.lock())
    {
        srv->remove_client(shared_from_this());
    }
    std::cout << "Client disconnected" << std::endl;
}

void client::start_reading()
{
    auto self = shared_from_this();
    socket_.async_read_some(boost::asio::buffer(read_buffer_, MESSAGE_BUFFER_SIZE),
    [self](const boost::system::error_code& ec, std::size_t bytes_transferred)
    {
        if(!ec)
        {
            std::string msg(self->read_buffer_.data(), bytes_transferred);
            std::cout << "Bytes transferred from client: " << bytes_transferred << std::endl;
            if(auto srv = self->server_ref.lock())
            {
                srv->broadcast(self, msg);
            }
            self->start_reading(); // заново читать данные
        }
        else
        {
            self->disconnect();
        }
    });
}

void client::send_message(const std::string& str)
{
    auto self = shared_from_this();
    boost::asio::async_write(socket_, boost::asio::buffer(str),
    [self](const boost::system::error_code& ec, size_t bytes_transferred)
    {
        if(!ec)
        {
            std::cout << "Bytes transferred to client: " << bytes_transferred << std::endl;
        }
        else
        {
            self->disconnect();
        }
    });
}

void client::set_client_nickname(std::string nickname)
{
    std::size_t MAX_NICKNAME_LEN = 64;
    if(nickname.size() > MAX_NICKNAME_LEN)
        nickname.resize(MAX_NICKNAME_LEN);
    while (!nickname.empty() && std::isspace((unsigned char)nickname.back()))
        nickname.pop_back();
    while (!nickname.empty() && std::isspace((unsigned char)nickname.front()))
        nickname.erase(nickname.begin());
    if(nickname.empty())
        nickname = "Unnamed";
    client_nickname_ = nickname;
    std::cout << "Cleint nickname set to: " << client_nickname_ << std::endl;
}

void client::request_client_nickname()
{
    auto self = shared_from_this();
    auto buffer = std::make_shared<std::array<char, MESSAGE_BUFFER_SIZE>>();
    send_message("Enter nickname: ");
    socket_.async_read_some(boost::asio::buffer(*buffer, MESSAGE_BUFFER_SIZE),
    [self, buffer](const boost::system::error_code &ec, std::size_t bytes_transferred)
    {
        if(!ec)
        {
            std::string nickname(buffer->data(), bytes_transferred);
            self->set_client_nickname(nickname);
            self->send_message("Welcome to the chat, " + self->client_nickname_ + "!\n");
            self->start_reading();
        }
        else
        {
            self->disconnect();
        }
    }
    );
}

std::string client::get_nickname()
{
    return client_nickname_;
}
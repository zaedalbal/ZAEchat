#include "client_connection.hpp"
#include "server.hpp"
#include <iostream>

client::client(boost::asio::ip::tcp::socket sock) : socket_(std::move(sock)), client_nickname_("Unnamed"){};

void client::start()
{
    auto self = shared_from_this();
    boost::asio::co_spawn(socket_.get_executor(),
    [self]()->boost::asio::awaitable<void>
    {
        co_await self->request_client_nickname();
    },
    boost::asio::detached);
}

void client::disconnect()
{
    boost::system::error_code ec;
    socket_.close(ec);
    if(auto srv = server_ref_.lock())
    {
        srv->remove_client(shared_from_this());
    }
    std::cout << "Client disconnected" << std::endl;
}

boost::asio::awaitable<void> client::start_reading()
{
    auto self = shared_from_this();
    try
    {
    while(socket_.is_open())
    {
        auto bytes_transferred = co_await socket_.async_read_some(boost::asio::buffer(read_buffer_), boost::asio::use_awaitable);
        std::string msg(read_buffer_.data(), bytes_transferred);
        if(auto srv = self->server_ref_.lock())
            srv->broadcast(self, msg);
    }
    }
    catch(const std::exception& ex)
    {
        self->disconnect();
    }
}

void client::send_message(const std::string& str)
{
    auto self = shared_from_this();
    boost::asio::co_spawn(socket_.get_executor(),
    [self, str]()->boost::asio::awaitable<void>
    {
        try
        {
            co_await boost::asio::async_write(self->socket_, boost::asio::buffer(str), boost::asio::use_awaitable);
        }
        catch(std::exception& ex)
        {
            self->disconnect();
        }
    },
    boost::asio::detached
    );
}

void client::set_client_nickname(std::string nickname)
{
    std::size_t MAX_NICKNAME_LEN = 64;
    nickname.erase(std::remove_if(nickname.begin(), nickname.end(),
    [](unsigned char c)
    {
        // только буквы, цифры, подчеркивание, дефис и пробелы
        return !(std::isalnum(c) || c == '_' || c == '-' || c == ' ');
    }
    ), nickname.end());
    if(nickname.size() > MAX_NICKNAME_LEN)
        nickname.resize(MAX_NICKNAME_LEN);
    while (!nickname.empty() && std::isspace((unsigned char)nickname.back()))
        nickname.pop_back();
    while (!nickname.empty() && std::isspace((unsigned char)nickname.front()))
        nickname.erase(nickname.begin());
    if(nickname.empty())
        nickname = "Unnamed";
    if(auto srv = server_ref_.lock())
    {
        nickname = srv->make_unique_nickname(nickname);
    }
    client_nickname_ = nickname;
    std::cout << "Client nickname set to: " << client_nickname_ << std::endl;
}

boost::asio::awaitable<void> client::request_client_nickname()
{
    auto self = shared_from_this();
    co_await boost::asio::async_write(socket_, boost::asio::buffer("Enter nickname: "), boost::asio::use_awaitable);
    auto bytes_transferred = co_await socket_.async_read_some(boost::asio::buffer(read_buffer_), boost::asio::use_awaitable);
    set_client_nickname(std::string(read_buffer_.data(), bytes_transferred));
    co_await boost::asio::async_write(socket_, boost::asio::buffer("Welcome to the chat, "+client_nickname_+"!\n"), boost::asio::use_awaitable);
    co_await start_reading();
}

std::string client::get_nickname()
{
    return client_nickname_;
}
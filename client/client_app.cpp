#include "client_app.hpp"
#include <iostream>
#include <thread>

client_app::client_app(boost::asio::io_context& io_context)
    : io_context_(io_context), socket_(io_context_){}

void client_app::connect(const std::string& host, const std::string& port)
{
    try
    {
        boost::asio::ip::tcp::resolver resolver(io_context_);
        auto endpoint = resolver.resolve(host, port);
        boost::asio::connect(socket_, endpoint);
        std::cout << "Connected to server" << std::endl;
        std::cout << "Host: " << host << std::endl;
        std::cout << "Port: " << port << std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void client_app::send_message(const std::string& raw_message)
{
    auto self = shared_from_this();
    std::string message = raw_message + "\n";
    boost::asio::async_write(socket_, boost::asio::buffer(message),
    [self, message](const boost::system::error_code &ec, std::size_t)
    {
        if(ec)
        {
            std::cerr << "Error: " << ec.message() << std::endl;
        }
    });
}

void client_app::start_reading_from_server()
{
    auto self = shared_from_this();
    socket_.async_read_some(boost::asio::buffer(read_buffer_), 
    [self](const boost::system::error_code& ec, std::size_t bytes_transferred)
    {
        if(!ec)
        {
            std::string message(self->read_buffer_.data(), bytes_transferred);
            std::cout << message;
            self->start_reading_from_server();
        }
        else
        {
            std::cerr << "Disconnected from server: " << ec.message() << std::endl;
            self->socket_.close();
        }
    });
}

void client_app::handle_user_input()
{
    std::string line;
    while(std::getline(std::cin, line))
    {
        if(line.empty())
            continue;
        if(line == "/quit" || line == "/exit")
        {
            std::cout << "Disconnecting" << std::endl;
            socket_.close();
            io_context_.stop();
            break;
        }
        send_message(line);
    }
}

void client_app::send_nickname_and_start()
{
    // ожидание приглашение от сервера
    std::array<char, MESSAGE_BUFFER_SIZE> buffer;
    boost::system::error_code ec;
    
    // синхронное чтение приглашения "Enter nickname: "
    size_t len = socket_.read_some(boost::asio::buffer(buffer), ec);
    if(ec)
    {
        std::cerr << "Error reading nickname prompt: " << ec.message() << std::endl;
        return;
    }
    
    // вывод приглашения
    std::cout << std::string(buffer.data(), len) << std::flush;
    
    // читаем никнейм от пользователя
    std::string nickname;
    std::getline(std::cin, nickname);
    
    // отправка никнейма синхронно
    nickname += "\n";
    boost::asio::write(socket_, boost::asio::buffer(nickname), ec);
    if(ec)
    {
        std::cerr << "Error sending nickname: " << ec.message() << std::endl;
        return;
    }
    
    // читаем приветственное сообщение
    len = socket_.read_some(boost::asio::buffer(buffer), ec);
    if(!ec && len > 0)
    {
        std::cout << std::string(buffer.data(), len) << std::flush;
    }
    
    start_reading_from_server();
}
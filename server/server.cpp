#include "server.hpp"
#include <iostream>

server::server(boost::asio::io_context& context, unsigned short port) :
io_context_(context),
port_(port),
acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
strand_(boost::asio::make_strand(io_context_))
{};

void server::start()
{
    accept_connection();
}

void server::accept_connection()
{
    acceptor_.async_accept(
    [this](const boost::system::error_code& ec, boost::asio::ip::tcp::socket client_socket)
    {
        if(!ec)
        {
            std::cout << "Client connected\n";
            auto t = std::make_shared<client>(std::move(client_socket));
            t->server_ref = shared_from_this();
            boost::asio::post(strand_, [this, t]()
            {
                clients_.insert(t);
            });
            clients_.insert(t);
            t->start();
        }
        else
        {
            std::cout << "Error: " << ec.message() << std::endl;
        }
        accept_connection();
    });
}

void server::broadcast(std::shared_ptr<client> sender, const std::string& raw_message)
{
    std::string message = sender->get_nickname() + ": " + raw_message;
    boost::asio::post(strand_, [this, sender, message]()
    {
        for(auto it = clients_.begin(); it != clients_.end();)
        {
            auto c = *it;
            if(c != sender) // не отправляем самому отправителю
            {
                try
                {
                    c->send_message(message);
                    ++it;
                }
                catch(...)
                {
                    it = clients_.erase(it);
                }
            }
            else
            {
                ++it; // пропуск отправителя
            }
        }
    });
}

void server::remove_client(std::shared_ptr<client> c)
{
    boost::asio::post(strand_, [this, c]()
    {    
    clients_.erase(c);
    });
    std::cout << "Removed client from server" << std::endl;
}

unsigned short server::get_port()
{
    return port_;
}
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
    auto self = shared_from_this();
    boost::asio::co_spawn(acceptor_.get_executor(),
        [self]() -> boost::asio::awaitable<void>
        {
            co_await self->accept_connection();
        },
        boost::asio::detached
    );
}

boost::asio::awaitable<void> server::accept_connection()
{
    auto self = shared_from_this();
    try
    {
        while(true)
        {
            boost::asio::ip::tcp::socket cliet_socket = co_await acceptor_.async_accept(boost::asio::use_awaitable);
            std::cout << "Client connected" << std::endl;
            auto t = std::make_shared<client>(std::move(cliet_socket));
            t->server_ref_ = self;
            boost::asio::post(strand_, [this, t]
            {
                clients_.insert(t);
                t->start();
            });
        }
    }
    catch(std::exception& ex)
    {
        std::cerr << "Accept error: " << ex.what();
    }
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

std::string server::make_unique_nickname(const std::string& raw_nickname)
{
    std::string nickname = raw_nickname;
    int counter = 1;
    bool nickname_exists = true;
    while(nickname_exists)
    {
        nickname_exists = false;
        for(const auto& client_ptr : clients_)
        {
            if(client_ptr->get_nickname() == nickname)
            {
                nickname_exists = true;
                nickname = raw_nickname + "(" + std::to_string(counter) + ")";
                counter++;
                break;
            }
        }
    }
    return nickname;
}

unsigned short server::get_port()
{
    return port_;
}
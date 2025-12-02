#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include "../defines.hpp"
#include "client_connection.hpp"
#include <set>
#include <memory>

class server : public std::enable_shared_from_this<server> // наследование от enable_shared чтобы хранить указатель на самого себя
                                                           // (без этого во время какой либо асинхронной операции можно потерять указатель
                                                           // и получить segmentation fault)
{
    public:
        server(boost::asio::io_context& context, unsigned short port); // конструктор
        
        void remove_client(std::shared_ptr<client> c); // удаление клиента

        void start(); // метод с запускается сервер

        void broadcast(std::shared_ptr<client> sender, const std::string& message); // рассылка всем кроме отправителя

        unsigned short get_port(); // получить порт

    private:
        void accept_connection(); // принимает клиента

    private:
        boost::asio::io_context& io_context_; // указатель на объект контекста ввода/вывода из boost asio
        boost::asio::ip::tcp::acceptor acceptor_; // acceptor чтобы принять соеденение клиента
        std::set<std::shared_ptr<client>> clients_; // контейнер с клиентами
        boost::asio::strand<boost::asio::io_context::executor_type> strand_; // для нормальной работы с потоками
        unsigned short port_;

};
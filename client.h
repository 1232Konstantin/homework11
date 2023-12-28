#ifndef CLIENT_H
#define CLIENT_H
#include <string>
#include <iostream>
#include <boost/asio.hpp>


using  boost::asio::ip::tcp;

class Client //синхронный клиент
{
    boost::asio::io_context io_context;
    tcp::socket m_socket;
public:
    Client(std::string port) : m_socket(io_context)
    {
        tcp::resolver resolver(io_context);
        boost::asio::connect(m_socket, resolver.resolve("127.0.0.1", boost::asio::string_view(port)));
        //boost::asio::connect(m_socket, resolver.resolve("127.0.0.1", "9000"));
    }
    void send(std::string data)
    {
        std::cout<<"\n> "<<data;
        boost::asio::write(m_socket, boost::asio::buffer(data));
        //получаем ответ

            boost::asio::streambuf responce;
            boost::asio::read_until(m_socket, responce, "\r\n");
            std::istream responce_stream(&responce);
            std::string _data;
            if (responce.size())
            {
                while(!std::getline(responce_stream,_data).eof())
                {
                    std::cout<<"< "<<_data<<std::endl;
                }

            }
    }
    void disconnect()
    {
        boost::system::error_code error;
        m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
        m_socket.close();
    }
};




#endif // CLIENT_H

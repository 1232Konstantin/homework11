#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
//#include <boost/locale.hpp>
#include <vector>
#include "common.h"

using namespace boost::asio;






class Session: public std::enable_shared_from_this<Session>
{
    ip::tcp::socket socket_m;
    const static size_t max_len=1025;
    char data_m[max_len];



public:
    Session(ip::tcp::socket socket) :socket_m(std::move(socket)) {}

    void start() { doRead();}

  private:
    void doRead()
    {
        auto self(shared_from_this());
        socket_m.async_read_some(buffer(data_m, max_len),
                   [this, self](boost::system::error_code error, std::size_t lenght) {

                        if (!error)
                        {
                            data_m[lenght]='\0';
                            std::string com(data_m);
                            std::vector<std::string> vector;
                            boost::algorithm::split(vector, com, [](char ch){return (ch=='\n');});

                            //очередь для комманд
                            MyQueue<UnitType>& db_requests_queue=MyQueue<UnitType>::get_instance();
                            for (auto x: vector)
                            {
                                if (x=="") break;
                                std::shared_ptr<UnitType> comm(new UnitType);
                                (*comm).first=x;

                                auto future= (comm->second).get_future(); //получаем фьючер из промиз
                                db_requests_queue.add(comm);
                                db_requests_queue.cv.notify_one(); //уведомляем обработчик команд
                                try
                                {
                                   std::string answer=future.get(); //ожидаем результат
                                   doWrite(answer);
                                   //doWrite(future.get());
                                }  catch (std::exception e)
                                {
                                    std::cout<<"Exception "<<e.what()<<std::endl;
                                }
                                
                            }

                        }
                        //else std::cout<<"Error:  "<<error.message()<<std::endl;
                        doRead();
        });

    }



 void doWrite(std::string& str)
    {
        auto self(shared_from_this());
        auto smart= std::make_shared<std::string>(std::move(str));
        async_write(socket_m, buffer(*smart), [this, self, smart](boost::system::error_code error, std::size_t)
        {
            if (error)
            {
                std::string utf8error=error.to_string();
                std::cout<<"SERVER WRITE ERROR: "<<utf8error<<std::endl;
            }
        });
    }

};

class Server
{
    ip::tcp::acceptor acceptor_m;
    std::vector<std::thread> m_threads;
    inline const auto static nThreads = std::thread::hardware_concurrency();

public:
    Server(io_context& io_context, short port) : acceptor_m(io_context, ip::tcp::endpoint(ip::tcp::v4(), port))
    {
        doAccept();

        //Use signal_set for terminate context
        signal_set signals{io_context, SIGINT, SIGTERM};
        //signal SIGINT will be send when user press Ctrl+C (terminate)
        //signal SIGTERM will be send when system terminate program
        signals.async_wait([&](auto, auto) { io_context.stop(); });

          m_threads.reserve(nThreads);
          for (unsigned int i = 0; i < nThreads; ++i) {
            m_threads.emplace_back([&io_context]() { io_context.run(); });
          }
          for (auto &th : m_threads) {
            th.join(); //this is io_context run procedure into every thread from threads
            //that will allow us to use multiple threads to work on client's messages
          }
    }
private:
    void doAccept()
    {
        acceptor_m.async_accept([this](boost::system::error_code error, ip::tcp::socket socket) {
           if (!error)
           {
               //std::cout<<"create session on:"<<socket.remote_endpoint().address().to_string()<<":"<<socket.remote_endpoint().port()<<"\n";
               std::make_shared<Session>(std::move(socket))->start();
           }
           else std::cout<<"Unfortunately an error is occured: "<<error.message()<<std::endl;
           doAccept();
        });
    }
};


#endif // SERVER_H

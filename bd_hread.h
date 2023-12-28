#ifndef BDTHREAD_H
#define BDTHREAD_H
#include "common.h"
//#include <fstream>
//#include <sstream>
#include <thread>
#include <functional>
#include "sqlite_wrapper.h"

namespace BDTread {     //Поток работающий с базой данных через очередькоманд

    inline static bool stop=false;

    template <typename F>
    struct final_action {
      final_action(F&& action) : action{std::move(action)} {}

      ~final_action() { action(); }
      F action;
    };

    //запуск потока обработки команд
    inline void bd_start()
    {
        //поток будет ждать работы на condition_variable из очереди db_requests_queue
        try {
            MyQueue<UnitType>& db_requests_queue=MyQueue<UnitType>::get_instance();
           // std::cout<<"\nstart bd thread"<<std::this_thread::get_id()<<std::endl;
            auto func=[&](std::unique_lock<std::mutex>& lock) //функция которая будет вызываться при наличии данных в очереди
            {
                auto comm=db_requests_queue.get();
                lock.unlock();

                std::list<std::string> list;
                //std::cout<<"*"<<comm->first<<std::endl;
                boost::algorithm::split(list, comm->first, [](char ch){return (ch==' ');});
                auto& bd=SQL::BDwrapper::get_instance();
                auto str1=list.front();
                list.pop_front();
                try
                {
                    auto temp=bd.execute(str1,list);//здесь вся обработка команд и работа с базой данных
                    //for (auto x: temp) std::cout<<"!"<<x<<std::endl;
                    //comm->second.set_value(std::move(temp));
                    //std::cout<<"++++"<<&(comm->second)<<std::endl;
                    comm->second.set_value(temp);
                }
                catch (std::exception e)
                {
                    std::cout<<"EXXXXCEPTION"<<e.what()<<std::endl;
                    comm->second.set_exception(std::make_exception_ptr(e));
                }


            };

        //подключаемся к очереди
        while (!BDTread::stop)
        {
            std::unique_lock<std::mutex> lock1(db_requests_queue.mutex);
            if (db_requests_queue.size()>0) func(std::ref(lock1)); //если очередь не пуста, рано вставать на ожидание. работаем!
            else //очередь пуста, встаем на ожидание
            {
                db_requests_queue.cv.wait(lock1, [&db_requests_queue]{return (db_requests_queue.size()>0);});
                func(std::ref(lock1));
            }
        }

        }  catch (...) {
            std::cout<<"There is a big trouble: Unexpected exception in bd_tread..."<<std::endl;
        }
    }



    inline void startBDThread()
    {
        try {
            //Запускаем в работу потоки loger, file1 и file2
            std::thread worker;
            worker= std::thread(bd_start);
            worker.detach();


        }  catch (...) {
            std::cout<<"There is a big trouble: Unexpected exception in startBDThread function..."<<std::endl;
        }

    }


}


#endif // BDTHREAD_H

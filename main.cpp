#include <iostream>
#include "server.h"
#include "client.h"
#include "bd_hread.h"
#include <thread>
#include <string>

using namespace std;

#define TEST 1

//Архитектура коротко:
//сервер многопоточный ассинхронный взаимодействует через очередь с потоком базы данных, в котором команды выполняются синхронно в порядке поступления.
//В сервере для каждого клиента создаем объект Session, загружающий полученные команды в очередь и отправляющий результат обратно клиенту.
//Для возврата значения в Session из потока базы данных используем std::promise, который передаем вместе с командой в std::pair







//Тесты
void test(short port)
{
    try {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        Client cl1(std::to_string(port));
        Client cl2(std::to_string(port));
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::list<std::string> list2 {"INSERT A 0 lean\n", "INSERT A 0 understand\n", "INSERT A 1 sweater\n", "INSERT A 2 frank\n", "INSERT A 3 violation\n", "INSERT A 4 quality\n", "INSERT A 5 precision\n", "INSERT B 3 proposal\n", "INSERT B 4 example\n", "INSERT B 5 lake\n"};
        //std::list<std::string> list3 {"INSERT B 6 flour\n","INSERT B 7 wonder\n", "INSERT B 8 selection\n", "INTERSECTION\n"};
        std::list<std::string> list3 {"INSERT B 6 flour\n","INSERT B 7 wonder\n", "INSERT B 8 selection\n", "INTERSECTION\n","SYMMETRIC_DIFFERENCE\n", "TRUNCATE A\n"};


        for(auto x: list2) cl1.send(x);
        for(auto x: list3) cl2.send(x);

        cl1.disconnect();
        cl2.disconnect();
    }  catch (std::exception e) { cout<<"Exception into test:  "<<e.what()<<endl;

    }


}



int main(int argc, char *argv[])
{
    short port=(argc==2)? std::stoi(std::string(argv[1])) : 9000;

    //Остановка потока обработки базы данных при закрытии приложения
    auto stop=BDTread::final_action([&]{BDTread::stop=true;});

    if (TEST)
    {
        //Запускаем двух клиентов с тестовыми данными
        auto res=std::thread(test, port);
        res.detach();
    }


    try
    {
        //Создаем поток для работы с базой данных
        BDTread::startBDThread();

        //Создаем и запускаем сервер
        io_context context;
        Server s(context, port);

    }
    catch (std::exception e)
    {
        cout<<"Wery bad Exception from Server:  "<<e.what()<<endl;
    }


    int i;
    cin>>i;



    return 0;
}

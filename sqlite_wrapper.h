#include "sqlite/sqlite3.h"

#include <cassert>
#include <string>
#include <iostream>
#include <stdexcept>
#include <list>
#include <map>

namespace SQL {

[[noreturn]] void sqlite_throw(int code, const char* msg = "") {
    std::string str="SQL Method failed: "+ std::string(sqlite3_errstr(code))+ " " +std::string(msg);
    std::cout<<"check exeption"<<str<<std::endl;
    throw std::runtime_error{str};
}

int sqlite_check(int code, const char* msg = "", int expected = SQLITE_OK) {
  if (code != expected) {
    sqlite_throw(code, msg);
  }
  return code;
}

int callback(void* payload, int col_num, char** str_data, char** col_names)
{
    std::string* rstr=reinterpret_cast<std::string*>(payload);

    for (int i = 0; i < col_num; i++)
    {
        std::string zap=(i==0)? "" : ",";
        std::string two="";
        if (str_data[i]!=NULL) two=std::string(str_data[i]);
        (*rstr)+=zap+two;
    }
    (*rstr)+="\n";
    return 0;
};


//********************************    command handlers      ***************************************
std::string insert (sqlite3*bd, std::list<std::string> args)
{
    if (args.size()!=3)
    {
        return std::string{"ERR invalid arguments\r\n"};
    }
    auto table=args.front();
    args.pop_front();
    if ((table!="A")&&(table!="B"))
    {
        return std::string{"ERR invalid table name\r\n"};
    }
    try
    {
        std::stoi(args.front());
    }
    catch (std::invalid_argument)
    {
        return std::string{"ERR invalid id\r\n"};
    }
    catch (std::out_of_range)
    {
        return std::string{"ERR invalid id\r\n"};
    }


    std::string str="INSERT INTO "+table+" VALUES ("+args.front()+", '"+args.back()+"');";
    //std::cout<<"## "<<str<<std::endl;
    char* errmsg{};
    int res = sqlite3_exec(bd,str.data(),nullptr, nullptr, &errmsg);
    if (res!=SQLITE_OK)
    {
        std::string temp="ERR "+std::string(errmsg)+"\r\n";
        return std::string{temp};
    }
    else return std::string{"OK\r\n"};

}

std::string truncate (sqlite3*bd, std::list<std::string> args)
{
    std::list<std::string> result;
    if (args.size()!=1)
    {
        return std::string{"ERR invalid arguments\r\n"};
    }
    auto table=args.front();
    args.pop_front();
    if ((table!="A")&&(table!="B"))
    {
        return std::string{"ERR invalid table name\r\n"};
    }

    std::string str="DELETE FROM "+table+";";
    char* errmsg{};
    int res = sqlite3_exec(bd,str.data(),nullptr, nullptr, &errmsg);
    if (res!=SQLITE_OK)
    {
        result.clear();
        std::string temp="ERR "+std::string(errmsg)+"\r\n";
        return std::string{temp};
    }
    else return std::string{"OK\r\n"};

}

std::string intersect (sqlite3*bd, std::list<std::string> args)
{
    std::string result;
    if (args.size())
    {
        return std::string{"ERR invalid arguments\r\n"};
    }
    std::string str="SELECT A.*, B.name FROM A INNER JOIN B ON A.id=B.id";

    char* errmsg{};
    int res = sqlite3_exec(bd,str.data(),SQL::callback, &result, &errmsg);
    if (res!=SQLITE_OK)
    {
        result.clear();
        std::string temp="ERR "+std::string(errmsg)+"\r\n";
        return std::string{temp};
    }
    else
    {
        result+="OK\r\n";
        return std::string{result};
    }
}

std::string diff (sqlite3*bd, std::list<std::string> args)
{
    std::string result;
    if (args.size())
    {
        return std::string{"ERR invalid arguments"};
    }
    std::string str="SELECT COALESCE(A.id, B.id) AS id, A.name, B.name FROM A FULL OUTER JOIN B ON A.id=B.id WHERE A.id IS NULL OR B.id IS NULL";

    char* errmsg{};
    int res = sqlite3_exec(bd,str.data(),SQL::callback, &result, &errmsg);
    if (res!=SQLITE_OK)
    {
        result.clear();
        std::string temp="ERR "+std::string(errmsg)+"\r\n";
        return std::string{temp};
    }
    else
    {
        result+="OK\r\n";
        return std::string{result};
    }

}
//******************************** end command handlers section ****************************************

//синглтон для работы с базой данных
//запросы в базу данных будем выпольнять синхронно, поэтому синглтон вполне подойдет
class BDwrapper
{
    typedef std::string (*Command_handler)(sqlite3*, std::list<std::string>);
    sqlite3* bd;
    std::map<std::string, Command_handler> handlers;

    BDwrapper()
    {
        sqlite_check(sqlite3_open("db", &bd), "bd not opened");
        handlers["INSERT"]=SQL::insert;
        handlers["TRUNCATE"]=SQL::truncate;
        handlers["INTERSECTION"]=SQL::intersect;
        handlers["SYMMETRIC_DIFFERENCE"]=SQL::diff;

        //тут создаем таблицы А и В
        std::string tabA{"CREATE TABLE IF NOT EXISTS A (id INTEGER PRIMARY KEY, name VARCHAR(255) NOT NULL);"};
        std::string tabB{"CREATE TABLE IF NOT EXISTS B (id INTEGER PRIMARY KEY, name VARCHAR(255) NOT NULL);"};
        sqlite_check(sqlite3_exec(bd, tabA.data(), nullptr, nullptr, nullptr), "bd create table A error");
        sqlite_check(sqlite3_exec(bd, tabB.data(), nullptr, nullptr, nullptr), "bd create table B error");
    }
    ~BDwrapper()
    {
        std::cerr<<"~BDwrapper\n";
        //очищаем базу данных и закрываем
        sqlite_check(sqlite3_exec(bd, "DROP TABLE A;", nullptr, nullptr, nullptr), "bd drop table A error");
        sqlite_check(sqlite3_exec(bd, "DROP TABLE B;", nullptr, nullptr, nullptr), "bd drop table B error");
        sqlite_check(sqlite3_close(bd), "bd not closed properly");
    }
    BDwrapper operator=(const BDwrapper&)=delete;
    BDwrapper(const BDwrapper&){};

    public:
    static BDwrapper& get_instance()
    {
        static BDwrapper instance;
        return instance;
    }
    std::string execute(std::string command, std::list<std::string> args)
    {
        //std::cout<<"@"<<command<<std::endl;
        if (handlers.find(command)!=handlers.end())
        return handlers[command](bd,args);
        else return std::string{"ERR uncknown command\r\n"};
    }

};






}//end namespace

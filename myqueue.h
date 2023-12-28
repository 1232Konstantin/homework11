#ifndef MYQUEUE_H
#define MYQUEUE_H
#include<list>
#include <memory>
#include <mutex>
#include <condition_variable>

//Очередь задач для обмена между потоками
//Использует mutex и contidional_variable для блокирования доступа
//очередь сделана синглтоном, чтоб все потоки точно работали гарантированно с одной и той же сущностью
template<typename T>
class MyQueue
{
  using Type=std::shared_ptr<T>;
    std::list<Type> queue;
    MyQueue(){}
    ~MyQueue(){}
    MyQueue(const MyQueue<T>& ){}
    MyQueue& operator=(const MyQueue<T>& ){}
public:
  std::mutex mutex;
  std::condition_variable cv;
  Type get()
  {
      auto res=queue.front();
      queue.pop_front();
      return res;
  }
  void add(Type t)
  {
      std::lock_guard<std::mutex> lock(mutex);
      queue.push_back(t);
  }
  size_t size()
  {
      return queue.size();
  }
  static MyQueue<T>& get_instance()
  {
      static MyQueue<T> instance;
      return instance;
  }

};


#endif // MYQUEUE_H

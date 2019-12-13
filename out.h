#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <queue>
#include <mutex> 
#include <chrono>
#include <atomic> 
#include <thread>
#include <sstream>
#include <algorithm>
#include <memory>
#include <list>
#include <iostream>
#include <chrono>
#include <map>
#include <cassert>

/// время первой команды + список всех команд
using commands_t = std::shared_ptr<std::pair<std::string, std::list<std::string> > >;

/// напечатать содержимое
template<typename T>
void print(T& output, const std::list<std::string>& list)
{
  for_each(list.cbegin(), list.cend(), [&] (const std::string &cmd) {
    output << cmd << " ";
  });
  output << "\n";
}

struct MyCout
{
  /// Вывести данные
  MyCout& operator << (const std::string &cmd) {
    std::cout << cmd;
    return *this;
  }
};

struct MyFile
{
  /// Конструктор
  /// @param time_str время в виде строки
  MyFile(std::string time_str, std::string suffix)
  {
    std::string name = std::string("bulk-") + suffix +
      std::string("-") + time_str + std::string(".log");
    file.open(name);
  }
  /// Деструктор
  ~MyFile() { file.close(); }
  /// вывести данные
  MyFile& operator << (const std::string &cmd) {
    file << cmd;
    return *this;
  }

private:
  std::ofstream file;
};


struct MyBase
{
  virtual void process_(commands_t cmds) = 0;
  
  void push(commands_t cmds)
  {
    std::lock_guard<std::mutex> lck(mtx);
    queue.push(std::move(cmds));
  }

  void process()
  {
    while ( true ) {
      commands_t cmds;
      {
        std::lock_guard<std::mutex> lck(mtx);
        if ( !queue.empty() ) {
          cmds = queue.front();
          queue.pop();
        } else if ( m_stop ) {
          break;
        }
      }
      if ( cmds ) 
        process_(cmds);

      std::this_thread::yield();
    }
  }

  std::atomic<bool> m_stop{false};
  std::queue<commands_t> queue;
  std::mutex mtx;
};


struct MyCountPrinter : public MyBase
{
  MyCountPrinter() : t(std::bind(&MyBase::process, this)) {}
  ~MyCountPrinter() 
  {
    m_stop = true;
    t.join();
  }
  void process_(commands_t cmds)
  {
    count_cmds += cmds->second.size();
    count_bloks += 1;
    MyCout my_cout;
    print(my_cout, cmds->second);
  } 

  void print_counts() const
  {
    std::cout << "log - " << count_cmds << " commands, " << count_bloks << " bloks" << std::endl;
  }

private:
  std::thread t;
  
  unsigned long count_cmds{0};
  unsigned long count_bloks{0};
};

struct MyFilePrinter : public MyBase
{
  MyFilePrinter()
    : t1(std::bind(&MyBase::process, this))
    , t2(std::bind(&MyBase::process, this))
  {
    thread2counts.insert(std::make_pair(t1.get_id(), thread_counts()));
    thread2counts.insert(std::make_pair(t2.get_id(), thread_counts()));
  }
  
  ~MyFilePrinter()
  {
    m_stop = true;
    t1.join();  
    t2.join();
  }  

  void process_(commands_t cmds)
  {
    auto id = std::this_thread::get_id();
    auto it = thread2counts.find(id);
    assert(it != thread2counts.end());
    it->second.cmds += cmds->second.size();
    it->second.bloks++;

    std::ostringstream ss;
    ss << id;
    std::string idstr = ss.str();

    MyFile myfile(cmds->first, idstr);
    print(myfile, cmds->second);
  }

  void print_counts() const
  {
    auto it1 = thread2counts.find(t1.get_id());
    assert(it1 != thread2counts.end());
    auto it2 = thread2counts.find(t2.get_id());
    assert(it2 != thread2counts.end());

    std::cout << "file1 - " << it1->second.cmds << " commands, " << it1->second.bloks << " bloks" << std::endl;;
    std::cout << "file2 - " << it2->second.cmds << " commands, " << it2->second.bloks << " bloks" << std::endl;;
  }
  
private:
  std::thread t1;
  std::thread t2;  
  struct thread_counts {
    unsigned long cmds{0};
    unsigned long bloks{0};
  };
  std::map<std::thread::id, thread_counts> thread2counts;
};

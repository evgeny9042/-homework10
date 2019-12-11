

#include "./out.h"

//#include <cassert>
//#include <stdlib.h>
#include <string>
#include <list>
#include <iostream>
#include <algorithm>
#include <chrono>
//#include <fstream>


/// Команды в блоке
struct BulkCommands
{
  /// добавить команду
  void push(std::string cmd) {
    if ( m_commands.empty() ) {
      m_time = std::to_string(std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now()));
    }
    m_commands.emplace_back(std::move(cmd));
  }
  /// напечатать все команды в блоке и очистить  
  void flush() {    
    if ( !m_commands.empty() ) {
      /// выводим в стандартный поток
      MyCout my_cout;
      print(my_cout);

      ///  выводим в файл
      MyFile myfile(m_time);
      print(myfile);

      m_commands.clear();
    }
  }
  
  /// напечатать содержимое
  template<typename T>
  void print(T& output) {
    for_each(m_commands.begin(), m_commands.end(), [&] (const std::string &cmd) {
      output << cmd << " "; 
    });
    std::cout << std::endl;
  }
  /// размер блока
  size_t size() const { return m_commands.size(); }

private:
  std::string m_time;                  ///< время первой команды
  std::list<std::string> m_commands;   ///< команды
};


int main(int argc, char const *argv[])
{
  if ( argc != 2 ) return 0;
  // количество команд в блоке
  int N = atoi(argv[1]); 
  if ( N <= 0 ) return 0;

  BulkCommands commands;
  size_t braces = 0;

  std::string cmd;
  while ( std::cin >> cmd ) 
  {
    if ( cmd.find("{") != std::string::npos ) {
      braces++;
      if ( braces == 1 ) {
        commands.flush();
      }
    } else if ( cmd.find("}") != std::string::npos ) {
      braces--;
      if ( braces == 0 ) {
        commands.flush();
      }
    } else {
      commands.push(std::move(cmd));
      if ( braces == 0 && commands.size() == N ) {
        commands.flush();
      }
    }
  }

  if ( braces == 0 ) {
    commands.flush();
  }

	return 0;
}

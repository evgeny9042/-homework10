#pragma once

#include <string>
#include <fstream>
#include <iostream>

struct MyCout
{
  /// Вывести данные
  MyCout& operator << (const std::string &cmd)
  {
    std::cout << cmd;
    return *this;
  }
};


struct MyFile
{      
  /// Конструктор
  /// @param time_str время в виде строки
  explicit MyFile(std::string time_str)
  {
    std::string name = std::string("bulk") + time_str + std::string(".log");
    file.open(name);
  }
  /// Деструктор
  ~MyFile()
  {
    file.close();
  }
  /// Вывести данные
  MyFile& operator << (const std::string &cmd)
  {
    file << cmd;
    return *this;
  }

private:
  std::ofstream file;
};
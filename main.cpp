

#include "./out.h"

/// Команды в блоке
struct BulkCommands
{
  /// Конструктор
  BulkCommands() { m_commands.reset(new std::pair<std::string, std::list<std::string> >()); }
  /// добавить команду
  void push(std::string cmd) {
    if ( m_commands->second.empty() ) {
      m_commands->first = std::to_string(std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now()));
    }
    m_commands->second.emplace_back(std::move(cmd));
  }
  /// напечатать все команды в блоке и очистить  
  bool flush() {    
    if ( !m_commands->second.empty() ) {
      /// выводим в файл
      m_file_printer.push(m_commands);      
      /// выводим в стандартный поток
      m_cout_printer.push(m_commands);
      /// заново создаем, старый удалиться, когда будет ненужен
      m_commands.reset(new std::pair<std::string, std::list<std::string> >());
      return true;
    }
    return false;
  }
  /// размер блока
  size_t size() const { return m_commands->second.size(); }

  void print_counts() const
  {
    m_cout_printer.print_counts();
    m_file_printer.print_counts();
  }

private:
  //std::string m_time;      ///< время первой команды  
  commands_t m_commands;   ///< команды

  MyFilePrinter m_file_printer;
  MyCountPrinter m_cout_printer;
};


int main(int argc, char const *argv[])
{
  if ( argc != 2 ) return 0;
  // количество команд в блоке
  int N = atoi(argv[1]); 
  if ( N <= 0 ) return 0;

  unsigned long count_strings = 0;
  unsigned long count_cmds    = 0;
  unsigned long count_bloks   = 0;
  
  BulkCommands bulk_commands;
  size_t braces = 0;

  std::string cmd;
  while ( std::cin >> cmd ) 
  {
    count_strings++;
    if ( cmd.find("{") != std::string::npos ) {
      braces++;
      if ( braces == 1 ) {
        if ( bulk_commands.flush() )
          count_bloks++;
      }
    } else if ( cmd.find("}") != std::string::npos ) {
      braces--;
      if ( braces == 0 ) {
        if ( bulk_commands.flush() )
          count_bloks++;
      }
    } else {
      count_cmds++;
      bulk_commands.push(std::move(cmd));
      if ( braces == 0 && bulk_commands.size() == N ) {
        if ( bulk_commands.flush() ) {
          count_bloks++;
        }
      }
    }
  }

  if ( braces == 0 ) {
    bulk_commands.flush();
  }

  std::cout << "main - " << count_strings << " strings, "
    << count_cmds << " commands, " << count_bloks << " bloks" << std::endl;
  
  bulk_commands.print_counts();
	return 0;
}

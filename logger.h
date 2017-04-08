#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <cstdarg>
#include <ctime>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>

#define ERROR 1
#define WARN 2
#define INFO 4
#define DEBUG 8
#define TRACE 1024

#define LOG_LEVEL 4
#define DEFAULT_LOG_FILE "/var/log/activeco.log"

extern std::unordered_map<std::string, int> logLevelToSwitch;
class cLogger {
  private:
    
    std::string __name;
    std::mutex __mutex;
    std::ofstream __logfile;
    std::ostringstream __msg;
    int __isOpen;

    bool __checkFile(std::string fileName) {
      this->__logfile.open(fileName, std::ofstream::out | std::ofstream::app);
      this->__isOpen = this->__logfile.is_open();
      if (!this->__isOpen) {
        std::cerr << "ERROR opening : " << fileName << std::endl;
        return false;
      }
      return true;
    }

    std::string __levelStr(int level) {
      std::string str;
      switch (level) {
        case INFO : str="INFO" ; break;
        case WARN : str="WARN" ; break;
        case ERROR : str="ERROR" ; break;
        case DEBUG : str="DEBUG" ; break;
        case TRACE : str="TRACE" ; break;
        default : str="INFO" ; break;
      }
      return str;
    }

    void __logWalk(int level) {
      char timeBuffer[100];
      std::time_t now = std::time(nullptr);
      if (level <= this->logLevel && this->__isOpen) {
        if (std::strftime(timeBuffer, sizeof(timeBuffer), "%F %T", std::localtime(&now))) {
          //std::thread::id id=std::this_thread::get_id();
          //std::cout << timeBuffer << " thr=" << id << " msg=[" << msg.str() << "]" << " logl=" << this->logLevel << "   inlogl=" << level <<std::endl;
          std::string str = this->__levelStr(level);
          this->__logfile << timeBuffer << " " << this->__name << " " << str << " msg=[" << this->__msg.str() << "]" <<std::endl;
          this->__logfile.flush();
          this->__msg.str("");
        }
      }
    }

    template<typename Head, typename ...Tail>
    void __logWalk(int level, Head head, Tail ...tail) {
      this->__msg << head;
      this->__logWalk(level, tail...);
    }


  public:
    std::string fileName;
    int logLevel;

    cLogger() : fileName(DEFAULT_LOG_FILE), logLevel(LOG_LEVEL) {
      this->__isOpen = false;
      this->__name = "";
      this->__checkFile(this->fileName);
    }
 
    cLogger(std::string fileName, int logLevel) : fileName(fileName), logLevel(logLevel) {
      this->__isOpen = false;
      this->__checkFile(this->fileName);
    }

    ~cLogger() {
      if (this->__isOpen) {
        this->__logfile.close();
      }
    }
   
    void setLogLevel(int level) {
      this->logLevel = level;
    }

    void setFileName(std::string fileName) {
      this->fileName = fileName;
      if (this->__isOpen) {
        this->__logfile.close();
      }
      this->__isOpen = false;
      this->__checkFile(this->fileName);
    }

    void setLoggerName(std::string name) {
      this->__name = name;
    }

    template<typename... Parts>
    void log(int level, Parts... parts) {
      this->__mutex.lock();
      if (level <= this->logLevel && this->__isOpen) {
        this->__logWalk(level, parts...);
      }
      this->__mutex.unlock();
    }
};

#define INIT_LOOGER cLogger logger;

extern cLogger logger;
#define LOG(int, ...) logger.log(int, __VA_ARGS__);

#endif

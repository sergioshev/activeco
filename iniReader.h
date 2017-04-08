#ifndef _INI_READER_
#define _INI_READER_

#define DEFAULT_INI_FILE "/etc/activeco/activeco_cam.ini"

#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>
#include <exception>

namespace po = boost::program_options;

class iniReader {
  protected:
    std::string filename;
    std::string lastError;
    po::options_description* piniStructureDescription;
    po::variables_map* piniValuesMap;
    /*
      funcion para construir el objeto que describe las opciones
      permitidas en el archivo ini
    */
    void buildStructureDescription();

    /*
      funcion para leer el archivo ini utilizando la definicion
      construida por buildStructureDescription
    */

  public:
    iniReader();
    iniReader(const std::string filename);
    ~iniReader();
    po::variables_map* getMap();
    void readFile();
    std::string getLastError();
    bool validateValues();
    int getLogLevel();
};

//productores de elementos del ini

class abstractFactory {
  protected:
    const po::variables_map & vm;

  public:
    abstractFactory(const po::variables_map & evm) : vm{evm} {};
    //OJO aca! 
    //Es responsabilidad del llamador a liberar la memoria
    virtual void* produce()=0;
};

class logLevelFactory : public abstractFactory {
  public:
    logLevelFactory(const po::variables_map & evm) : abstractFactory(evm) {};
    void* produce();
};

class logFileFactory : public abstractFactory {
  public:
    logFileFactory (const po::variables_map & evm) : abstractFactory(evm) {};
    void* produce();
};

class pointNameFactory : public abstractFactory {
  public:
    pointNameFactory(const po::variables_map & evm) : abstractFactory(evm) {};
    void* produce();
};

class urlFactory : public abstractFactory {
  public:
    urlFactory(const po::variables_map & evm) : abstractFactory(evm) {};
    void* produce();
};

class chronoObserverFactory : public abstractFactory {
  public:
    chronoObserverFactory(const po::variables_map & evm) : abstractFactory(evm) {};
    void* produce();
};

class moveObserverFactory : public abstractFactory {
  public:
    moveObserverFactory(const po::variables_map & evm) : abstractFactory(evm) {};
    void* produce();
};

class dumper2DbFactory : public abstractFactory {
  public:
    dumper2DbFactory(const po::variables_map & evm) : abstractFactory(evm) {};
    void* produce();
};

class dumper2FileFactory : public abstractFactory {
  public:
    dumper2FileFactory(const po::variables_map & evm) : abstractFactory(evm) {};
    void* produce();
};


#endif

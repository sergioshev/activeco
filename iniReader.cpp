#include <memory>
#include "iniReader.h"
#include "logger.h"
#include "observers.h"
#include "dumper.h"

void iniReader::buildStructureDescription() {
  this->piniStructureDescription = new(po::options_description);
  this->piniStructureDescription->add_options()
    /*
      opciones globales:
      [global]
        logLevel = trace | debug | info | error | warn
        logFile = /var/log/activeco.log (default)
    */
    ("global.logLevel", po::value<std::string>()->default_value("info"),
      "Nivel de log de activeco")
    ("global.logFile", po::value<std::string>()->default_value("/var/log/activeco.log"),
      "Nombre de archivo de log")
    /*
      [camera]
        url = rtsp:://user:pass@address...
        pointName = calado | bruto | pv5i | pv5d ... etc
    */
    ("camera.url", po::value<std::string>(), 
      "url para definir la conexion rtsp")
    ("camera.pointName", po::value<std::string>(),
      "Nombre del puesto de control")
    /* observadores = [ moveObserver | chronoObserver ] ... */ 
    ("camera.observers", po::value<std::vector<std::string>>(),
      "lista de los observadores de frames")
    /* dumpers = [ dumper2Db | dumper2File ] ... */
    ("camera.dumpers", po::value<std::vector<std::string>>(),
      "dumpers adores de frames")
    /*moveObserver*/
    ("moveObserver.pointX", po::value<int>(),"x del ROI")
    ("moveObserver.pointY", po::value<int>(),"y del ROI")
    ("moveObserver.width", po::value<int>(),"ancho del ROI")
    ("moveObserver.height", po::value<int>(),"alto del ROI")
    /*chronoObserver*/
    ("chronoObserver.timeout", po::value<int>()->default_value(10),
      "tiempo de espera en segundos para tomar el frame")
    ("dumper2Db.dbName", po::value<std::string>(),
      "nombre de la base de datos")
    ("dumper2Db.dbIp", po::value<std::string>(),
      "ip del servidor de la base de datos")
    ("dumper2Db.dbPort", po::value<std::string>(),
      "puerto del servidor de la base de datos")
    ("dumper2File.fsPath", po::value<std::string>(),
      "directorio donde almacenar los frames")
  ;
}

void iniReader::readFile() {
  std::filebuf fb;
  std::filebuf* ptr = fb.open(this->filename.c_str(), std::ios::in);

  if (ptr == nullptr) {
    std::cerr << "No puedo abrir el archivo " << this->filename << std::endl;
    this->lastError = "No se puede abrir el archivo de configuracion";
    throw std::string("File open error");
  }
  std::istream iniStream(&fb);
  this->buildStructureDescription();
  this->piniValuesMap = new (po::variables_map);
  try {
    po::store(
      po::parse_config_file(
        iniStream, 
        *this->piniStructureDescription,
        true),
      *this->piniValuesMap
    );
    po::notify(*this->piniValuesMap);
  } catch (std::exception e) {
    std::cerr << "Errores leyendo archivo ini" << std::endl;
    this->lastError = "Errores detectados en la configuracion";
  }
  fb.close();
}

std::string iniReader::getLastError() {
  return this->lastError;
}

iniReader::iniReader() {
  this->piniValuesMap = NULL;
  this->piniStructureDescription = NULL;
  this->lastError = "";
  this->filename = DEFAULT_INI_FILE;
}

iniReader::iniReader(const std::string filename) {
  this->piniValuesMap = NULL;
  this->piniStructureDescription = NULL;
  this->lastError = "";
  this->filename = filename;
}

iniReader::~iniReader() {
  if (this->piniValuesMap != NULL) {
    delete(this->piniValuesMap);
  }
  if (this->piniStructureDescription != NULL) {
    delete(this->piniStructureDescription);
  }
}

po::variables_map* iniReader::getMap() {
  return this->piniValuesMap;
}

//validacion basica, solo chequea la presencia de los valores
//obligatorios
//la semantica de los datos esta delegada a los productores
bool iniReader::validateValues() {
  //TODO: no salir si existe un valor por 
  po::variables_map & ivm = *this->piniValuesMap;
  //logLevel
  if (!ivm.count("global.logFile")) {
    this->lastError = "No esta definida la variable logFile";
//    return false;
  }
  if (!ivm.count("global.logLevel")) {
    this->lastError = "No esta definida la variable logLevel";
//    return false;
  } else {
    std::string logLevelStr = ivm["global.logLevel"].as<std::string>();
    for (auto & c : logLevelStr) {
      c = toupper(c);
    }
    try {
      logLevelToSwitch.at(logLevelStr.c_str());
      //int j = logLevelToSwitch.at(logLevelStr.c_str());
    } catch (std::exception & e) {
      this->lastError = "La variable logLevel tiene un valor desconocido";
      return false;
    }
  }
  if (!ivm.count("camera.pointName")) {
    this->lastError = "No esta definida la variable pointName";
    return false;
  }
  if (!ivm.count("camera.url")) {
    this->lastError = "No esta definida la variable url";
    return false;
  }
  if (!ivm.count("camera.observers")) {
    this->lastError = "No estan definidos los observadores";
    return false;
  }
  if (!ivm.count("camera.dumpers")) {
    this->lastError = "No estan definidos los volcadores (dumpers)";
    return false;
  }
  
  return true; 
}


//productores de lo elementos
void* logLevelFactory::produce() {
  std::string logLevelStr = this->vm["global.logLevel"].as<std::string>();
  int* pLogLevel= new(int);
  *pLogLevel = 0;
  for (auto & c : logLevelStr) {
    c = toupper(c);
  }
  *pLogLevel = logLevelToSwitch.at(logLevelStr.c_str());
  return pLogLevel;
}

void* pointNameFactory::produce() {
  std::string *ppointName = new std::string;
  *ppointName = this->vm["camera.pointName"].as<std::string>();
  return ppointName;
}

void* logFileFactory::produce() {
  std::string *logFile = new std::string;
  *logFile = this->vm["global.logFile"].as<std::string>();
  return logFile;
}

void* urlFactory::produce() {
  std::string *url = new std::string;
  *url = this->vm["camera.url"].as<std::string>();
  return url;
}

void* chronoObserverFactory::produce() {
  cChronoObserver* pobs=NULL;
  const po::variables_map & vm = this->vm;
  std::vector<std::string> obs = vm["camera.observers"].as<std::vector<std::string>>();
  bool isPresent = false;

  for (const auto & it : obs) {
    if (it.compare(std::string("chronoObserver")) == 0) {
      isPresent = true;
      break;
    }
  }
  if (! isPresent) {
    return NULL;
  }
  if (!vm.count("chronoObserver.timeout")) {
    return NULL;
  }
  int timeout = vm["chronoObserver.timeout"].as<int>();
  if (timeout < 0) {
    return NULL;
  }
  pobs = new cChronoObserver(timeout);
  return pobs;
}

void* moveObserverFactory::produce() {
  cMoveObserver* pobs=NULL;
  const po::variables_map & vm = this->vm;
  std::vector<std::string> obs = vm["camera.observers"].as<std::vector<std::string>>();
  bool isPresent = false;

  for (const auto & it : obs) {
    if (it.compare(std::string("moveObserver")) == 0) {
      isPresent = true;
      break;
    }
  }
  if (! isPresent) {
    return NULL;
  }
  if (!vm.count("moveObserver.pointX") ||
      !vm.count("moveObserver.pointY") ||
      !vm.count("moveObserver.width") ||
      !vm.count("moveObserver.height")) {
    return NULL;
  }
  int pointX = vm["moveObserver.pointX"].as<int>();
  int pointY = vm["moveObserver.pointY"].as<int>();
  int width = vm["moveObserver.width"].as<int>();
  int height = vm["moveObserver.height"].as<int>();

  pobs = new cMoveObserver(pointX, pointY, width, height);
  return pobs;
}

void* dumper2DbFactory::produce() {
  dumper2Db* pdumper=NULL;
  const po::variables_map & vm = this->vm;
  std::vector<std::string> dumpers = vm["camera.dumpers"].as<std::vector<std::string>>();
  bool isPresent = false;

  for (const auto & it : dumpers) {
    if (it.compare(std::string("dumper2Db")) == 0) {
      isPresent = true;
      break;
    }
  }
  if (! isPresent) {
    return NULL;
  }
  if (!vm.count("dumper2Db.dbName") ||
      !vm.count("dumper2Db.dbIp") ||
      !vm.count("dumper2Db.dbPort")) {
    return NULL;
  }
  std::string dbName = vm["dumper2Db.dbName"].as<std::string>();
  std::string dbIp = vm["dumper2Db.dbIp"].as<std::string>();
  std::string dbPort = vm["dumper2Db.dbPort"].as<std::string>();

  pdumper = new dumper2Db(dbName, dbIp, dbPort);
  return pdumper;
}

void* dumper2FileFactory::produce() {
  dumper2File* pdumper=NULL;
  const po::variables_map & vm = this->vm;
  std::vector<std::string> dumpers = vm["camera.dumpers"].as<std::vector<std::string>>();
  bool isPresent = false;

  for (const auto & it : dumpers) {
    if (it.compare(std::string("dumper2File")) == 0) {
      isPresent = true;
      break;
    }
  }
  if (! isPresent) {
    return NULL;
  }
  if (!vm.count("dumper2File.fsPath")) {
    return NULL;
  }
  std::string fsPath = vm["dumper2File.fsPath"].as<std::string>();

  pdumper = new dumper2File(fsPath);
  return pdumper;
}


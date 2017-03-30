#include <memory>
#include "iniReader.h"
#include "logger.h"

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
    /*chronoObserver*/
    ("chronoObserver.timeout", po::value<int>()->default_value(10),
      "tiempo de espera en segundos para tomar el frame")
    ("dumper2Db.dbName", po::value<std::string>(),
      "nombre de la base de datos")
    ("dumper2Db.dbIp", po::value<std::string>(),
      "ip del servidor de la base de datos")
    ("dumper2Db.dbPort", po::value<int>(),
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


bool iniReader::validateValues() {
  po::variables_map & ivm = *this->piniValuesMap;
  //logLevel
  if (!ivm.count("global.logFile")) {
    this->lastError = "No esta definida la variable logFile";
    return false;
  }
  if (!ivm.count("global.logLevel")) {
    this->lastError = "No esta definida la variable logLevel";
    return false;
  } else {
    std::string logLevelStr = ivm["global.logLevel"].as<std::string>();
    for (auto & c : logLevelStr) {
      c = toupper(c);
    }
    try {
      int j = logLevelToSwitch.at(logLevelStr.c_str());
    } catch (std::exception & e) {
      this->lastError = "La variable logLevel tiene un valor desconocido";
      return false;
    }
  }   
  return true; 
}

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

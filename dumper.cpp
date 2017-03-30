#include "dumper.h"
#include "logger.h"
#include <ctime>
#include <opencv2/imgproc/imgproc.hpp>

ENABLE_LOGGER

dumper2Db::dumper2Db(std::string cDbName, std::string cHost, std::string cPort) {
  this->dbName = cDbName;
  this->host = cHost;
  this->port = cPort;
}

dumper2Db::~dumper2Db() {
}

void dumper2Db::dump(detectionData data) {
 
std::string cConnect = "dbname=" + this->dbName + " user=activeco password=4ct1v3c0" +
                        " hostaddr=" + this->host + " port=" + this->port ;
 try{
    pqxx::connection C(cConnect);
      if (C.is_open()) {
        LOG(DEBUG, "{dumper2Db::dump} ", "Base de datos se abrio Exitosamente ", C.dbname());  
        pqxx::work transaction(C);
        
        char timeBuffer[100];
        std::time_t now = std::time(nullptr);
        std::strftime(timeBuffer, sizeof(timeBuffer), "%F %T", std::localtime(&now));        
        
        std::ostringstream sentence;
        sentence <<  "INSERT INTO activeco (patente, fecha_de_referencia, "  <<
                     " nivel_de_confianza, puesto_de_control) " <<
                     "VALUES (" << "'"<< data.plate << "'"<< ", " << "'" << timeBuffer << "' ," <<
                     data.confidence << ", " << "'"<< data.pointName << "');" ;
                                  
       pqxx::result r = transaction.exec(sentence.str());
       LOG(INFO, "{dumper2Db::dump} ", "Se inserto la patente ", data.plate);
       
       transaction.commit();
       C.disconnect();

      } else {
         LOG(ERROR, "{dumper2Db::dump} No se pudo abrir la base de datos");
      }   
      C.disconnect (); 
   }catch (const std::exception &e){
      std::cerr << e.what() << std::endl;
   }  
}


dumper2File::dumper2File(std::string cPath){
  this->path = cPath;
}

dumper2File::~dumper2File(){
}

void dumper2File::dump(detectionData data){
  std::ostringstream  name;
  name << this-> path << data.plate  << "_" << data.pointName << ".jpg";
  std::vector<int> v;
  v.push_back(1);
 
  try {
      cv::imwrite( name.str(), data.frame, v );
      LOG(INFO, "{dumper2File::dump} ", "Se grabo la imagen de la  patente ", data.plate, " en el punto ", data.pointName );
  } catch   (const std::exception &e){
      LOG(ERROR, "{dumper2File::dump} No se pudo guardar la imagen");
 }
    
}

void dumperFrame2File::dump(detectionData data){
  char timeBuffer[100];
  std::time_t now = std::time(nullptr);
  std::strftime(timeBuffer, sizeof(timeBuffer), "%F %T", std::localtime(&now));
     
  std::ostringstream  name;
  name << this-> path << timeBuffer << ".jpg";
  std::vector<int> v;
  v.push_back(1);
 
  try {
    cv::imwrite( name.str(), data.frame, v );
    LOG(INFO, "{dumperFrame2File::dump} ", "Se grabo el frame de la cola de proceso en ", name.str().c_str() );
  } catch   (const std::exception &e){
    LOG(ERROR, "{dumperFrame2File::dump} No se pudo guardar el frame");
  }
}

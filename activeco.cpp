#include <boost/program_options.hpp>
#include <cctype>
#include <exception>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <pqxx/pqxx>
#include <stdio.h>
#include <unistd.h>
#include <thread>

#include "logger.h"
INIT_LOOGER

#include "framesDispatcher.h"
#include "dumper.h"
#include "iniReader.h"
#include "observers.h"
#include "threadSafeQueue.h"
#include "videoStreamReader.h"
#include "vpmr.h"

namespace po = boost::program_options;

//bandera para cortar el bucle del thread consumidor de frames
int queuedFramesConsumerRun = 1;

void frameReadyCallback(cv::Mat frame, void* ptr) {
  if (ptr != NULL) {
    if (frame.cols != -1 && frame.rows != -1) {
      ((cFrameDispatcher<cFrameObserver>*)ptr)->dispatch(frame);
    }
  }
}

void queuedFramesConsumer(cThreadSafeQueue<cv::Mat>* queue, clibVpar* pvpar, std::string pointName, std::vector<dumper*> dumpers ) {
  cv::Mat frame;
  long numberOfPlates;
  long res;
  char plateText[100];
  float confidence;
  detectionData ddata; 

  LOG(INFO, "{queuedFramesConsumer} Se lanza el consumidor de frames");
  while (queuedFramesConsumerRun) {
    queue->pop(frame);
    LOG(DEBUG, "{queuedFramesConsumer} Se recibe el frame desde la cola ", frame.cols, "x", frame.rows);
    //cv::imshow("TEST", frame);
    ddata.frame = frame;
    //TODO agregar cuando se configure el INI, aqui para guardar frame de la cola de deteccion (movimiento o chrono)
    res = pvpar->vpmrReadRGB24(frame.cols, frame.rows, frame.ptr(), false);
    // Se pudo leer la imagen
    if (res) {
      numberOfPlates = pvpar->vpmrGetNumberOfPlates();
      if (numberOfPlates > 0) {
        pvpar->vpmrGetText(plateText, 0);
        confidence = pvpar->vpmrGetGlobalConfidence(0);
        // la pantente se reconocio por encima del umbra minimo
        if (confidence >= MIN_CONFIDENCE) {
          ddata.pointName = pointName;
          ddata.confidence = confidence;
          ddata.plate = plateText;
          for (dumper* it : dumpers) {
            it->dump(ddata);
            LOG(INFO, "{dumpFrame} Patente reconocida ", ddata.plate, " con confianza ", ddata.confidence);
          }
          //cv::imshow("TEST", frame);
        }
      } //if (numberOfPlates)
    } // if (res)
  } // while
}

/*
 Funcion para liberar una lista de punteros.
 recorre la lista llamando delete si el puntero
 existe
*/
template<typename H>
void freeVars(H p) {
  p ? delete(p) : (void)0;
}

template <typename H, typename ...T>
void freeVars(H h, T ...t) {
  freeVars(h);
  freeVars(t...);
}


int main(int argc, char **argv) {
  po::options_description args("Argumentos de linea de comando");

  args.add_options()
    ("help", "activeco [--help] [--config | -c] archivo_camara.ini ")
    ("config,c", po::value<std::string>(), "Archivo de configuracion de la camara")
  ;
  po::variables_map vm;
  try {
    po::store(po::parse_command_line(argc, argv, args), vm);
    po::notify(vm);
  } catch (std::exception &e) {
    std::cout << "Algunas opciones no se reconocieron" << std::endl;
    std::cout << args << std::endl;
  }

  if (vm.count("help")) {
    std::cout << args << std::endl;
    return 0;
  }

  if (!vm.count("config")) {
    std::cout << "Falta el archivo de configuracion de la camara (.ini)" << std::endl;
    std::cout << args << std::endl;
    return 0;
  }
  
  iniReader ir(vm["config"].as<std::string>());
  try {
    ir.readFile();
  } catch (std::exception &e) {
    std::cerr << "Excepcion durante la lectura de la configuración" << std::endl;
    return 1;
  }
  if (!ir.validateValues()) {
    std::cerr << "Se detectaron valores invalidos en la configuración" << std::endl;
    std::cerr << "ERROR: " << ir.getLastError() << std::endl;
    return 1;
  }

  po::variables_map & ivm = *ir.getMap();

  logLevelFactory logLevelF(ivm);
  logFileFactory logFileF(ivm);
  pointNameFactory pointNameF(ivm);
  urlFactory urlF(ivm);
  chronoObserverFactory chronoF(ivm);
  moveObserverFactory moveObsF(ivm);
  dumper2DbFactory dumper2DbF(ivm);
  dumper2FileFactory dumper2FileF(ivm);

  int* logLevel = (int*)logLevelF.produce();
  std::string* logFile = (std::string *)logFileF.produce();
  std::string* pointName = (std::string *)pointNameF.produce();
  std::string* url = (std::string*)urlF.produce();

  cChronoObserver* chronoObs = (cChronoObserver*)chronoF.produce();
  cMoveObserver* moveObs = (cMoveObserver*)moveObsF.produce();
  dumper2Db* d2Db = (dumper2Db*)dumper2DbF.produce();
  dumper2File* d2File = (dumper2File*)dumper2FileF.produce();
  std::vector<dumper*> dumpers;

// Iniciando la aplicacion
  logger.setLogLevel(*logLevel);
  logger.setFileName(*logFile);
  logger.setLoggerName(*pointName);

  clibVpar* pvpar;

  LOG(INFO, "Inicializando la libreria vpar");
  try {
    pvpar = new clibVpar();
  } catch (std::exception& e) {
    //e.what();
    LOG(ERROR, "Error inicializando la libreria vpar");
    freeVars(pvpar, logFile, logLevel, pointName, url, chronoObs,
      moveObs, d2Db, d2File);
    return 1;
  }

  long res = pvpar->vpmrInit(204, -1, false, false, 0, true);
  if (!res) {
    LOG(ERROR, "Error inicializando el motor de reconocimiento");
  }

  //Cola de frames que son resibidos de los observadores.
  //Sirve como canal entre vpar y el flujo de frames listos para analizarse.
  cThreadSafeQueue<cv::Mat> framesQueue;

  //Despachador de frames a los observadores registrados
  cFrameDispatcher<cFrameObserver> frameDispatcher;

  if (chronoObs) {
    chronoObs->setQueue(&framesQueue);
    frameDispatcher.addObserver(chronoObs);
  }
  if (moveObs) {
    moveObs->setQueue(&framesQueue);
    frameDispatcher.addObserver(moveObs);
  }
  if (d2Db) { dumpers.push_back(d2Db); }
  if (d2File) { dumpers.push_back(d2File); }

  //El lector del flujo de frames propiamente dicho.
  //Esta clase ejecuta la funcion "callback" por cada
  //frame recibido desde la camara. Se setea el user
  //poiner con el framesDispacher que es pasado como 
  //parametro en el callback para depacharlo a los observadores
  cvStreamReader sr(*url, frameReadyCallback);
  sr.setUserPointer(&frameDispatcher);

  //cv::namedWindow("TEST", CV_WINDOW_AUTOSIZE); 
  LOG(INFO, "Iniciando la captura de frames desde streamReader");
  try {
    sr.startCapture();
  } catch (std::exception e) {
    LOG(ERROR, "Error iniciando la lectura ");
    freeVars(pvpar, logFile, logLevel, pointName, url, chronoObs,
      moveObs, d2Db, d2File);
    return 1;
  }

  std::thread th(queuedFramesConsumer, &framesQueue, pvpar, *pointName, dumpers);

  while (queuedFramesConsumerRun) {
    if (cv::waitKey(30) >=0) {
      queuedFramesConsumerRun = 0;
      sr.stopCapture();
    }
  }
 
  th.join();
  sr.stopCapture();
  LOG(INFO, "Se detuvo el streamReader");
  pvpar->vpmrEnd();
  LOG(INFO, "Se detuvo el motor de reconocimiento VPAR");
  freeVars(pvpar, logFile, logLevel, pointName, url, chronoObs,
    moveObs, d2Db, d2File);
  return 0;
}

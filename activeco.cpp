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

void dumpFrame(detectionData d) {
  LOG(INFO, "{dumpFrame} Patente reconocida ", d.plate, " con confianza ", d.confidence);
  dumper2Db dbDumper("laridae", "192.168.1.6", "5433");
  dumper2File fileDumper("/var/lib/activeco/patentes/");
  dbDumper.dump(d);
  fileDumper.dump(d);
}

void frameReadyCallback(cv::Mat frame, void* ptr) {
  if (ptr != NULL) {
    if (frame.cols != -1 && frame.rows != -1) {
      ((cFrameDispatcher<cFrameObserver>*)ptr)->dispatch(frame);
    }
  }
}

void queuedFramesConsumer(cThreadSafeQueue<cv::Mat>* queue, clibVpar* pvpar, std::string pointName ) {
  cv::Mat frame;
  int j;
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
          dumpFrame(ddata);
          //cv::imshow("TEST", frame);
        }
      } //if (numberOfPlates)
    } // if (res)
  } // while
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

  int* logLevel = (int*)logLevelF.produce();
  std::cout << "Loglevel final = " << *logLevel << std::endl;
  delete (logLevel);
  return 0;

// Iniciando la aplicacion
  char *pointName;

  logger.setLogLevel(*logLevel);

  clibVpar* pvpar;

  LOG(INFO, "Inicializando la libreria vpar");
  try {
    pvpar = new clibVpar();
  } catch (std::exception& e) {
    //e.what();
    LOG(ERROR, "Error inicializando la libreria vpar");
    if ( pvpar != NULL ) {
      delete(pvpar);
    }
    return 1;
  }

  long res = pvpar->vpmrInit(204, -1, false, false, 0, true);
  if (!res) {
    LOG(ERROR, "Error inicializando el motor de reconocimiento");
    //std::cerr << "Fallo al inicializar VPAR." << std::endl;
  }

  //const std::string videoStreamAddress = "rtsp://admin:c3b4d4@192.168.1.32";
  //const std::string videoStreamAddress = "rtsp://admin:v1s1b1l1d4d@192.168.1.50";
  pointName = argv[1];


  std::string videoStreamAddress;
  const std::string prefectura = "rtsp://admin:c3b4d4@192.168.1.31";
  const std::string calado = "rtsp://admin:c3b4d4@192.168.1.32";
  const std::string bruto = "rtsp://admin:c3b4d4@192.168.1.45";
  const std::string pv5d = "rtsp://admin:v1s1b1l1d4d@192.168.1.50";
  const std::string pv5i = "rtsp://admin:&c3b4d4%@192.168.1.61";

  videoStreamAddress = calado;

  int px,py,width;

  px=py=width=20;

  if ( strcmp(pointName, "CALADO") == 0 ) {
    std::cout << "CALADO seteado" << std::endl;
    videoStreamAddress=calado;
    logger.setLoggerName(std::string("CALADO"));
    px=500;
    py=400;
    width=30;
  } 

  if ( strcmp(pointName, "PREFECTU") == 0 ) {
    std::cout << "PREFECTURA seteado" << std::endl;
    videoStreamAddress=prefectura;
    logger.setLoggerName(std::string("PREFECT"));
    px=870;
    py=260;
    width=30;
  } 

  if ( strcmp(pointName, "BRUTO") == 0 ) {
    std::cout << "BRUTO seteado" << std::endl;
    videoStreamAddress=bruto;
    logger.setLoggerName(std::string("BRUTO"));
    px=870;
    py=260;
    width=80;
  } 

  if ( strcmp(pointName, "PV5I") == 0 ) {
    std::cout << "PV5I seteado" << std::endl;
    videoStreamAddress=pv5i;
    logger.setLoggerName(std::string("PV5I"));
    px=700;
    py=450;
    width=30;
  } 

  if ( strcmp(pointName, "PV5D") == 0 ) {
    std::cout << "PV5D seteado" << std::endl;
    videoStreamAddress=pv5d;
    logger.setLoggerName(std::string("PV5D"));
    px=760;
    py=480;
    width=30;
  } 


  //Cola de frames que son resibidos de los observadores.
  //Sirve como canal entre vpar y el flujo de frames listos para analizarse.
  cThreadSafeQueue<cv::Mat> framesQueue;

  //Observador cronometrado
  cChronoObserver chronoObserver(10);
  chronoObserver.setQueue(&framesQueue);

  //Observador por Movimiento
  cMoveObserver moveObserver(px, py, width, width);
  moveObserver.setQueue(&framesQueue);

  //Despachador de frames a los observadores registrados
  cFrameDispatcher<cFrameObserver> frameDispatcher;

  if ( strcmp(pointName, "BRUTO") == 0 ) {
    frameDispatcher.addObserver(&chronoObserver);
  } else {
    frameDispatcher.addObserver(&moveObserver);
  }

  //El lector del flujo de frames propiamente dicho.
  //Esta clase ejecuta la funcion "callback" por cada
  //frame recibido desde la camara. Se setea el user
  //poiner con el framesDispacher que es pasado como 
  //parametro en el callback para depacharlo a los observadores
  cvStreamReader sr(videoStreamAddress, frameReadyCallback);
  sr.setUserPointer(&frameDispatcher);

  //cv::namedWindow("TEST", CV_WINDOW_AUTOSIZE); 
  LOG(INFO, "Iniciando la captura de frames desde streamReader");
  try {
    sr.startCapture();
  } catch (std::exception e) {
    //std::cerr << "No puedo arrancar la lectura" << e.what() << std::endl;
    //LOG(ERROR, "Error iniciando la lectura ", e.what());
    LOG(ERROR, "Error iniciando la lectura ");
    return 1;
  }

  std::thread th(queuedFramesConsumer, &framesQueue, pvpar, std::string(pointName));


  while (queuedFramesConsumerRun) {
    if (cv::waitKey(30) >=0) {
      queuedFramesConsumerRun = 0;
      sr.stopCapture();
    }
  }
 

  th.join();
  sr.stopCapture();
  //std::cout << "Se detuvo el stream" << std::endl;
  LOG(INFO, "Se detuvo el streamReader");
  pvpar->vpmrEnd();
  //std::cout << "VPAR parado." << std::endl;
  LOG(INFO, "Se detuvo el motor de reconocimiento VPAR");
  return 0;
}

#include <unistd.h>

#include "logger.h"
#include "observers.h"

ENABLE_LOGGER

//cronoObserver

void cChronoObserver::__start() {
  while (this->runFlag) {
    usleep(USLEEP_CHRONO_OBSERVER);
    if (this->current>0) {
      int i = this->current;
      LOG(TRACE, "{cChronoObserver::__start } Timer actual=", i);
      this->current--;
    }
  }
}

void cChronoObserver::__reset() {
  LOG(TRACE, "{cChronoObserver::__reset} Se resetea el reloj");
  this->current = this->timeout;
}

void cChronoObserver::__stop() {
  this->runFlag = 0;
}

cChronoObserver::cChronoObserver(int t) {
  this->timeout = t;
  this->current = t;
  this->runFlag = 1;
  this->th = std::thread(&cChronoObserver::__start, this);
}

cChronoObserver::~cChronoObserver() {
  this->__stop();
  this->th.join();
}

int cChronoObserver::isReady() {
  return (this->current <= 0);
}

void cChronoObserver::notify(cv::Mat frame) {
 // std::chrono::time_point<std::chrono::system_clock> now;
 // std::ostringstream fileName("frame_", std::ios_base::ate);
  if (this->isReady()) {
    //cv::imshow("TEST", frame);
    this->__reset();
  //  now = std::chrono::system_clock::now();
  //  fileName << std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count() << ".bmp";
  //  std::cout << "El frame se envia a la cola. " << fileName.str() << std::endl;
    LOG(DEBUG, "{cChronoObserver::notify} El frame se envia a la cola");
  this->queue->push(frame);
/*
    std::vector<int> v;
    v.push_back(1);
    cv::imwrite(fileName.str(), frame, v);*/
  }
}

// moveObserver
cMoveObserver::cMoveObserver(Rect roi) {
  this->roi = roi;
  this->lastValueR = 0;
  this->lastValueG = 0;
  this->lastValueB = 0;
  this->confidence = 0.75;
}

cMoveObserver::cMoveObserver(int x, int y, int width, int height) {
  this->roi.topLeft.x = x;
  this->roi.topLeft.y = y;
  this->roi.width = width;
  this->roi.height = height;
  this->lastValueR = 0;
  this->lastValueG = 0;
  this->lastValueB = 0;
  this->confidence = 0.75;
}

cMoveObserver::~cMoveObserver() {
}

void cMoveObserver::setConfidence(float confidence) {
  this->confidence = confidence;
}

float cMoveObserver::getConfindence() {
  return this->confidence;
}

Rect cMoveObserver::getRoi() {
  return this->roi;
}


int cMoveObserver::isReady() {
  return 1;
}

void cMoveObserver::notify(cv::Mat frame) {
  int i = getRoi().topLeft.x;
  int j = getRoi().topLeft.y;
  int w = getRoi().width;
  int h = getRoi().height;
  int x, y;
  cv::Vec3b pic; 
  float promR, promG, promB;
  float conf = this->confidence;  

  //cv::imshow("TEST", frame);
  promR = promG = promB = 0;
  for (x = 1; x <= w; x++) 
    for (y = 1; y <= h; y++) {
      pic = frame.at<cv::Vec3b>(j+y, i+x); 
      promR+=pic.val[0];
      promG+=pic.val[1];
      promB+=pic.val[2];
   }
  promR = promR/(w+h);
  promG = promG/(w+h);
  promB = promB/(w+h); 

  if (!((promR/lastValueR >= conf) &&
      (promR/lastValueR <= (2-conf)))) {
    this->queue->push(frame);
    LOG(DEBUG, "{cMoveObserver::notify} Hay Movimiento, el frame se envia a la cola");
  }
 
  lastValueR = promR;
  lastValueG = promG;
  lastValueB = promB;
  
  LOG(TRACE, "{cMoveObserver::notify } Promedio actual=", lastValueR, ";" ,lastValueG, ";",  lastValueB);
} 
// fin cMoveObserver

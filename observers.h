#ifndef OBSERVERS_H
#define OBSERVERS_H

#include <atomic>
#include <thread>
#include <opencv2/opencv.hpp>

#include "threadSafeQueue.h"

#define USLEEP_CHRONO_OBSERVER 1000000 

struct Point {
  int x;
  int y;
};

struct Rect {
   Point topLeft;
   int width;
   int height;
};  

/*
 * Clase abstracta de un observador de frames
 *  se usa para especificar la interfaz de
 *  los diferentes observadores
 */

class cFrameObserver {
  protected:
    cThreadSafeQueue<cv::Mat>* queue;

  public:
    virtual void notify(cv::Mat frame)=0;
    virtual int isReady()=0;
    void setQueue(cThreadSafeQueue<cv::Mat>* q) {
      this->queue = q;
    }
};

/*
 * Observador, basado en un cronometro. Consume frames solo
 *   luego de que ocurre un timeout
 */

class cChronoObserver : public cFrameObserver {
  private:
    int runFlag;
    int timeout;
    std::atomic<int> current;
    std::thread th;

    void __start();
    void __reset();
    void __stop();
  
  public:
    cChronoObserver(int t);
    ~cChronoObserver();
    void notify(cv::Mat frame);
    int isReady();
};

/* Observador basado en el movimiento detectado dentro
 * de una zona de interes ROI.
 */

class cMoveObserver : public cFrameObserver {
  private: 
    Rect roi;
    float lastValueR, lastValueG, lastValueB;
    float confidence;

  public:
    cMoveObserver(Rect roi);
    cMoveObserver(int x, int y, int width, int height);
    ~cMoveObserver();
    void notify(cv::Mat frame);
    int isReady();
    void setConfidence(float confidence);
    float getConfindence();
    Rect getRoi();
};

#endif

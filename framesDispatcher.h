#ifndef FRAMES_DISPATCHER_H
#define FRAMES_DISPATCHER_H

#include <opencv2/opencv.hpp>
#include <vector>

/*
 * Despachador de frames.
 *   Notifica a todos los observadores registrados.
 *   Por el momento estan definidos dos observadores
 *   1) observador cronometrado
 *   2) observador de movimiento
 * 
 *   Es una plantilla, que luego se parametriza con la 
 *     clase base cFrameObserver. Ya que cualquier 
 *     observador nuevo tiene que extender esa 
 *     clase generica, redefiniendo los metodo isReady()
 *     y notify()
 *
 *   NOTA: por ser una platilla al menos el compilador GNU 
 *     de c++, exige que la definicion de la clase tiene que
 *     ocurrir en el archivo .h. Si se mueve el codigo a .cpp
 *     ocurren errores en tiempo de linkeo.
 */

template<class O> 
class cFrameDispatcher {
  private:
    std::vector<O*> observers;
    
  public:
    cFrameDispatcher();
    ~cFrameDispatcher();
    
    void addObserver(O* o);
    void dispatch(cv::Mat frame);
};

template<class O>
cFrameDispatcher<O>::cFrameDispatcher() {
}

template<class O>
cFrameDispatcher<O>::~cFrameDispatcher() {
/*  typename std::vector<O*>::iterator i;
  for (i = this->observers.begin() ;
       i != this->observers.end() ;
       i++ ) {
      //delete(*i);
  }*/
}

template<class O>
void cFrameDispatcher<O>::addObserver(O* o) {
  this->observers.push_back(o);
}

template<class O>
void cFrameDispatcher<O>::dispatch(cv::Mat frame) {
  typename std::vector<O*>::iterator i;
  for (i = this->observers.begin() ;
       i != this->observers.end() ;
       i++ ) {
    (*i)->notify(frame);
  }
}

#endif

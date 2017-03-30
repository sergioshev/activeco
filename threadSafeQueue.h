#include <condition_variable>
#include <mutex>
#include <queue>
//#include <unistd.h>


#ifndef TS_QUEUE
#define TS_QUEUE

#define MAX_QUEUE_LEN 300

template<class T>
class cThreadSafeQueue {
  private:
    std::mutex __mutex;
    std::condition_variable __queueEmpty;
    std::queue<T> __queue;
    
  public:
    cThreadSafeQueue();
    ~cThreadSafeQueue();

    bool empty();
    void push(T const &elem);
    //Importante: pop es un metodo que dormirá antes de sacar el elemento si 
    // el conjuto es vacio.
    // pop() devuelve el parametro en el parametro por referencia
    // esto es necesario para evitar excepciones al crear objetos si se usa
    // retorno por copia. Si se retorna por copia "T pop()" se hara una
    // copia del elemento que se saca, si falla el constructor al momento 
    // de crear el objeto se pierde el dato sacado, ya que no esta en la cola
    // y el constructor fallo.
    void pop(T &elem);
    int size();
};

template<class T>
cThreadSafeQueue<T>::cThreadSafeQueue() {
}

template<class T>
cThreadSafeQueue<T>::~cThreadSafeQueue() {
  //std::unique_lock<std::mutex> lockVar(this->__mutex);
}

template <class T>
bool cThreadSafeQueue<T>::empty() {
  std::unique_lock<std::mutex> lockVar(this->__mutex);
  return this->__queue.empty();
}

template<class T>
void cThreadSafeQueue<T>::push(T const& elem) {
  std::unique_lock<std::mutex> lockVar(this->__mutex);
  if (this->__queue.size() < MAX_QUEUE_LEN) {
    this->__queue.push(elem); 
  }
  lockVar.unlock();
  this->__queueEmpty.notify_all();
}

template<class T>
void cThreadSafeQueue<T>::pop(T& elem) {
  std::unique_lock<std::mutex> lockVar(this->__mutex);
  // Esto es necesario debido al llamado "spurious wakeup"
  // Wikipedia dice...
  //   Spurious wakeup describes a complication in the 
  //   use of condition variables as provided by certain
  //   multithreading APIs such as POSIX Thread.
  //
  // Basicamente significa que podes ser despertado cuando
  // el evento por el cual estabas esperando no ocurrio.
  // Entonces lo tenes que volver a chequear y volver a dormir
  // si resulto falso.
  while (this->__queue.empty()) {
    this->__queueEmpty.wait(lockVar);
  }
  elem = this->__queue.front();
  this->__queue.pop();
//  usleep(4000*1000);
}

template<class T>
int cThreadSafeQueue<T>::size() {
  std::unique_lock<std::mutex> lockVar(this->__mutex);
  return this->__queue.size();
}

#endif

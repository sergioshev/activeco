#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <stdio.h>

#include "threadSafeQueue.h"

#define PRODUCERS 15
#define CONSUMERS 10


void consumer(int id, int sleepMilliseconds, cThreadSafeQueue<int>& q) {
  int value;
  std::chrono::duration<int, std::ratio<1, 1000>> span(sleepMilliseconds);

  while (true) {
    q.pop(value);
    printf("Consumidor[ %d ] valor=%d  en cola=%d\n", id, value, q.size());
    std::this_thread::sleep_for(span);
  }
}

void producer(int id, int sleepMilliseconds, cThreadSafeQueue<int>& q) {
  int value;
  std::chrono::duration<int, std::ratio<1, 1000>> span(sleepMilliseconds);

  while (true) {
    int value = std::rand() % 1000;    
    q.push(value);
    printf("Productor[ %d ] valor=%d en cola=%d\n", id, value, q.size());
    std::this_thread::sleep_for(span);
  }
}

int main(int argc, char **argv) {
  cThreadSafeQueue<int> q;

  std::thread prods[PRODUCERS];
  std::thread consumers[CONSUMERS];
  int j;

  std::srand(std::time(0)); //seteo la semilla
  for (j=0 ; j<PRODUCERS ; j++) {
    std::cout << "Lanzando productor " << j << std::endl;
    prods[j] = std::thread(producer, j, 1, std::ref(q));
  }

  for (j=0 ; j<CONSUMERS ; j++) {
    std::cout << "Lanzando consumidor " << j << std::endl;
    consumers[j] = std::thread(consumer, j, 1, std::ref(q));
  }
  
  (*prods).join();
  (*consumers).join();
  
  return 0;
}

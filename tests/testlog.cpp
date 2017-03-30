#include <thread>
#include <chrono>

#include "logger.h"
INIT_LOOGER

void runThr(int thrid) {
  for (int i=0; i<10 ; i++) {
    LOG(DEBUG,  "Thread ", thrid, " i= ", i);
    //std::this_thread::sleep_for(std::chrono::milliseconds(thrid));
    std::this_thread::sleep_for(std::chrono::seconds(thrid));
  }
}

int main(int argc, char **argv) {

  LOG(INFO,  "esto", "es", "un ", "log que ", "toma", 1, 2, 3);
  logger.setLogLevel(DEBUG);
  std::thread th1=std::thread(runThr, 1);
  std::thread th2=std::thread(runThr, 2);
  th1.join();
  th2.join();
  logger.setLogLevel(DEBUG);
  logger.setFileName("/var/log/activeco_2.log");
  std::thread th3=std::thread(runThr, 3);
  th3.join();
  return 0;
}

#include <iostream>
#include <pqxx/pqxx>
#include "dumper.h"
#include "logger.h"

INIT_LOOGER

int main(int argc, char* argv[])
{
  dumper2Db testDumper("laridae", "5433", "192.168.1.6");
  detectionData prof;
  cv::Mat frame;
  prof.frame = frame;
  prof.pointName = "CALADO";
  prof.plate = "ZZZ999" ;
  prof.confidence = 78.4;

  testDumper.dump(prof);

return 1;
}

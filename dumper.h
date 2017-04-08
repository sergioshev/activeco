#include <opencv2/opencv.hpp>
#include <iostream>
#include <pqxx/pqxx>


typedef struct{
  cv::Mat frame;
  std::string pointName;
  std::string plate;
  float confidence;
} detectionData;

class dumper {
  public:
    virtual void dump(detectionData data)=0;
    virtual ~dumper() {};
};


class dumper2Db : public dumper {
  private:
    std::string dbName;
    std::string host;
    std::string port;
  public:   
    dumper2Db(std::string cDbName, std::string cHost, std::string cPort);
    ~dumper2Db(); 
    void dump(detectionData data);
};

class dumper2File : public dumper {
  protected:
    std::string path;
  public: 
   dumper2File(std::string cPath);
   ~dumper2File(); 
   void dump(detectionData data);  

};

class dumperFrame2File : public dumper2File {
  public:
    dumperFrame2File(std::string cPath):dumper2File{cPath} {};
    ~dumperFrame2File();
    void dump(detectionData data);
};

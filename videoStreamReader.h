#ifndef VIDEOSTREAM_H
#define VIDEOSTREAM_H

//#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <thread>
//#include <vector>

#ifdef __cplusplus
extern "C" {
  #include <libavcodec/avcodec.h>
  #include <libavformat/avformat.h>
  #include <libswscale/swscale.h>
}
#endif

//tasa de entrega de frames
//en milisegundos
#define FRAME_FEED_PERIOD 300

#define MILLISECONDS_PER_FRAME 30
//#define MILLISECONDS_PER_FRAME 3

//Valor maximo de fallas en una funcion bloqueante
//Se usa en el callback del AVFormatContext
#define BLOCK_FUNCTION_MAX_COUNT 4

//Segundos de espera entre intentos de reconexion
//al flujo de rtsp.
#define RECONNECT_PAUSE 5

/**
 * Stream reader, clase encargada de recibir el flujo de frames
 *   desde la camara
 **/

// definicion de la interfaz del callback para el evento
// de un frame recibido
typedef void (*cbFrameReady)(cv::Mat frame, void* userPointer);

class cvStreamReader {
private:
  // callback a ser llamado cuando este listo el frame.
  cbFrameReady cb;
  std::string url;
  std::thread th;
  int frameFeedPeriod;
  int runFlag;
  int abortBlockingCount;
  void* userPointer;

private:
  void __pollFrames();

public:
  static int __blokingFunctionCallback(void* p);
  void startCapture();
  void stopCapture();
  void stopStream();
  int isRunning();
  void setUserPointer(void* ptr);

  // ejemplo : "rtsp://admin:pass@192.168.1.51/axis-cgi/mjpg/video.cgi";
  cvStreamReader(std::string url, cbFrameReady cb);
  ~cvStreamReader();
};

#endif 

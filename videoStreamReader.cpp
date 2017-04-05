#include <chrono>
#include <thread>
#include "videoStreamReader.h"

#include "logger.h"

ENABLE_LOGGER

cvStreamReader::cvStreamReader(
  std::string url,
  cbFrameReady cb) {
  this->cb = cb;
  this->url = url;
  this->runFlag = 0;
  this->frameFeedPeriod = FRAME_FEED_PERIOD;
  this->userPointer = NULL;
  this->abortBlockingCount = BLOCK_FUNCTION_MAX_COUNT;
}

cvStreamReader::~cvStreamReader() {
//  if (this->pvc != NULL) {
//    delete(this->pvc);
//  }
}

int cvStreamReader::__blokingFunctionCallback(void* p) {
  int *v = (int*)p;
  if (*v > BLOCK_FUNCTION_MAX_COUNT) {
    LOG(DEBUG, "{cvStreamReader::__blokingFunctionCallback} Abortando la funciones bloqueantes");
    return 1;
  }
  return 0;
}

void cvStreamReader::__pollFrames() {
  std::chrono::time_point<std::chrono::system_clock> start, end;
  AVFormatContext *formatCtx = NULL;
  AVCodecContext *codecCtx = NULL;
  AVCodecContext *codecCopyCtx = NULL;
  AVCodec *pCodec = NULL;
  AVFrame *pFrame = NULL;
  AVFrame *pRGBFrame = NULL;
  AVPacket packet;
  struct SwsContext *sws_ctx = NULL;
  int frameFinished;
  int res;
  int bytes;
  int videoStreamIndex = -1;
  int failCount = 0;
  uint8_t *buffer = NULL;
  cv::Mat *frame;

  res = avformat_open_input(&formatCtx, (const char *)this->url.c_str(), NULL, NULL);
  if (res != 0) {
    LOG(ERROR,"{cvStreamReader::__pollFrames} No puedo abrir el contenedor");
    return;
  }

  res = avformat_find_stream_info(formatCtx, NULL);
  if (res < 0) {
    LOG(ERROR, "{cvStreamReader::__pollFrames} No puedo leer la informacion del flujo");
    avformat_close_input(&formatCtx);    
    return;
  }
  for (unsigned int j=0 ; j<formatCtx->nb_streams ; j++) {
    if (formatCtx->streams[j]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
      videoStreamIndex = j;
      break;
    }
  }
  if (videoStreamIndex == -1) {
    LOG(ERROR, "{cvStreamReader::__pollFrames} No puedo encontrar el flujo de video");
    avformat_close_input(&formatCtx);
    return;
  }
  codecCtx = formatCtx->streams[videoStreamIndex]->codec; 
  pCodec = avcodec_find_decoder(codecCtx->codec_id);
  if (pCodec == NULL) {
    LOG(ERROR, "{cvStreamReader::__pollFrames} Codec no soportado codec_id=", codecCtx->codec_id);
    avformat_close_input(&formatCtx);
    return;
  }

  codecCopyCtx = avcodec_alloc_context3(pCodec);
  res = avcodec_copy_context(codecCopyCtx, codecCtx);
  if (res != 0) {
    LOG(ERROR, "{cvStreamReader::__pollFrames} No puedo hacer copia del contexto del codec");
    avformat_close_input(&formatCtx);
    return;
  }

  res = avcodec_open2(codecCopyCtx, pCodec, NULL);
  if (res < 0) {
    LOG(ERROR, "{cvStreamReader::__pollFrames} No puedo abrir el codec");
    avcodec_free_context(&codecCopyCtx);
    avformat_close_input(&formatCtx);
    return;
  }

  pFrame=av_frame_alloc();
  pRGBFrame=av_frame_alloc();

  bytes = avpicture_get_size(PIX_FMT_RGB24, codecCopyCtx->width,
    codecCopyCtx->height);

  LOG(INFO, "{cvStreamReader::__pollFrames} alto=", codecCopyCtx->height,
    " ancho=", codecCopyCtx->width, " bytes=", bytes);

  buffer = (uint8_t *)av_malloc(bytes*sizeof(uint8_t));
  res = avpicture_fill((AVPicture *) pRGBFrame, buffer, PIX_FMT_BGR24,
    codecCopyCtx->width, codecCopyCtx->height);

  sws_ctx = sws_getContext(
    codecCopyCtx->width,
    codecCopyCtx->height,
    codecCopyCtx->pix_fmt,
    codecCopyCtx->width,
    codecCopyCtx->height,
//    PIX_FMT_RGB24,
    PIX_FMT_BGR24, //opencv guarda como BGR
    SWS_BICUBIC,
//    SWS_BILINEAR,
    NULL, NULL, NULL
  );

  formatCtx->interrupt_callback.opaque=(void*)&this->abortBlockingCount;
  formatCtx->interrupt_callback.callback = cvStreamReader::__blokingFunctionCallback;

  start = std::chrono::system_clock::now();
  this->runFlag = 1;  
  while (this->runFlag) {
    end = std::chrono::system_clock::now();
    if (formatCtx != NULL) {
      res = av_read_frame(formatCtx, &packet);
      if (res < 0) {
        failCount++;
        LOG(ERROR, "{cvStreamReader::__pollFrames} Fallo al leer los paquetes");
      }
    }
    if (failCount >= this->abortBlockingCount) {
      LOG(INFO,"{cvStreamReader::__pollFrames} Intentando reconectar");
      if (formatCtx != NULL) {
        avformat_close_input(&formatCtx);
      }
      res = avformat_open_input(&formatCtx, (const char *)this->url.c_str(), NULL, NULL);
      if (res == 0) {
        LOG(INFO, "{cvStreamReader::__pollFrames} Reconectado");
        failCount = 0;
      } else {
        LOG(ERROR, "{cvStreamReader::__pollFrames} Fallo al reconectar, pausando por ", RECONNECT_PAUSE, " seg.");
        std::this_thread::sleep_for(std::chrono::seconds(RECONNECT_PAUSE));
      }
    } else {
      if (res >= 0 && packet.stream_index == videoStreamIndex) {
        avcodec_decode_video2(codecCopyCtx, pFrame, &frameFinished, &packet);
        if (frameFinished) {
          LOG(TRACE, "{cvStreamReader::__pollFrames} Frame listo");
          sws_scale(sws_ctx, (uint8_t const * const *) pFrame->data,
            pFrame->linesize, 0, codecCopyCtx->height,
            pRGBFrame->data, pRGBFrame->linesize);
          frame = new cv::Mat(codecCopyCtx->height, codecCopyCtx->width,
            CV_8UC3, pRGBFrame->data[0]);
          if (std::chrono::duration_cast<std::chrono::milliseconds>
                        (end-start).count() >= this->frameFeedPeriod) {
            start = end;
            (this->cb)(*frame, this->userPointer);
          }
          frame->release();
          delete(frame);
          //un flujo de video a 30fps tendra el siguiente en un tiempo
          //superior a 30ms, asi que me puedo dormir un ratito.
          std::this_thread::sleep_for(
            std::chrono::milliseconds(MILLISECONDS_PER_FRAME));
        }
      }
    }
    av_free_packet(&packet);
    av_init_packet(&packet);
  }
  sws_freeContext(sws_ctx);
  av_frame_free(&pFrame);
  av_frame_free(&pRGBFrame);
  av_free(buffer);
  avcodec_free_context(&codecCopyCtx);
  avcodec_close(codecCtx);
  avformat_close_input(&formatCtx);
}

void cvStreamReader::startCapture() {
  this->th = std::thread(&cvStreamReader::__pollFrames, this);
  std::this_thread::sleep_for(std::chrono::seconds(5));
  if (!this->isRunning()) {
    LOG(ERROR, "{cvStreamReader::startCapture} No puedo iniciar la captura de video");
    std::runtime_error e("No puedo iniciar la captura de video");
    throw e;
    //return;
  }
}

void cvStreamReader::stopCapture() {
  if (this->runFlag == 1) {
    this->runFlag = 0;
    this->th.join();
  }
}

int cvStreamReader::isRunning() {
  return (this->runFlag == 1);
}

void cvStreamReader::setUserPointer(void* ptr) {
  this->userPointer = ptr;
}



#include <iostream>
#include <iomanip>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
  #include <libavcodec/avcodec.h>
  #include <libavformat/avformat.h>
  #include <libswscale/swscale.h>
}
#endif


void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) {
  FILE *pFile;
  char szFilename[32];
  int  y;
  
  // Open file
  sprintf(szFilename, "frame%d.ppm", iFrame);
  pFile=fopen(szFilename, "wb");
  if(pFile==NULL)
    return;
  
  // Write header
  fprintf(pFile, "P6\n%d %d\n255\n", width, height);
  
  // Write pixel data
  for(y=0; y<height; y++)
    fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);
  
  // Close file
  fclose(pFile);
}


int callback(void* p) {
  int *v = (int*)p;
  if (*v > 2) {
    std::cout << "Abortando la funciones bloqueantes timeout=" << *v << std::endl;
    return 1;
  }
  return 0;
}

int main(int argc, char **argv) {
  AVFormatContext *formatCtx = NULL;
  AVCodecContext *codecCtx = NULL;
  AVCodecContext *codecCopyCtx = NULL;
  AVCodec *pCodec = NULL;
  AVFrame *pFrame = NULL;
  AVFrame *pRGBFrame = NULL;
  cv::Mat *pCvFrame = NULL;

  int res;
  unsigned int j;
  int videoStreamIndex = -1;

  av_register_all();
  avformat_network_init();
  int timeout = 0;
//  pv5i
//  res = avformat_open_input(&formatCtx, "rtsp://admin:&c3b4d4%@192.168.1.61", NULL, NULL);

//  pv5d
//  res = avformat_open_input(&formatCtx, "rtsp://admin:v1s1b1l1d4d@192.168.1.50", NULL, NULL);

//  prefectura
  res = avformat_open_input(&formatCtx, "rtsp://admin:c3b4d4@192.168.1.31", NULL, NULL);
  //res = avformat_open_input(&formatCtx, "salida.avi", NULL, NULL);
  if (res != 0) {
    std::cout << "No puedo abrir el contenedor" << std::endl;
    return -1;
  }

  std::cout << "aca" << std::endl;
  formatCtx->interrupt_callback.opaque=(void*)&timeout;
  formatCtx->interrupt_callback.callback = &callback;
  res = avformat_find_stream_info(formatCtx, NULL);
  if (res < 0) {
    std::cout << "No puedo leer la informacion del flujo" << std::endl;
    return -1;
  }
  //av_dump_format(formatCtx, 0, "rtsp://admin:&c3b4d4%@192.168.1.61", 0);
  for (j=0 ; j<formatCtx->nb_streams ; j++) {
    if (formatCtx->streams[j]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
      videoStreamIndex = j;
      break;
    }
  }
  if (videoStreamIndex == -1) {
    std::cout << "No puedo encontrar el flujo de video" << std::endl;
    return -1;
  }
  codecCtx = formatCtx->streams[videoStreamIndex]->codec; 
  pCodec = avcodec_find_decoder(codecCtx->codec_id);
  if (pCodec == NULL) {
    std::cout << "Codec no soportado codec_id=" << codecCtx->codec_id << std::endl;
    av_free(formatCtx);
    return -1;
  }

  codecCopyCtx = avcodec_alloc_context3(NULL);
  res = avcodec_copy_context(codecCopyCtx, codecCtx);
  if (res != 0) {
    std::cout << "No puedo hacer copia del contexto del codec" << std::endl;
    av_free(formatCtx);
    av_free(pCodec);
    return -1;

  }

  res = avcodec_open2(codecCopyCtx, pCodec, NULL);
  if (res < 0) {
    std::cout << "No puedo abrir el codec" << std::endl;
    av_free(formatCtx);
    av_free(pCodec);
    av_free(codecCopyCtx);
    return -1;
  }

  pFrame=av_frame_alloc();
  pRGBFrame=av_frame_alloc();

  uint8_t *buffer = NULL;
  int bytes;

  bytes = avpicture_get_size(PIX_FMT_RGB24, codecCopyCtx->width,
    codecCopyCtx->height);

  std::cout << "codecCopyCtx->height = " << codecCopyCtx->height << " codecCopyCtx->width= " << codecCopyCtx->width << std::endl;
  std::cout << "bytes = " << bytes << std::endl;

  buffer = (uint8_t *)av_malloc(bytes*sizeof(uint8_t));

  res = avpicture_fill((AVPicture *) pRGBFrame, buffer, PIX_FMT_BGR24,
    codecCopyCtx->width, codecCopyCtx->height);

  std::cout << "avpicture_fill return value=" << res << std::endl;

  struct SwsContext *sws_ctx = NULL;
  int frameFinished;
  AVPacket packet;

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

  j = 0;
  int s = 50*60;
  std::chrono::system_clock::time_point start; 
  std::chrono::system_clock::time_point current; 
//  char filename[40];

  start = std::chrono::system_clock::now();
  std::cout << "Start" << std::endl;

  while (true) {
    if (timeout < 4) {
      res = av_read_frame(formatCtx, &packet);
      if (res < 0) {
        std::cout << "Fallo al leer el frame" << std::endl;
        timeout++;
      }
    }
    if (timeout >= 4) {
      std::cout << "Intentando reconectar" << std::endl;
  //    printf("%x\n", formatCtx);
      if (formatCtx != NULL) {
        avformat_close_input(&formatCtx);
      }
 //     printf("%x<---\n", formatCtx);
      res = avformat_open_input(&formatCtx, "rtsp://admin:v1s1b1l1d4d@192.168.1.50", NULL, NULL);
      if (res == 0) {
        std::cout << "Reconectado" << std::endl;
        timeout = 0;
      } else {
        std::cout << "Fallo al reconectar, me duermo por 5 seg" << std::endl;
        usleep(5000000);
      }
    } else {
      if (res >=0 && packet.stream_index == videoStreamIndex) {
        //el paquete es leido del stream de video
        //decodificamos
  //      pFrame=av_frame_alloc();
        avcodec_decode_video2(codecCopyCtx, pFrame,
          &frameFinished, &packet);
        if (frameFinished) {
          j++;
          sws_scale(sws_ctx, 
            pFrame->data,
            pFrame->linesize, 0, codecCopyCtx->height,
            pRGBFrame->data, pRGBFrame->linesize);
            pCvFrame = new cv::Mat(codecCopyCtx->height, codecCopyCtx->width, CV_8UC3, pRGBFrame->data[0]);
        /*    cv::imshow("test", *pCvFrame);
            cv::waitKey(10);*/
            pCvFrame->release();
            delete(pCvFrame);
        }
//        av_frame_free(&pFrame);
      }
    }
    av_free_packet(&packet);
    av_init_packet(&packet);
    current = std::chrono::system_clock::now();
    if ((std::chrono::duration_cast<std::chrono::seconds>
        (current-start)).count() >= s ) {
      break;
    }
  }

  std::cout << "Frames leidos " << j << " en " << s << std::endl;
  std::cout << std::setprecision(5) << (double)j/s << " fps" << std::endl;

  sws_freeContext(sws_ctx);

  av_frame_free(&pRGBFrame);
  //if (pFrame != NULL) {
    av_frame_free(&pFrame);
  //}
  av_free(buffer);

  avcodec_free_context(&codecCopyCtx);


  avcodec_close(codecCtx);

  avformat_close_input(&formatCtx);
  return 0;
}

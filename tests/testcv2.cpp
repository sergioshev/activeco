#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <exception>
#include <jpeglib.h>
#include "videoStreamReader.h"

int size_raw;
unsigned char* raw_image;

int read_jpeg_file( void *buffer, int size  )
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    int width, height;

    /* libjpeg data structure for storing one row, that is, scanline of an image */
    JSAMPROW row_pointer[1];

    FILE *infile =  fmemopen(buffer,size,"rb");
    unsigned long location = 0;
    int i = 0;

    if ( !infile )
    {
        printf("Error opening jpeg file %s\n!", "asdfas" );
        return -1;
    }

    /* here we set up the standard libjpeg error handler */
    cinfo.err = jpeg_std_error( &jerr );
    /* setup decompression process and source, then read JPEG header */
    jpeg_create_decompress( &cinfo );
    /* this makes the library read from infile */
    jpeg_stdio_src( &cinfo, infile );
    /* reading the image header which contains image information */
    jpeg_read_header( &cinfo, TRUE );
    /* Uncomment the following to output image information, if needed. */
    
    printf( "JPEG File Information: \n" );
    printf( "Image width and height: %d pixels and %d pixels.\n", cinfo.image_width, cinfo.image_height );
    printf( "Color components per pixel: %d.\n", cinfo.num_components );
    printf( "Color space: %d.\n", cinfo.jpeg_color_space );
   
    /* Start decompression jpeg here */

    jpeg_start_decompress( &cinfo );

    width=cinfo.image_width;
    height= cinfo.image_height;
    size_raw=cinfo.output_width*cinfo.output_height*cinfo.num_components ;

    /* allocate memory to hold the uncompressed image */
    raw_image = (unsigned char*)malloc( cinfo.output_width*cinfo.output_height*cinfo.num_components );
    /* now actually read the jpeg into the raw buffer */
    row_pointer[0] = (unsigned char *)malloc( cinfo.output_width*cinfo.num_components );
    /* read one scan line at a time */
    while( cinfo.output_scanline < cinfo.image_height )
    {
        jpeg_read_scanlines( &cinfo, row_pointer, 1 );
        for( i=0; i<cinfo.image_width*cinfo.num_components;i++)
            raw_image[location++] = row_pointer[0][i];
    }
    /* wrap up decompression, destroy objects, free pointers and close open files */

    jpeg_finish_decompress( &cinfo );
    jpeg_destroy_decompress( &cinfo );
    free( row_pointer[0] );
    fclose( infile );
    /* yup, we succeeded! */
    return 1;
}


void windowUpdater(cv::Mat frame, void* ptr) {
  //cv::imshow("TEST", frame);
  uchar* buffer;
  double size;

  if (ptr != NULL) {
    ((cFrameDispatcher<cFrameObserver>*)ptr)->dispatch(frame);
    if (frame.cols != -1 && frame.rows != -1) {
/*      size = frame.cols * frame.rows * sizeof(uchar);
      buffer = (uchar*)malloc(size);
      for (int i = 0 ; i < frame.rows ; i++) {
        for (int j = 0 ; j < frame.cols ; j++ ) {
          buffer[i*frame.cols+j] = frame.at<uchar>(i,j);
        } 
      }
      read_jpeg_file(buffer, size);
      if (buffer) {
        free(buffer);
      }*/
    }
  }
}

int main( int argc, char** argv ) {
  // barrera pv5
  //const std::string videoStreamAddress = "rtsp://admin:1m4g1n4c10n@192.168.1.51/axis-cgi/mjpg/video.cgi";
  // salida calado
  //const std::string videoStreamAddress = "rtsp://admin:c3b4d4@192.168.1.32";
  // bruto
  const std::string videoStreamAddress = "rtsp://admin:c3b4d4@192.168.1.45";
  cChronoObserver chronoObserver(3);
  cFrameDispatcher<cFrameObserver> frameDispatcher;
  cvStreamReader sr(videoStreamAddress, windowUpdater);
 
  sr.setUserPointer(&frameDispatcher);
  frameDispatcher.addObserver(&chronoObserver);
  cv::namedWindow("TEST", CV_WINDOW_AUTOSIZE);
  try {
    sr.startCapture();
  } catch (std::exception e) {
    std::cerr << "No puedo arrancar la lectura" << e.what() << std::endl;
    return 1;
  }
  while(sr.isRunning()) {
    if (cv::waitKey(30) >=0) {
      sr.stopCapture();
    }
  }
  std::cout << "Se detuvo el stream" << std::endl;
}

 


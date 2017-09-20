#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>

#include <iostream>
#include <string.h>
#include <stdlib.h>

#include "libfakevpar.h"

extern "C" {

tClbkVpar vparCallback;
bool trace;


long vpmrInit(
  long lWidth,
  long lHeight,
  unsigned char * pImageData,
  bool bFlip) {
  std::cout << "fakevpmr: init" << std::endl;
  return 1;
}

long vpmrReadRGB24(long lWidth, long lHeight, unsigned char * pbImageData,
  bool bFlip) {
  std::ifstream infile("fakeplate.txt");
  std::string line;
  
  if (std::getline(infile, line)) {
    std::istringstream iss(line);
    iss >> confidenceFromFile >> plateFromFile;
    confidenceGenerated = rand() % 100 + 1;
    std::cout << "fakevpmr:readRGB exito generado=" << confidenceGenerated << " exito del archivo=" << confidenceFromFile << " patente="<< plateFromFile << std::endl;
    return 1;
  }
  return 0;
}

long vpmrGetNumberOfPlates(void) {
  if (confidenceGenerated < confidenceFromFile) {
    return 1;
  }
  return 0;
}

long vpmrGetText(char * strResult, long lPlate) {
  if (confidenceGenerated < confidenceFromFile) {
    strResult = new char[plateFromFile.length()+1];
    strcpy(strResult, plateFromFile.c_str());
    std::cout << "fakevpmr:getText patente establecida=" << plateFromFile << std::endl;
  }
  return 1;
}

float vpmrGetGlobalConfidence(long lPlate) {
  globalConfidence = 0.31;
  if (confidenceGenerated < confidenceFromFile) {
    globalConfidence = 0.99;
  }
  std::cout << "fakevpmr:getGlobalConfidence confidence=" << globalConfidence << std::endl;
  return globalConfidence;
}

void vpmrEnd(void) {
  std::cout << "fakevpmr:end termiando el motor" << std::endl;
  return;
}

//null no plate, string - plate
char* recognizePlate(char *strFilename) {
  std::ifstream infile(strFilename);
  std::string line;

  int successCoef;
  std::string plate;
  
  if (std::getline(infile, line)) {
    std::istringstream iss(line);
    iss >> successCoef >> plate;
    int r = rand()  % 100 + 1;
    if (trace) {
      std::cout << "fakeVpar:recognizePlate: exito generado=" << r << " exito definido=" << successCoef << std::endl;
    }
    if (r<=successCoef) { //success recognition
      char *pplate = new char[plate.length()+1];
      strcpy(pplate, plate.c_str());
      //no olvidar hacer delete [] pplate;
      if (trace) {
        std::cout << "fakeVpar:recognizePlate: retornando patente=" << plate << std::endl;
      }
      return pplate;
    } 
  }
  return NULL;
}

long  vparmtInit(
  tClbkVpar callback, 
  long lCountryCode,
  long lAvCharacterHeight, 
  bool bDuplicateLines, 
  bool bReserved1, 
  long lReserved2,
  bool bTrace)
{
  trace = bTrace;
  vparCallback = callback;
  if (trace) {
    std::cout << "fakeVpar:vparmtInit: callback seteado" << std::endl;
  }
  srand(time(NULL));
  return 1;
}

void vparmtEnd(void) {
  if (trace) {
    std::cout << "fakeVpar:vparmtEnd: saliendo" << std::endl;
  }
}

long vparmtReadJPG(
  tRecognitionEngineSettings *settings,
  char* strFilename) {

  char *plate;
  tResults *r;
  char rplate[MAX_CHAR];
  int i=0;

  int q = rand() % 1000;
  
  if ((plate=recognizePlate(strFilename)) != NULL) {
    r=(tResults*)malloc(sizeof(tResults));
    while (i<MAX_CHAR && plate[i] != '\0') {
      r->strResult[0][i]=plate[i];
      i++;
    }
    r->lres = 1;
    r->lNumberOfPlates = 1;
    r->vlNumberOfCharacters[0]=6;
    r->vlGlobalConfidence[0]=0.95;
    (vparCallback)(q,r);
    free(r);
  }
  return 0;
}

long vparmtReadJPG_sync(
  tRecognitionEngineSettings *settings,
  char *strFilename,
  tResults *results) {

  char *plate;
  int i=0;

  results->lres = 0;
  results->lNumberOfPlates = 0;

  if ((plate=recognizePlate(strFilename)) != NULL) {
    while (i<MAX_CHAR && plate[i] != '\0') {
      results->strResult[0][i]=plate[i];
      i++;
    }
    results->lres = 1;
    results->lNumberOfPlates = 1;
    results->vlNumberOfCharacters[0]=6;
    results->vlGlobalConfidence[0]=0.95;
  }

  return 1;
}

long vparmtReadBMP(
  tRecognitionEngineSettings *settings,
  char *strFilename) {

  char *plate;
  tResults *r;
  int i=0;

  int q = rand() % 1000;
  
  if ((plate=recognizePlate(strFilename)) != NULL) {
    r=(tResults*)malloc(sizeof(tResults));
    while (i<MAX_CHAR && plate[i] != '\0') {
      r->strResult[0][i]=plate[i];
      i++;
    }
    r->lres = 1;
    r->lNumberOfPlates = 1;
    r->vlNumberOfCharacters[0]=6;
    r->vlGlobalConfidence[0]=0.95;
    (vparCallback)(q,r);
    free(r);
  }
 
  return q;
}

long vparmtReadBMP_sync(
  tRecognitionEngineSettings *settings,
  char *strFilename,
  tResults *results) {

  char *plate;
  int i=0;
  
  results->lres = 0;
  results->lNumberOfPlates = 0;


  if ((plate=recognizePlate(strFilename)) != NULL) {
    while (i<MAX_CHAR && plate[i] != '\0') {
      results->strResult[0][i]=plate[i];
      i++;
    }
    results->lres = 1;
    results->lNumberOfPlates = 1;
    results->vlNumberOfCharacters[0]=6;
    results->vlGlobalConfidence[0]=0.95;
  }

  return 1;
}

}

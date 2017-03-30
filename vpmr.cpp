#include <exception>
#include <iostream>
#include <dlfcn.h>
#include <string.h>
#include "vpmr.h"

clibVpar::clibVpar() {
  vparInitException initError;

  this->pathToLib = PATH_TO_LIB;
  if ((this->pvpmr=dlopen(this->pathToLib, RTLD_LAZY))==NULL) {
    std::cerr << dlerror() << std::endl;
    throw initError;
  }
  if ((this->pVPMRInit=(fpvpmrInit)dlsym(this->pvpmr,"vpmrInit"))==NULL) {
    std::cerr << dlerror() << std::endl;
    throw initError;
  }
  if ((this->pVPMREnd=(fpvpmrEnd)dlsym(this->pvpmr,"vpmrEnd"))==NULL) {
    std::cerr << dlerror() << std::endl;
    throw initError;
  }
  if ((this->pVPMRReadBMP=(fpvpmrReadBMP)dlsym(this->pvpmr,"vpmrReadBMP"))==NULL) {
    std::cerr << dlerror() << std::endl;
    throw initError;
  }
  if ((this->pVPMRReadJPG=(fpvpmrReadJPG)dlsym(this->pvpmr,"vpmrReadJPG"))==NULL) {
    std::cerr << dlerror() << std::endl;
    throw initError;
  }
  if ((this->pVPMRReadRGB24=(fpvpmrReadRGB24)dlsym(this->pvpmr,"vpmrReadRGB24"))==NULL) {
    std::cerr << dlerror() << std::endl;
    throw initError;
  }

  if ((this->pVPMRGetNumberOfPlates=(fpvpmrGetNumberOfPlates)dlsym(this->pvpmr,"vpmrGetNumberOfPlates"))==NULL) {
    std::cerr << dlerror() << std::endl;
    throw initError;
  }

  if ((this->pVPMRGetText=(fpvpmrGetText)dlsym(this->pvpmr,"vpmrGetText"))==NULL) {
    std::cerr << dlerror() << std::endl;
    throw initError;
  }
  if ((this->pVPMRGetGlobalConfidence=(fpvpmrGetGlobalConfidence)
         dlsym(this->pvpmr,"vpmrGetGlobalConfidence"))==NULL) {
    std::cerr << dlerror() << std::endl;
    throw initError;
  }
  if ((this->pVPMRGetProcessingTime=(fpvpmrGetProcessingTime)
         dlsym(this->pvpmr,"vpmrGetProcessingTime"))==NULL) {
    std::cerr << dlerror() << std::endl;
    throw initError;
  }
  if ((this->pVPMRSetTimeOut=(fpvpmrSetTimeOut)
         dlsym(this->pvpmr,"vpmrSetTimeOut"))==NULL) {
    std::cerr << dlerror() << std::endl;
    throw initError;
  }
  this->pathToMap = new char[sizeof(PATH_TO_PACK)];
  strcpy(this->pathToMap,PATH_TO_PACK);
}

clibVpar::~clibVpar() {
  delete[] this->pathToMap;
  dlclose(this->pvpmr);
}

long clibVpar::vpmrInit(
  long lCountryCode, 
  long lAvCharacterHeight,
  bool bDuplicateLines,
  bool bReserved1,
  long lReserved2,
  bool bTrace) {

  long res = (this->pVPMRInit)(this->pathToMap, lCountryCode, lAvCharacterHeight,
               bDuplicateLines, bReserved1, lReserved2, bTrace);
  return res;
}

void clibVpar::vpmrEnd(void) {
  (this->pVPMREnd)();
}

long clibVpar::vpmrReadJPG(char* filename) {
  long res = (this->pVPMRReadJPG)(filename);
  return res;
}

long clibVpar::vpmrReadRGB24(long lWidth, long lHeight, unsigned char* pbImageData, bool bFlip) {
  long res = (this->pVPMRReadRGB24)(lWidth, lHeight, pbImageData, bFlip);
  return res;
}
/*
long clibVpar::vpmrReadRGB24_cb(long lWidth, long lHeight,
 unsigned char* pbImageData,
 bool bFlip,
 tClbkVpar cb) {
  int j;
  long numberOfPlates;
  float maxConfidence = 0;
  tResults result;

  long res = (this->pVPMRReadRGB24)(lWidth, lHeight, pbImageData, bFlip);
  // Se pudo leer la imagen
  if (res) {
    numberOfPlates = this->vpmrGetNumberOfPlates();
    if (numberOfPlates <= MAX_PLATES && numberOfPlates > 0) {
      result.lres = res;
      result.lNumberOfPlates = numberOfPlates;
      for (j = 0 ; j < numberOfPlates ; j++) {
        this->vpmrGetText(result.strResult[j], j);
        result.vlGlobalConfidence[j] = this->vpmrGetGlobalConfidence(j);
        if (result.vlGlobalConfidence[j] > maxConfidence) {
          maxConfidence = result.vlGlobalConfidence[j];
        }
      }
      // al menos una patente se detecto por encima del umbral
      // llamamos el callback
      if (maxConfidence >= MIN_CONFIDENCE) {
        (cb)(0, &result);
      }
    }
  }
  return res;
}
*/
long clibVpar::vpmrGetNumberOfPlates(void) {
  int res;
  res = this->pVPMRGetNumberOfPlates();
  return res;
}

long clibVpar::vpmrGetText(char* strResult, long iPlate) {
  long res = (this->pVPMRGetText)(strResult, iPlate);
  return res;
}

float clibVpar::vpmrGetGlobalConfidence(long iPlate) {
  float res = (this->pVPMRGetGlobalConfidence)(iPlate);
  return res;
}


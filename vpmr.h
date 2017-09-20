#ifndef VPMR_H
#define VPMR_H
#include <exception>


#define PATH_TO_PACK "/usr/lib/map"
//#define PATH_TO_LIB "/usr/lib/libvpmr.so"
#define PATH_TO_LIB "/usr/lib/libfakevpar.so"

/*
  maximo de patentes a detectar
  maximo largo de patente
*/

// maximo de patentes por imagen
#define MAX_PLATES 2

//maximo de caracteres en una patente
#define MAX_CHAR 10

//tamaño maxima de la ruta de un archivo
#define MAX_FILE_PATH 300

//coeficiente de confianza minima que se
// concidera como exitosa
#define MIN_CONFIDENCE 0.8

typedef struct {
  //resultado de lectura. 1 correto. 0 incorrecto
  long lres;

  //numero de patentes leidas en la imagen
  long lNumberOfPlates;

  //arreglo con las patentes leidas
  char strResult[MAX_PLATES][MAX_CHAR];

  //arreglo con la cantidad de caracteres en cada patente
  long vlNumberOfCharacters[MAX_PLATES];

  //arreglo con el coeficiente de confianza de cada patente
  float vlGlobalConfidence[MAX_PLATES];

} tResults;

//callback para llamar asincronicamente cada vez que el motor tenga un resultado
// code - codigo de la peticion encolada, ese codigo es retornado 
//    en la funcion Read
// results - la estructura con los resultados tResults
typedef long (*tClbkVpar)(long code, tResults *results);

// Tipos de callbacks de la libreria
typedef long (*fpvpmrInit)(char* pathToMap, long lCountryCode, long lAvCharacterHeight, bool bDuplicateLines, bool bReserved1, long lReserved2, bool bTrace);
typedef void (*fpvpmrEnd)(void);
//typedef long (*fpvpmrRead)(long lWidth, long lHeight, unsigned char* pbImageData);
//typedef long (*fpvpmrReadBMP)(char* filename);
//typedef long (*fpvpmrReadJPG)(char* filename);
typedef long (*fpvpmrReadRGB24)(long lWidth, long lHeight, unsigned char* pbImageData, bool bFlip);
typedef long (*fpvpmrGetNumberOfPlates)(void);
typedef bool (*fpvpmrGetText)(char * strResult, long lPlate);
//typedef long (*fpvpmrGetNumberOfCharacters)(long lPlate);
typedef float (*fpvpmrGetGlobalConfidence)(long lPlate);
//typedef float (*fpvpmrGetCharacterConfidence)(long lIndex, long lPlate);
//typedef float (*fpvpmrGetAverageCharacterHeight)(long lPlate);
//typedef void (*fpvpmrGetRectangle)(long* plLeft, long* plTop, long* plRight, long* plBottom, long lPlate);
//typedef long (*fpvpmrGetProcessingTime)();
//typedef void (*fpvpmrSetTimeOut)(long lMilliseconds);

class vparInitException: public std::exception {
  virtual const char* what() const throw() {
    return "Loading vpar library error";
  }
};

class clibVpar {
  private:
    const char* pathToLib;
    char *pathToMap;
    void *pvpmr;
    fpvpmrInit pVPMRInit;
    fpvpmrEnd pVPMREnd;
//    fpvpmrRead pVPMRRead;
//    fpvpmrReadBMP pVPMRReadBMP;
    fpvpmrReadRGB24 pVPMRReadRGB24;
//    fpvpmrReadJPG pVPMRReadJPG;
    fpvpmrGetNumberOfPlates pVPMRGetNumberOfPlates;
    fpvpmrGetText pVPMRGetText;
//    fpvpmrGetNumberOfCharacters pVPMRGetNumberOfCharacters;
    fpvpmrGetGlobalConfidence pVPMRGetGlobalConfidence;
//    fpvpmrGetCharacterConfidence pVPMRGetCharacterConfidence;
//    fpvpmrGetAverageCharacterHeight pVPMRGetAverageCharacterHeight;
//    fpvpmrGetRectangle pVPMRGetRectangle;
//    fpvpmrGetProcessingTime pVPMRGetProcessingTime;
//    fpvpmrSetTimeOut pVPMRSetTimeOut;

  public:
    clibVpar();
    ~clibVpar();

    long vpmrInit(
      long lCountryCode, 
      long lAvCharacterHeight,
      bool bDuplicateLines,
      bool bReserved1,
      long lReserved2,
      bool bTrace);

   void vpmrEnd(void);

//   long vpmrReadJPG(char* filename);
   long vpmrGetText(char* strResult, long iPlate);
   long vpmrReadRGB24(long lWidth, long lHeight, unsigned char* pbImageData, bool bFlip);
   long vpmrGetNumberOfPlates(void);
/*   long vpmrReadRGB24_cb(long lWidth, long lHeight, unsigned char* pbImageData,
     bool bFlip, tClbkVpar cb);*/
   float vpmrGetGlobalConfidence(long iPlate);
};
#endif

#ifndef _FAKEVPAR_H_
#define _FAKEVPAR_H_

#define MAX_PLATES 2

//maximo de caracteres en una patente
#define MAX_CHAR 10

//tamaño maxima de la ruta de un archivo
#define MAX_FILE_PATH 300

extern "C" {

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

  //arreglo con el alto de caracter promedio de cada patente
  float vfAverageCharacterHeight[MAX_PLATES];

  //arreglo con la confianza de caracter de cada patente
  float vlCharacterConfidence[MAX_PLATES][MAX_CHAR];

  //arreglo con la pos. izquierda de cada patente
  long vlLeft[MAX_PLATES];
  
  //arreglo con la pos. arriba de cada patente
  long vlTop[MAX_PLATES];

  //arreglo con la pos. derecha de cada patente
  long vlRight[MAX_PLATES];

  //arreglo con la pos. abajo de cada patente
  long vlBottom[MAX_PLATES];

  //tiempo de procesamiento de la deteccion en milisegundos
  long lProcessingTime;

  //arreglo con el formato de cada patente
  long vlFormat[MAX_PLATES];
  
  //puntero de la estructura de configuracion retornada
  void *lUserParam1;
  
  //puntero de la estructura de configuracion retornada
  void *lUserParam2;

  //polaridad de cada patente
  // 1 black on white
  // 2 otros
  // Cada posicion representa una patente, patente 0 en las unidades
  //   patente 1 en la decimales ...
  long lPolarity;

  //formato de confianza de la primera patente
  float vlFormatConfidence;

  //parametro para la funcion de borrado de sombra
  void *EliminateShadow;

  //ruta con la imagen salvada en caso de que la funcion CharacterRectangle esta
  //  presente
  char strPAthCorrectedImage[MAX_FILE_PATH];

  //arreglo con la posicion de cada caracter en caso de que la funcion 
  //  CharacterRectangle esta presente
  long vlCaracterPosition[MAX_PLATES][MAX_CHAR][4];
} tResults;


//Esta estructura es la responsable de configurar el motor de reconocimiento.
//  En cada llamada esta configura el motor, de este modo se puede cambiar la
//  configuracion para diferentes situaciones.
typedef struct {
  //tiempo en milisegundos para que el motor procese una imagen. Si este valor es 0
  //  este parametro no se usa
  long milliseconds;

  //flag que indica si el motor aplicara la correcciones
  // 1 es aplicar
  // 0 no aplicar
  long bApplyCorrection;

  //distancia entre la camara y el objeto en metros
  float fDistance;

  //coef. de correccion la perspectiva vertical
  float fVerticalCoeff;

  //coef. de correccion de la perspectiva horizontal
  float fHorizontalCoeff;

  //angulo de correccion de rotacion (inclinacion)
  float fAngle;

  //coef. de correccion del coeficiente radial
  float fRadialCoeff;

  //coef. de correccion del sesgo vertical
  float fVerticalSkew;

  //coef. de correccion del sesgo horizontal
  float fHorizontalSkew;

  //especificar si el motor funciona con un rango especifico
  //  de altos de caracter
  //  0 no aplica
  //  2 aplica.
  //  Aplica se refiere a aplicar el valor de primeras posiciones de
  //  vlSteps (ver mas abajo)
  long lNumSteps;

  //arreglo con los altos de caracter. Solamente usa las primeras 2
  //  posiciones en caso de que lNumSteps es 2.
  long vlSteps[8];

  //Los sigientes parametros indican el ROI en la imagen
  // ROI = Region Of Interest

  // Coordinada de la esquina superior izquierda del area de interes
  long lLeft;
  long lTop;

  //dimenciones de  RIO en pixeles
  long lWidth;
  long lHeight;

  //valor de la escala de la imagen.
  //1 no usar la escala
  float fScale;

  //puntero de uso de usuario. Este puntero se retorna en la estructura
  //  de resultado tResults
  void *lUserParam1;
  void *lUserParam2;
/*  
  void *luserParam3;
  void *lUserParam4,*/
  
  //parametro para indicar el uso o no de la funcion "shadow killer".
  // 1 es para usar la funcion
  void *KillShadow;

  //parametro para indicar el uso o no de salvado de la imagen y de
  //  la funcion de salvado
  bool CharacterRectangle;

  long SlantDetection;
} tRecognitionEngineSettings;

//callback para llamar asincronicamente cada vez que el motor tenga un resultado
// code - codigo de la peticion encolada, ese codigo es retornado 
//    en la funcion Read
// results - la estructura con los resultados tResults
typedef long (*tClbkVpar)(long code, tResults *results);

long  vparmtInit(
  tClbkVpar callback, 
  long lCountryCode,
  long lAvCharacterHeight, 
  bool bDuplicateLines, 
  bool bReserved1, 
  long lReserved2,
  bool bTrace);

void vparmtEnd(void);

long vparmtReadJPG(
  tRecognitionEngineSettings *settings,
  char* strFilename);

long vparmtReadJPG_sync(
  tRecognitionEngineSettings *settings,
  char *strFilename,
  tResults *results);

long vparmtReadBMP(
  tRecognitionEngineSettings *settings,
  char *strFilename);

long vparmtReadBMP_sync(
  tRecognitionEngineSettings *settings,
  char *strFilename,
  tResults *results);
}
#endif

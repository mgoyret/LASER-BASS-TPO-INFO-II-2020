#ifndef GRABAR_H
#define GRABAR_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <QDialog>
#include <QFile>
#include <QTextStream>
#include <QSerialPort>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>

#define DEBUG

#define	TRUE    1
#define	FALSE   0

#define EXITO   1
#define ERROR   0

#define ON      1
#define OFF     0

#define TIMER1  1

#define SIN_NOTA    0
#define NOTA1       '1'   //aca va el numero que represente a lo que llega por puerto serie al llegar la nota de valor mas chico
#define NOTA28      '9'  // "" de valor mas grande  (estoy presuponiendo que cada nota tiene valor consecutivo)
#define TOTAL_NOTAS 52
#define NOTA_MAX    28

#define SONG_FILE_NAME "cancionGrabada.csv" //agregar el nombre que sea con el path deseado

#define TIMER_TIME 100


//////////////////////////////////   DEFINES PARA MANEJAR TRAMA MAS COMODAMENTE    //////////////////////////////////////
#define PRIMER_MITAD    240 // 240 = 11110000
#define ULTIMA_MITAD    15  // 15  = 00001111
#define INICIO_TRAMA    10  // 10  = 1010 que es el inicio de trama que esta en el primer byte
#define FIN_TRAMA       13  // 13  = 1101 que es fin de trama 101 mas el bit de paridad 1000
//#define FIN_TRAMA     5   // 5   = 0101 que es el fin de trama que esta en el segundo byte
#define INICIO_TRAMA_OK ( ( ( ((uint8_t)datos[0]) & PRIMER_MITAD ) >>4) == (uint8_t)INICIO_TRAMA )
#define FIN_TRAMA_OK    ( ( ((uint8_t)datos[1]) & ULTIMA_MITAD ) == (uint8_t)FIN_TRAMA )

#define BIT1_MITAD1 ( ( ((uint8_t)datos[0]) & PRIMER_MITAD ) >>4)
#define BIT1_MITAD2 ( ((uint8_t)datos[0]) & ULTIMA_MITAD )
#define BIT2_MITAD1 ( ( ((uint8_t)datos[1]) & PRIMER_MITAD ) >>4)
#define BIT2_MITAD2 ( ((uint8_t)datos[1]) & ULITMA_MITAD )
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/* Estructura que almacena una nota. Esta compuesta por la posicion cronologica de la nota (ctr) y la nota
    en si, que va desde 0 a 28 para los noteOnn y desde 29 a 52 para los noteOff */
typedef struct noteBuffer{
    uint64_t cntr;
    uint8_t note;
}noteBuffer;


/* Estructura que almacena una cancion. Esta compuesta por la suma total de posiciones de la cancion
    y un array de notas del tipo "noteBuffer".  La estructura noteBuffer la declaro arriba para que
    al hacer esta estructura detecte el tipo de dato, sino no funca. */
typedef struct songBuffer{
    uint64_t total_cntr;
    noteBuffer *note_st;
} songBuffer;

namespace Ui {
class Grabar;
}

class Grabar : public QDialog
{
    Q_OBJECT

public:
    explicit Grabar( QWidget *parent = nullptr );

    ~Grabar( void );

    void setPuerto( QSerialPort* );
    void inicializar( void );
    void iniciarTimer( void );
    void monitoreo( void );
    void guardarNota( void );
    uint8_t guardarCancion( void );
    uint8_t procesarNota( QByteArray );
    uint8_t tramaOk( QByteArray );
    uint8_t tramaInfo( QByteArray );

private slots:
    void on_PBrec_clicked( void );
    void on_PBfinRec_clicked( void );
    void timer_handler( void );
    void puertoSerieRcv_handler( void );


private:
    Ui::Grabar *ui;
    uint8_t     grabacion; //flag para saber cuando cortar loop de timers en cuyos handlers se realiza el proceso de grabado
    uint8_t     notaTocada;
    uint8_t     status;
    QFile       songFile;
    songBuffer  recBuf;
    QSerialPort *puerto;
    QMetaObject::Connection conection; //almacena el valor retornado por connect() para podes desconectar con disconnect()
};

#endif // GRABAR_H

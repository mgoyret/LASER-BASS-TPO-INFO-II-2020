/*
    PROBAR QSERIALPORT DE MAINWINDOW COMO PUNTERO
*/


#include "inc/grabar.h"
#include "ui_grabar.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <QMessageBox>
#include <QTimer>
#include <QDebug>

#include <QBitArray>

#define DEBUG

Grabar::Grabar(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Grabar)
{
    ui->setupUi(this);
}

Grabar::~Grabar()
{
    /* Desconecta el vinculo signal slot del puerto serie que cree en set_puerto()   */
    disconnect(conection);
    delete ui;
}


/////////////////////////     PUBLIC     //////////////////////////////////////////////////////

/**
*	\fn void inicializarMdE(void)
*	\brief Inicializa variables que haya que inicializar
*	\details Detalles
*	\author Marcos Goyret
*/
void Grabar::inicializar(void)
{
    recBuf.note_st = nullptr;
    recBuf.total_cntr = 0;
    notaTocada = SIN_NOTA;
}

/**
*	\fn         void iniciarTimer_250ms(void)
*	\brief      inicializa un timer de 250ms
*	\details    Inicia un timer que ejecuta el timer handler al terminar
*	\author     Marcos Goyret
*/
void Grabar::iniciarTimer()
{
    QTimer::singleShot(TIMER_TIME, this, SLOT(timer_handler()));
}


/**
*	\fn         void timer_250ms_handler(void)
*	\brief      handler del timer inicializado de 250ms
*	\details    ejecuta la funcion guardar nota, y restaura el valor de la nota a sin nota
*	\author     Marcos Goyret
*/
void Grabar::timer_handler( void )
{
    monitoreo(); // Ver documentacion de la funcion
    if(grabacion == ON)
    {
        /* aca tendria que limpiar la cola por si toco mas de 1 nota en 250ms, solo quedarme con la primera
            limpiarCola(); o podria hacerlo dentro de la fnc notaRecibida() */
        guardarNota();
        notaTocada = SIN_NOTA;
        iniciarTimer();
    }
}

/**
*	\fn         void monitoreo(void)
*	\brief      Envia senales midi y colores al qGuitarView
*	\details    Para que el usuario al estar grabando tambien escuche y vea lo que esta tocando
*	\author     Marcos Goyret
*/
void Grabar::monitoreo()
{
    //enviar midi signal
    //setColor()
}

/**
*	\fn         void guardarNota(void)
*	\brief      guarda la nota tocada en el archivo
*	\details    Guarda la nota actual almacenada en el array secuencial de estructuras notas y tiempos
*	\author     Marcos Goyret
*/
void Grabar::guardarNota( void )
{
    uint32_t i=0;
    noteBuffer *aux = new noteBuffer[recBuf.total_cntr + 1];
    for(i=0; i<recBuf.total_cntr; i++)
    {
        aux[i].note = recBuf.note_st[i].note;
        aux[i].cntr = i;
    }
    aux[i].note = notaTocada;
    aux[i].cntr = i;

    #ifdef DEBUG
    qDebug()<<"guardando: aux[" << i << "].note = [" << aux[i].note<<"]";
    #endif

    delete[] recBuf.note_st;
    recBuf.note_st = aux;
    recBuf.total_cntr++;
}

/**
*	\fn         void guardarCancion(void)
*	\brief      Guarda la cancion grabada, en un archivo
*	\details    Imprime la informacion contenida en la estructura del tipo songBuffer, en un archivo
*	\author     Marcos Goyret
*/
uint8_t Grabar::guardarCancion( void )
{
    uint8_t i, res = ERROR;

    #ifdef DEBUG
    qDebug()<<"total notas: " << recBuf.total_cntr;
    for(i=0; i<recBuf.total_cntr; i++)
    {
        qDebug()<<"recBuf.note_st[" << recBuf.note_st[i].cntr << "].note = [" << recBuf.note_st[i].note<<"]";
    }
    #endif

    //Pedir nombre y verificar que no exista
    songFile.setFileName(SONG_FILE_NAME);
    if(songFile.open(QFile::WriteOnly |QFile::Truncate))
    {
        QTextStream out(&songFile);
        for(i=0; i<recBuf.total_cntr; i++)
        {
            out << recBuf.note_st[i].cntr << "," << recBuf.note_st[i].note << "\n";
        }
        songFile.close();
        res = EXITO;
    }
    return res;
}


/**
*	\fn         void prosesarNota(QByteArray datos)
*	\brief      Transforma informacion del puerto serie en una representacion util
*	\details    La trama que recibo por puerto serie, la decodifico y la represento con una letra o numero, segun la nota que sea
*	\author     Marcos Goyret
*/
uint8_t Grabar::prosesarNota( QByteArray datos )
{
    uint8_t res = SIN_NOTA, nota; // nota == ultimos 4 bits de byte 1 y primeros 4 bits de byte 2

    if(tramaOk(datos))
    {
        nota = tramaInfo(datos); //relleno "nota" con lo dicho en su declaracion (comentario)

        /*
            notaTocada podra tomat valores del 0 al 52. Entre 0 y 28 corresponde a los note on
            y entre el 29 y 52 corresponde a los noteoff de las notas que no son "al aire".
            ( las notas al aire no tienen noteoff, duran un tiempo por defecto )
        */

        switch(nota)
        {
            case 1: notaTocada = 1;  // terminar este switch para lo que se recibiria en cada case y a que igualar notaTocada
            break;
            case 2: notaTocada = 2;
            break;
            //etc. Para poder continuar, nesecito saber como es la trama exacta que se recibe.

            default: notaTocada = SIN_NOTA;
        }
    }

    //Esto es temporal para experimentar
    srand(time(nullptr));
    res = rand() % 53; //random entre 0 y 53
    notaTocada = res;

    return res;
}

/**
*	\fn         void tramaOk(QByteArray dataRcv)
*	\brief      Verifica que lo recibido por puerto serie sea una nota enviada por el microprosesador
*	\details    Verifica especificamente los primeros y ultimos 4 bits de lo recibido por puerto serie
*	\author     Marcos Goyret
*/
uint8_t Grabar::tramaOk(QByteArray dataRcv)
{
    uint8_t res = ERROR;
    if( (INICIO_TRAMA_OK) && (FIN_TRAMA_OK) )
        res = EXITO;
    return res;
}

/**
*	\fn         void tramaInfo(QByteArray datos)
*	\brief      Obtiene la informacion de la nota tocada
*	\details    En el mensaje recibido por puerto serie, la info. de la nota esta en los ultimos 4 bits del primer
*               byte, y en los primeros 4 bits del segundo byte
*	\author     Marcos Goyret
*/
uint8_t Grabar::tramaInfo(QByteArray datos)
{
    uint8_t res;

    res = ( (datos[0])&ULTIMA_MITAD ) + ( (datos[1])&PRIMER_MITAD );
    return res;
}

/**
*	\fn         void set_ouerto(void)
*	\brief      Apunta mi puntero de QSerialPort, a la direccion del puerto que tenemos conectado, y conecta el slot
*	\details    La conecion del slot, sera desconectada en el destructor, para dejar la senal readyRead() libre para otras ventanas
*	\author     Marcos Goyret
*/
void Grabar::set_puerto(QSerialPort *puertoExt)
{
    puerto = puertoExt;
    conection = connect(puerto, SIGNAL(readyRead()), this, SLOT(puertoSerieRcv_handler()));
}
/////////////////////////     PRIVATE SLOTS    //////////////////////////////////////////////////////

//boton iniciar grabacion
void Grabar::on_PBrec_clicked()
{
    ui->PBrec->setEnabled(false);
    ui->PBfinRec->setEnabled(true);

    grabacion = ON;
    inicializar();
    iniciarTimer();
}

//boton finalizar grabacion
void Grabar::on_PBfinRec_clicked()
{
    QMessageBox::StandardButton opcion;
    ui->PBfinRec->setEnabled(false);
    grabacion = OFF;

    //Pregunto si descartar grabacion o guardar
    opcion = QMessageBox::question(this, "Fin de la grabacion", "guardar?");
    //si no se quiere guardar liberar memoria

    if( opcion == QMessageBox::StandardButton::Yes )
    {
        if(guardarCancion())
            QMessageBox::information(this, "Bien hecho", "Cancion guarada exitosamente");
    }else if ( (opcion == QMessageBox::StandardButton::No) || (opcion == QMessageBox::StandardButton::Escape) )
    {
        //se descarta la grabacion
        QMessageBox::information(this, "Atencion", "Grabacion descartada");
    }else
        QMessageBox::critical(this, "ERROR", "Ocurrio un error inesperado [on_PBfinRed_clicked()]");

    ui->PBrec->setEnabled(true);
}

/**
*	\fn         void puertoSerieRcv_handler()
*	\brief      Slot de la interrupcion cada vez que se emite la senal ReadyRead()
*	\details    Guarda la informacion disponible en el puerto en una variable, y llama a setColor()
*	\author     Marcos Goyret
*/
void Grabar::puertoSerieRcv_handler()
{
    #ifdef DEBUG
    qDebug() << "Datos recibidos ";
    #endif

    QByteArray datos;
    uint8_t cant = (int)puerto->bytesAvailable();
    datos.resize(cant);
    puerto->read(datos.data(), cant);
    /* prosesar data recibida y transformarla a un char o uint8_t
     * pros.nota devuelve el numero de nota 1-28 */
    prosesarNota(datos);
}

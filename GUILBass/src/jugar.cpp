#include "jugar.h"
#include "ui_jugar.h"

Jugar::Jugar(QWidget *parent, QString nombre) :
    QDialog(parent),
    ui(new Ui::Jugar)
{
    ui->setupUi(this);
    bufferSerie.clear();
    qDebug() << puertoMidi.abrirPuerto(0);
    qDebug() << puertoMidi.getNombreSalida(0) << "\n" << puertoMidi.getNombresSalidas();
    qDebug() << puertoMidi.inicializarGS();
    nombreCancion = nombre;
    nombreCancion=nombreCancion.prepend("../media/");
    qDebug()<< "el nombre es:"<<nombreCancion;
      LeerArchivo();
      int i;
      for(i=0;i<listaNota.size();i++){
          qDebug()<<"lista["<<i<<"] ="<<listaNota[i].toInt();
      }
       qDebug()<<"size de lista"<<listaNota.size();
      //cargar archivo
      puntajes.cargarDesdeArchivo();
      //arranco desde 1 xq la posicion 0 es imposible menos Tom Cruise
      i=1;
      int cuerda,nota,cant=0,duracion=0;



      while(i<listaNota.size()-1){

        cuerda =(listaNota[i].toInt()-1)/7;
        nota=(std::abs(listaNota[i].toInt())-7*cuerda-1);
       // qDebug()<<i<<"cuerda"<<cuerda<<"notaa"<<nota;
        if(listaNota[i].toInt()>0){
             int notaAux=-(listaNota[i+1+cant].toInt());
             int notaCapa=listaNota[i].toInt();
             while(notaCapa!=(notaAux) && (i+cant+1)<listaNota.size()-1){
                cant++;
                notaAux=-(listaNota[i+1+cant].toInt());
             }

            if(!cant){
               duracion=0;
            }else{
                duracion=cant;
                cant=0;
            }
            ui->graphicsView_2->agregarNota(nota,cuerda,i,duracion);
        }
        i++;
      }

      connect(ui->graphicsView_2, SIGNAL(monitoreoSignal()), this, SLOT(monitoreoPuntos()));

      //despues hay q apagarlo
      ui->graphicsView_2->startTiempo();
      ui->graphicsView->setColorNotaApagada(Qt::black);
      ui->graphicsView->setColorNotaPrendida(Qt::darkRed);
      ui->graphicsView->setColorCuerdaPrendida(Qt::darkRed);
      ui->graphicsView->setColorCuerdaApagada(Qt::black);
      ui->Puntos->setPalette(Qt::white);
      qDebug()<<"contruimos";
}


Jugar::~Jugar()
{
    delete ui;
}
void Jugar::iniciarTimer(int nota)
{
    timerNota[nota]=1;
    QTimer::singleShot(TIMER_TIME_, this, SLOT(timer_handler()));
}
void Jugar::timer_handler()
{
    int i;
    for(i=0;i<29;i++){
        if(timerNota[i]== 1)
        {
            i=-i;
            mostrarNota(i);
            ui->Puntos->setPalette(Qt::white);
            timerNota[i]=0;
        }
    }
}
void Jugar::monitoreoPuntos() {

    //esto es lo que tengo
    //cuerda =(listaNota[i].toInt()/4)-1;
    //nota= (listaNota[i].toInt()-7*cuerda)-1;
    //necesito llevarlo a lo que habia
   char nota= (ui->graphicsView_2->getCuerdaMostrar())*7 +(ui->graphicsView_2->getNroMostrar())+1;
  // char estado; //determina el estado de la nota:
                //-1 : Se paso y no se toco
                //0 : Esta en el futuro
                //1 : Ya se toco bien
                //2 : (Solo largas) se esta tocando
                //3 : Esta en un momento donde se puede tocar
                //4 : Se toco
                //5 : (Solo largas) se solto a mitad de camino
   // 0->no me importa 4->no me importa transitorio en el, 3->espero nota asi q no me importa
  // 5-> poner amarillo es medio punto -1-> rojo  1-> verde
   if(ui->graphicsView_2->getEstadoMostrar()==-1){
       //ESTO DE PALETTA NI IDEA SI ANDA O HAY Q PONERLE DE OTRA FORMA EL COLOR
       ui->Puntos->setPalette(Qt::red);
   }else if(ui->graphicsView_2->getEstadoMostrar()==1) {
       //ESTO DE PALETTA NI IDEA SI ANDA O HAY Q PONERLE DE OTRA FORMA EL COLOR
       ui->Puntos->setPalette(Qt::green);
       puntos+=PUNTOCSIMPLE;
       ui->Puntos->setText(QString::number(puntos));
       iniciarTimer(nota);
   }else if(ui->graphicsView_2->getEstadoMostrar()==2) {
       pesoPunto+=1;
       ui->Puntos->setText(QString::number(puntos));
       ui->Puntos->setPalette(Qt::blue);
       puntos=puntos+pesoPunto*(int)PUNTOCSIMPLE;
       ui->Puntos->setText(QString::number(puntos));
       iniciarTimer(nota);
   }else if(ui->graphicsView_2->getEstadoMostrar()==5) {
       ui->Puntos->setPalette(Qt::darkYellow);
       //esto lo dejo en dark yellow para poder diferenciar entre estados
       //despues se puede poner red
       pesoPunto=0;
       iniciarTimer(nota);
   }
   mostrarNota(nota);
   //muestro qguitarview y puntos
}
void Jugar::setPuerto(QSerialPort *puertoExt)
{
    puerto = puertoExt;
    conection = connect(puerto, SIGNAL(readyRead()), this,  SLOT(on_datosRecibidos() ));
}

void Jugar::mostrarNota(char nota) {

    int cuerdaYNota = notaACuerdaYNota(std::abs(nota));
    if (nota > 0) {
        if ((cuerdaYNota & 0x000000ff) != 0xff) ui->graphicsView->setNotaPrendida(cuerdaYNota & 0x000000ff);
        ui->graphicsView->setCuerdaPrendida(cuerdaYNota >> 8);
    } else {
        if ((cuerdaYNota & 0x000000ff) != 0xff) ui->graphicsView->setNotaApagada(cuerdaYNota & 0x000000ff);
        ui->graphicsView->setCuerdaApagada(cuerdaYNota >> 8);
    }

   // qDebug() << "Valor nota de mostrar (Nota/Cuerda): " << (cuerdaYNota & 0x000000ff) << "/" << (cuerdaYNota >> 8);
}
int Jugar::notaACuerdaYNota(uint8_t nota) {
    int ret = 0, cuerda = 0, notaConv = 0;
    nota--;
    notaConv = nota % 7;
    if (notaConv) {
        cuerda = nota / 7;
        notaConv = (6 - notaConv) + (6 * cuerda);
    } else {
        cuerda = nota / 7;
        notaConv = 0xff;
    }
   // qDebug() << "Cuerda: " << cuerda << "\nNotaConvertida: " << notaConv;
    ret |= notaConv;
    ret |= cuerda << 8;
    return ret;
}
void Jugar::on_datosRecibidos() {
    bufferSerie.append(puerto->readAll());
    validarDatos();
}
void Jugar::validarDatos() {
    int cant = bufferSerie.size();
    QByteArray datoAProcesar;
    datoAProcesar.clear();
    while (cant > 1) {
        if (bufferSerie[0] & 0x50) {
            if (cant == 1) break;
            datoAProcesar.append(bufferSerie.at(0));
            datoAProcesar.append(bufferSerie.at(1));
            bufferSerie.remove(0, 2);
            procesarNotaATocar(datoAProcesar);
            datoAProcesar.clear();
        } else {
            bufferSerie.remove(0, 1);
        }
        cant = bufferSerie.size();
    }
}

void Jugar::procesarNotaATocar(QByteArray dato) {
    char nota = 0;
    if (dato.size() != 2) qDebug() << "array de datos con mas de 2 bytes";
    nota |= (uint8_t)(dato.at(0) << 4) & 0xf0;
    nota |= (uint8_t)(dato.at(1) >> 4) & 0x0f;
   // qDebug()<<"la nota es:"<<(uint8_t) nota;
    if (nota < 0) {
        qDebug() << puertoMidi.enviarNoteOff(0, 32 + (uint8_t)std::abs(nota) * 2);
        nota=-nota;
        ui->graphicsView_2->soltarNota(nota/7,nota-7*(nota/7)-1);
    } else {
        qDebug() << puertoMidi.enviarNoteOn(0, 32 + (uint8_t)std::abs(nota) * 2, 127);
        ui->graphicsView_2->tocarNota(nota/7,nota-7*(nota/7)-1);
    }
}
/*void Tocar::procesarNota(QByteArray data) {
    uint8_t nota;
    if(tramaOk(data)){
        nota = TramaInfo(data);
        if((nota<1) || (nota>NOTE_MAX*2))
            NOT
    }
}*/


/**
*	\fn         void tramaOk(QByteArray datos)
*	\brief      Verifica que lo recibido por puerto serie sea una nota enviada por el microprosesador
*	\details    Verifica especificamente los primeros y ultimos 4 bits de lo recibido por puerto serie
*	\author     Marcos Goyret
*/
uint8_t Jugar::tramaOk(unsigned char* data)
{
    uint8_t res = ERROR_;

    if( INICIO_TRAMA_OK_ && FIN_TRAMA_OK_ )
        res = EXITO_;

    return res;
}

/**
*	\fn         void tramaInfo(QByteArray datos)
*	\brief      Obtiene la informacion de la nota tocada
*	\details    En el mensaje recibido por puerto serie, la info. de la nota esta en los ultimos 4 bits del primer
*               byte, y en los primeros 4 bits del segundo byte
*	\author     Marcos Goyret
*/
uint8_t Jugar::tramaInfo( unsigned char* data)
{
    uint8_t res=0;

    res = ( (((uint8_t)data[0])&ULTIMA_MITAD_)<<4 ) + ( (((uint8_t)data[1])&PRIMER_MITAD_)>>4 );

    #ifdef DEBUG
    qDebug()<< "info: " << res << " = " << (BIT1_MITAD2_<<4) << " + " << BIT2_MITAD1_;
    #endif

    return res;
}
void Jugar::LeerArchivo(void){
    int i = 0;
    QFile cancion(nombreCancion);
    if(cancion.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream in(&cancion);

        QString aux = in.readLine();
        //while (aux.size()>0)
    //while (!in.atEnd()) //La funcion !in.atEnd() no me funcionaba bien, asi que quizas haya que reemplazarla por otra
     while(!aux.isNull())
     {
        //qDebug()<<"estalapalabra"<<aux;
         listaNota +=aux.split(',');
        // qDebug()<<listaNota.size();
        //listaNota = line.split(QLatin1Char(',')); //guarda lo separado por las comas en posiciones distintas de un array
        //saco el numero de la posicion de la lista
        int tam=(int)listaNota.size();
        listaNota.swapItemsAt(tam-1,tam-2);
        listaNota.removeLast();
        i++;
        aux = in.readLine();
    }
    /*
    for(i=0;i<listaNota.size();i++){
        qDebug()<< "lista[" << i << "] =" <<listaNota[i];
    }*/
    cancion.close();
   qDebug()<<"cerramos leer archivo";
    }
}


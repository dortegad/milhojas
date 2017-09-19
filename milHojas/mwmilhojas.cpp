#include "mwmilhojas.h"
#include "ui_mwmilhojas.h"
#include "qfiledialog.h"
#include "stdio.h"
#include "iostream"

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"


MWMilHojas::MWMilHojas(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MWMilHojas)
{
    ui->setupUi(this);

    CM_PULGADA = 0.395;
    PULGADA_CM = 2.54;

    A4_LADO_CORTO_PULGADAS = 8.268;
    A4_LADO_LARGO_PULGADAS = 11.693;
}

MWMilHojas::~MWMilHojas()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------------
void MWMilHojas::showImage(QLabel *label, cv::Mat &img)
{
    //imgInFrame = img;
    cv::Mat imgMostrar;
    if (img.channels() == 1)
        cv::cvtColor(img,imgMostrar,CV_GRAY2RGB);
    else
        cv::cvtColor(img,imgMostrar,CV_BGR2RGB);
    QImage imagenQT = QImage((const unsigned char *)imgMostrar.data, imgMostrar.cols, imgMostrar.rows, imgMostrar.step, QImage::Format_RGB888);
    label->setScaledContents(true);
    label->setPixmap(QPixmap::fromImage(imagenQT));
    label->setGeometry(0,0,imagenQT.width(),imagenQT.height());
}

//------------------------------------------------------------------------------------------------------------------------------------------------
void MWMilHojas::extender(const cv::Mat &imOrigen,
                          int posfila,
                          int numFoliosPorLinea,
                          int tamPixelsPorTira,
                          cv::Mat &imExtendida)
{
    cv::Mat fila = imOrigen.row(posfila);


    //Creamos la tira de esa fila
    cv::Mat tira = cv::Mat(tamPixelsPorTira,imOrigen.cols,imOrigen.type());
    for (int i=0; i<tamPixelsPorTira; i++)
        fila.copyTo(tira.row(i));
    //Numeros la tira
    cv::Mat imgNumero(tamPixelsPorTira-2,tamPixelsPorTira,imOrigen.type(),cv::Scalar(255));
    std::stringstream sNum;
    sNum << posfila;
    cv::putText(imgNumero,
                sNum.str(),
                cv::Point(0,imgNumero.rows/2),
                cv::FONT_HERSHEY_SCRIPT_SIMPLEX,
                0.5,
                cv::Scalar(0),0.8,cv::LINE_8);

    //Añadimos marcas de corte a la tira
    tira.rowRange(tira.rows-3,tira.rows).colRange(0,10) = cv::Scalar(0);
    tira.rowRange(tira.rows-3,tira.rows).colRange(10,20) = cv::Scalar(255);
    tira.rowRange(tira.rows-3,tira.rows).colRange(20,30) = cv::Scalar(0);

    tira.rowRange(tira.rows-3,tira.rows).colRange(tira.cols-30,tira.cols-20) = cv::Scalar(0);
    tira.rowRange(tira.rows-3,tira.rows).colRange(tira.cols-20,tira.cols-10) = cv::Scalar(255);
    tira.rowRange(tira.rows-3,tira.rows).colRange(tira.cols-10,tira.cols-1) = cv::Scalar(0);

    cv::circle(imgNumero,cv::Point(imgNumero.rows/2,imgNumero.cols/2),5,cv::Scalar(0),-1);

    imgNumero.copyTo(tira.rowRange(0,imgNumero.rows).colRange(0,imgNumero.cols));
    imgNumero.copyTo(tira.rowRange(0,imgNumero.rows).colRange(tira.cols-imgNumero.cols-1,tira.cols-1));

//    cv::imshow("tira",tira);
//    cv::waitKey();

    //Creamos el conjunto de tiras para esa fila
    cv::Mat tirasDeUnaFila = cv::Mat(tira.rows*numFoliosPorLinea,imOrigen.cols,imOrigen.type());
    for (int i=0; i<numFoliosPorLinea; i++)
        tira.copyTo(tirasDeUnaFila.rowRange(i*tira.rows,(i*tira.rows)+tira.rows));

    //Añadimos marcas de señalizacion de fin de tiras de una fila
    tirasDeUnaFila.row(tirasDeUnaFila.rows-1).colRange(0,10) = cv::Scalar(255);
    tirasDeUnaFila.row(tirasDeUnaFila.rows-1).colRange(10,20) = cv::Scalar(0);
    tirasDeUnaFila.row(tirasDeUnaFila.rows-1).colRange(20,30) = cv::Scalar(255);

    tirasDeUnaFila.row(tirasDeUnaFila.rows-1).colRange(tirasDeUnaFila.cols-30,tirasDeUnaFila.cols-20) = cv::Scalar(255);
    tirasDeUnaFila.row(tirasDeUnaFila.rows-1).colRange(tirasDeUnaFila.cols-20,tirasDeUnaFila.cols-10) = cv::Scalar(0);
    tirasDeUnaFila.row(tirasDeUnaFila.rows-1).colRange(tirasDeUnaFila.cols-10,tirasDeUnaFila.cols-1) = cv::Scalar(255);

//    cv::imshow("tiras de una fila",tirasDeUnaFila);
//    cv::waitKey();

    //Añadimos a la imagen final el conjunto de tiras de la fila que estamos estendiendo
    tirasDeUnaFila.copyTo(imExtendida.rowRange(posfila*tirasDeUnaFila.rows,
                                               (posfila+1)*(tirasDeUnaFila.rows) ));

}


//------------------------------------------------------------------------------------------------------------------------------------------------
void MWMilHojas::extender(const cv::Mat &imagen,
                          cv::Mat &imExtendida,
                          std::vector <cv::Mat> &recortes)
{
    double dpisDeseados = this->ui->lEDPISResultadoTiras->text().toInt();
    double altoDeseado_cm = this->ui->lEAltoResultadoTiras->text().toInt(); // un metro
    double anchoDeseado_cm = this->ui->lEAnchoResultadoTiras->text().toInt(); // un metro

    //El tañano de la tira que sea de media pulgada
    int tamPixelsPorTira = dpisDeseados/*/2*/;

    //Suponemos que 500 folios son 5 cms
    double relaccionFoliosCentimetros = 500/5;
    double altoDeseado_folios = altoDeseado_cm * relaccionFoliosCentimetros;


    //Sabiendo qe apra tener esa altura necesitamos ese numero de folios cuantos folios tendremos
    //que hacer con cadauna de las lineas de la imagen (cuantas tiras de una misma linea se haran)
    int numFoliosPorLinea = altoDeseado_folios / imagen.rows;

    //ajuste para que cuadre exacatamente ocn el tamañao desado (corregimos el redondeo)
    int ajuste = (altoDeseado_folios-(numFoliosPorLinea*imagen.rows))/numFoliosPorLinea;
    int anchoDesado_px = (int)(anchoDeseado_cm*CM_PULGADA*dpisDeseados);
    cv::Mat imOrigen;
    cv::resize(imagen,imOrigen,cv::Size(anchoDesado_px,imagen.rows+ajuste));

    //Calculamos el tamaño total que va a tener la imagen en pixel con el nuemro de tiras que se van ha hacer
    //y el numero de pixeles de alto que tiene cada linea
    int numLineas = imOrigen.rows;
    int numLineasFinal = numLineas*numFoliosPorLinea*tamPixelsPorTira;


    imExtendida = cv::Mat(numLineasFinal,imOrigen.cols,imOrigen.type());

    for (int i=0; i<numLineas; i++)
    {
        extender(imOrigen,
                 i,
                 numFoliosPorLinea,
                 tamPixelsPorTira,
                 imExtendida);
    }

    //Calulamos cada cuanto pixels de alto debemos dar el corte para generar imagens imprimibles
    double corteDeseado_cm = this->ui->lECorteTiras->text().toInt(); // dos metros
    int corteDesado_px = corteDeseado_cm*CM_PULGADA*dpisDeseados;
    int numCortes = imExtendida.rows / corteDesado_px;
    for (int i=0; i<numCortes; i++)
    {
        recortes.push_back(imExtendida.rowRange(i*corteDesado_px,(i+1)*corteDesado_px).clone());
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------------
void MWMilHojas::on_pBGenerate_clicked()
{
    cv::Mat imOrigen = cv::imread(this->ui->lEImagenTiras->text().toStdString(),cv::IMREAD_GRAYSCALE);
    cv::Mat imExtendida;
    std::vector <cv::Mat> recortes;
    this->extender(imOrigen, imExtendida, recortes);
//    this->recortar(imExtendia,imgsrecortadas);
  //  this->showImage(this->ui->lImagen,imExtendida);
    int numRecortes = recortes.size();
    if (numRecortes == 0)
        return;
    if (!this->ui->cBDobles->isChecked())
    {
        for (int i=0; i<numRecortes; i++)
        {
            std::stringstream recorteFileName;
            recorteFileName << this->ui->lECarpetaDestino->text().toStdString() << "/" << "recorte_" << i << ".bmp";
            cv::imwrite(recorteFileName.str(),recortes[i]);
        }
    }
    else
    {
        cv::Mat imgDoble(recortes[0].rows,recortes[0].cols*2+2,recortes[0].type(),cv::Scalar(255));
        int columnas = recortes[0].cols;
        int numDoble = 0;
        while (numRecortes > 0)
        {
            recortes[--numRecortes].copyTo(imgDoble.colRange(0,columnas));
            if (numRecortes >0 )
                recortes[--numRecortes].copyTo(imgDoble.colRange(columnas+1,imgDoble.cols-1));
            std::stringstream dobleFileName;
            dobleFileName << this->ui->lECarpetaDestino->text().toStdString() << "/" << "doble_" << numDoble++ << ".bmp";
            cv::imwrite(dobleFileName.str(),imgDoble);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------------
void MWMilHojas::on_pBDirDestino_clicked()
{
    QString dirDestino = QFileDialog::getExistingDirectory (this,
                                               "Directorio destino",
                                               QString(),
                                               QFileDialog::ShowDirsOnly);

    this->ui->lECarpetaDestino->setText(dirDestino);
}

//------------------------------------------------------------------------------------------------------------------------------------------------
void MWMilHojas::on_pBImagenParaTiras_clicked()
{
    QString imagenParaTiras = QFileDialog::getOpenFileName (this,
                                               "Imagen para tiras",
                                               tr("Image Files (*.png *.jpg *.bmp)"));

    this->ui->lEImagenTiras->setText(imagenParaTiras);
}

//------------------------------------------------------------------------------------------------------------------------------------------------
void MWMilHojas::on_pBFrenteHojas_clicked()
{
    QString imagenFrenteHojas = QFileDialog::getOpenFileName (this,
                                               "Imagen frente hojas",
                                               tr("Image Files (*.png *.jpg *.bmp)"));

    this->ui->lEImagenFrenteHojas->setText(imagenFrenteHojas);
}

//------------------------------------------------------------------------------------------------------------------------------------------------
void MWMilHojas::on_pBIzquierdaHojas_clicked()
{
    QString imagenIzquierdaHojas = QFileDialog::getOpenFileName (this,
                                               "Imagen frente hojas",
                                               tr("Image Files (*.png *.jpg *.bmp)"));

    this->ui->lEImagenIzquierdaHojas->setText(imagenIzquierdaHojas);
}

//------------------------------------------------------------------------------------------------------------------------------------------------
void MWMilHojas::on_pBDerechaHojas_clicked()
{
    QString imagenDerechaHojas = QFileDialog::getOpenFileName (this,
                                               "Imagen frente hojas",
                                               tr("Image Files (*.png *.jpg *.bmp)"));

    this->ui->lEImagenDerechaHojas->setText(imagenDerechaHojas);
}

//------------------------------------------------------------------------------------------------------------------------------------------------
void MWMilHojas::on_pBTraseraHojas_clicked()
{
    QString imagenTraseraHojas = QFileDialog::getOpenFileName (this,
                                               "Imagen frente hojas",
                                               tr("Image Files (*.png *.jpg *.bmp)"));

    this->ui->lEImagenTraseraHojas->setText(imagenTraseraHojas);
}

//------------------------------------------------------------------------------------------------------------------------------------------------
double MWMilHojas::altoEnFolios()
{
    double altoDeseado_cm = this->ui->lEAltoResultadoFolios->text().toInt(); // un metro

    //Suponemos que 500 folios son 5 cms
//    double relaccionFoliosCentimetros = 500/5;
    double relaccionFoliosCentimetros = 80/1; //Relamente 78folios -> 1 cm
    double altoDeseado_folios = altoDeseado_cm * relaccionFoliosCentimetros;

    return altoDeseado_folios;
}

//------------------------------------------------------------------------------------------------------------------------------------------------
void MWMilHojas::extender_2(const cv::Mat &imagenFrente,
                            const cv::Mat &imagenTrasera,
                            const cv::Mat &imagenDerecha,
                            const cv::Mat &imagenIzquierda,
                            std::vector<cv::Mat> &recortes)
{
    double anchoDeseado_cm = this->ui->lEAnchoResultadoFolios->text().toInt();
    double largoDeseado_cm = this->ui->lELargoResultadoFolios->text().toInt();
    double dpisDeseados = this->ui->lEDPISResultadoFolios->text().toInt();

    std::cout << "ancho deseado en centimetros " << anchoDeseado_cm << std::endl;
    std::cout << "largo desado en centimetros " << largoDeseado_cm << std::endl;
    std::cout << "dpis desados " << dpisDeseados << std::endl;

    //Sabiendo que para tener esa altura necesitamos ese numero de folios cuantos folios tendremos
    //que hacer con cada una de las lineas de la imagen (cuantas tiras de una misma linea se haran)
    double altoDeseado_folios = altoEnFolios();
    int numFoliosPorLinea = altoDeseado_folios / imagenFrente.rows;

    int anchoDesado_px = (int)(anchoDeseado_cm*CM_PULGADA*dpisDeseados);

    std::cout << "alto en folios " << altoDeseado_folios << std::endl;
    std::cout << "numero de folios por linea " << numFoliosPorLinea << std::endl;

    cv::Mat imOrigen;
    cv::Mat imOrigenTrasera;
    //if (numFoliosPorLinea > 0)
    //{
    //    //ajuste para que cuadre exacatamente ocn el tamañao desado (corregimos el redondeo)
    //    int ajuste = (altoDeseado_folios-(numFoliosPorLinea*imagenFrente.rows))/numFoliosPorLinea;
    //    cv::resize(imagenFrente,imOrigen,cv::Size(anchoDesado_px,imagenFrente.rows+ajuste));
    //}
    //else
    //{
        //Tendremos que reducir la imagen para que al menos solo tenga una fila por folio
        cv::resize(imagenFrente,imOrigen,cv::Size(anchoDesado_px,altoDeseado_folios));
        cv::resize(imagenTrasera,imOrigenTrasera,cv::Size(anchoDesado_px,altoDeseado_folios));
    //}

    std::cout << "tamano final de la imagen FRENTE para partir: Filas : " << imOrigen.rows << " - Columnas : " << imOrigen.cols << std::endl;

    int numLineas = imOrigen.rows;

    int ladoCortoPx = A4_LADO_CORTO_PULGADAS*dpisDeseados;
    int ladoLargoPx = A4_LADO_LARGO_PULGADAS*dpisDeseados;
    int largoDeseado_px = (int)(largoDeseado_cm*CM_PULGADA*dpisDeseados);
    int anchoDeseado_px = (int)(anchoDeseado_cm*CM_PULGADA*dpisDeseados);

    cv::Mat imagenDerechaModificada;
    cv::resize(imagenDerecha,imagenDerechaModificada,cv::Size(largoDeseado_px,altoDeseado_folios));

    cv::Mat imagenIzquierdaModificada;
    cv::resize(imagenIzquierda,imagenIzquierdaModificada,cv::Size(largoDeseado_px,altoDeseado_folios));

    std::cout << "tamano final de la imagen DERECHA para partir: Filas : " << imagenDerechaModificada.rows << " - Columnas : " << imagenDerechaModificada.cols << std::endl;
    std::cout << "tamano final de la imagen IZQUIERDA para partir: Filas : " << imagenIzquierdaModificada.rows << " - Columnas : " << imagenIzquierdaModificada.cols << std::endl;


    int tamMarca = 100;

    std::vector <cv::Point> poli;
    poli.push_back(cv::Point(0,0));
    poli.push_back(cv::Point(0,largoDeseado_px-1));
    poli.push_back(cv::Point((imOrigen.cols/2)-1,(largoDeseado_px/2)-1));
    poli.push_back(cv::Point(imOrigen.cols-1,largoDeseado_px-1));
    poli.push_back(cv::Point(imOrigen.cols-1,0));
    std::vector <std::vector <cv::Point > > polisFrente;
    polisFrente.push_back(poli);

    poli.clear();
    poli.push_back(cv::Point(0,0));
    poli.push_back(cv::Point(0,imagenDerechaModificada.cols-1));
    poli.push_back(cv::Point(anchoDeseado_px-1,imagenDerechaModificada.cols-1));
    poli.push_back(cv::Point((anchoDeseado_px/2)-1,
                             (imagenDerechaModificada.cols/2)-1));
    poli.push_back(cv::Point(anchoDeseado_px-1,0));
    std::vector <std::vector <cv::Point > > polisDerecha;
    polisDerecha.push_back(poli);

    poli.clear();
    poli.push_back(cv::Point(0,0));
    poli.push_back(cv::Point(anchoDeseado_px-1,0));
    poli.push_back(cv::Point(anchoDeseado_px-1,largoDeseado_px-1));
    poli.push_back(cv::Point(0,largoDeseado_px-1));
    poli.push_back(cv::Point((anchoDeseado_px/2)-1,
                             (largoDeseado_px/2)-1));
    std::vector <std::vector <cv::Point > > polisIzquierda;
    polisIzquierda.push_back(poli);

    poli.clear();
    poli.push_back(cv::Point(0,0));
    poli.push_back(cv::Point(0,largoDeseado_px-1));
    poli.push_back(cv::Point(anchoDeseado_px-1,largoDeseado_px-1));
    poli.push_back(cv::Point(anchoDeseado_px-1,0));
    poli.push_back(cv::Point((anchoDeseado_px/2)-1,
                             (largoDeseado_px/2)-1));
    std::vector <std::vector <cv::Point > > polisTrasera;
    polisTrasera.push_back(poli);

    for (int i=0; i<numLineas; i++)
    {
        //Frente
        cv::Mat_<uchar> imgFolioFrente = imOrigen.row(i).clone();
        cv::copyMakeBorder(imgFolioFrente,imgFolioFrente,0,largoDeseado_px-1,0,0,cv::BORDER_REPLICATE);
        cv::fillPoly(imgFolioFrente,polisFrente,255);
        //cv::imshow("prueba",imgFolioFrente);
        //cv::waitKey();

        //Derecha
        cv::Mat_<uchar> imgFolioDerecha = imagenDerechaModificada.row(i).clone();
        imgFolioDerecha = imgFolioDerecha.reshape(0,imagenDerechaModificada.cols);
        cv::copyMakeBorder(imgFolioDerecha,imgFolioDerecha,0,0,0,imOrigen.cols-1,cv::BORDER_REPLICATE);
        cv::fillPoly(imgFolioDerecha,polisDerecha,255);
        //cv::imshow("prueba",imgFolioDerecha);
        //cv::waitKey();

        //Izquierda
        cv::Mat_<uchar> imgFolioIzquierda = imagenIzquierdaModificada.row(i).clone();
        imgFolioIzquierda = imgFolioIzquierda.reshape(0,imagenIzquierdaModificada.cols);
        cv::copyMakeBorder(imgFolioIzquierda,imgFolioIzquierda,0,0,imOrigen.cols-1,0,cv::BORDER_REPLICATE);
        cv::fillPoly(imgFolioIzquierda,polisIzquierda,255);
        //cv::imshow("prueba",imgFolioIzquierda);
        //cv::waitKey();

        //Trasera
        cv::Mat_<uchar> imgFolioTrasera = imOrigenTrasera.row(i).clone();
        cv::copyMakeBorder(imgFolioTrasera,imgFolioTrasera,largoDeseado_px-1,0,0,0,cv::BORDER_REPLICATE);
        cv::fillPoly(imgFolioTrasera,polisTrasera,255);
        //cv::imshow("prueba",imgFolioTrasera);
        //cv::waitKey();


        cv::Mat imgFolio = imgFolioFrente.clone();
        imgFolio = imgFolioFrente & imgFolioDerecha & imgFolioIzquierda & imgFolioTrasera;

        cv::copyMakeBorder(imgFolio,
                           imgFolio,
                           tamMarca,
                           tamMarca,
                           tamMarca,
                           tamMarca,
                           cv::BORDER_REPLICATE);

        int colMarca_01 = tamMarca + (ladoLargoPx-imgFolio.cols)/2;
        int rowMarca_01 = tamMarca + (ladoCortoPx-imgFolio.rows)/2;

        cv::copyMakeBorder(imgFolio,
                           imgFolio,
                           (ladoCortoPx-imgFolio.rows)/2,
                           (ladoCortoPx-imgFolio.rows)/2,
                           (ladoLargoPx-imgFolio.cols)/2,
                           (ladoLargoPx-imgFolio.cols)/2,
                           cv::BORDER_CONSTANT,
                           cv::Scalar(255));

        int colMarca_02 = imgFolio.cols - colMarca_01;
        int rowMarca_02 = imgFolio.rows - rowMarca_01;

        imgFolio.colRange(colMarca_01-1,colMarca_01+1).rowRange(0,tamMarca) = 0;
        imgFolio.colRange(colMarca_01-1,colMarca_01+1).rowRange(imgFolio.rows-tamMarca,imgFolio.rows) = 0;
        imgFolio.colRange(colMarca_02-1,colMarca_02+1).rowRange(0,tamMarca) = 0;
        imgFolio.colRange(colMarca_02-1,colMarca_02+1).rowRange(imgFolio.rows-tamMarca,imgFolio.rows) = 0;

        imgFolio.rowRange(rowMarca_01-1,rowMarca_01+1).colRange(0,tamMarca)  = 0;
        imgFolio.rowRange(rowMarca_01-1,rowMarca_01+1).colRange(imgFolio.cols-tamMarca,imgFolio.cols)  = 0;
        imgFolio.rowRange(rowMarca_02-1,rowMarca_02+1).colRange(0,tamMarca)  = 0;
        imgFolio.rowRange(rowMarca_02-1,rowMarca_02+1).colRange(imgFolio.cols-tamMarca,imgFolio.cols)  = 0;

        //recortes.push_back(imgFolio);

        //cv::imshow("prueba",imgFolio);
        //cv::waitKey();
        std::stringstream recorteFileName;
        recorteFileName << this->ui->lECarpetaDestino->text().toStdString() << "\\" << "folio" << i << ".bmp";
        cv::imwrite(recorteFileName.str(),imgFolio);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------------
void MWMilHojas::on_pBGenerateHojas_clicked()
{
    cv::Mat imagenFrente = cv::imread(this->ui->lEImagenFrenteHojas->text().toStdString(),cv::IMREAD_GRAYSCALE);
    cv::Mat imagenDerecha = cv::imread(this->ui->lEImagenDerechaHojas->text().toStdString(),cv::IMREAD_GRAYSCALE);
    cv::Mat imagenIzquierda = cv::imread(this->ui->lEImagenIzquierdaHojas->text().toStdString(),cv::IMREAD_GRAYSCALE);
    cv::Mat imagenTrasera = cv::imread(this->ui->lEImagenTraseraHojas->text().toStdString(),cv::IMREAD_GRAYSCALE);

    //cv::imshow("FRENTE",imagenFrente);
    //cv::imshow("DERECHA",imagenDerecha);
    //cv::imshow("IZQUIERDA",imagenIzquierda);
    //cv::imshow("TRASERA",imagenTrasera);
    //cv::waitKey();

    std::vector <cv::Mat> recortes;
    this->extender_2(imagenFrente,
                     imagenTrasera,
                     imagenDerecha,
                     imagenIzquierda,
                     recortes);
    /*
    int numRecortes = recortes.size();
    if (numRecortes == 0)
        return;

    for (int i=0; i<numRecortes; i++)
    {
        std::stringstream recorteFileName;
        recorteFileName << this->ui->lECarpetaDestino->text().toStdString() << "\\" << "folio" << i << ".bmp";
        cv::imwrite(recorteFileName.str(),recortes[i]);
    }
    */
}



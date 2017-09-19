#ifndef MWMILHOJAS_H
#define MWMILHOJAS_H

#include <QMainWindow>
#include <QLabel>
#include <opencv2/core/core.hpp>

namespace Ui {
class MWMilHojas;
}

class MWMilHojas : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MWMilHojas(QWidget *parent = 0);
    ~MWMilHojas();
    
private slots:
    void on_pBGenerate_clicked();

    void on_pBDirDestino_clicked();

    void on_pBImagenParaTiras_clicked();

    void on_pBFrenteHojas_clicked();

    void on_pBIzquierdaHojas_clicked();

    void on_pBDerechaHojas_clicked();

    void on_pBGenerateHojas_clicked();

    void on_pBTraseraHojas_clicked();

private:
    double CM_PULGADA;
    double PULGADA_CM;

    double A4_LADO_CORTO_PULGADAS;
    double A4_LADO_LARGO_PULGADAS;

    Ui::MWMilHojas *ui;
    void showImage(QLabel *label, cv::Mat &img);
    void extender(const cv::Mat &imOrigen,
                    int posfila,
                    int numFoliosPorLinea,
                    int tamPixelsPorTira,
                    cv::Mat &imExtendida);
    void extender(const cv::Mat &imagen,
                  cv::Mat &imExtendida,
                  std::vector<cv::Mat> &recortes);

    void extender_2(const cv::Mat &imagenFrente,
                    const cv::Mat &imagenTrasera,
                    const cv::Mat &imagenDerecha,
                    const cv::Mat &imagenIzquierda,
                    std::vector<cv::Mat> &recortes);

    double altoEnFolios();
};

#endif // MWMILHOJAS_H

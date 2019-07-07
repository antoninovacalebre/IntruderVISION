#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "C:\Users\newsi\Downloads\SmtpClient-for-Qt-1.1\src\SmtpMime"

#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

#include <QTimer>
#include <QMainWindow>
#include <QString>
#include <QList>
#include <QListWidgetItem>
#include <QSsl>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public slots:
    //Durante l'acquisizione viene chiamato ad ogni frame
    //per gestire la schermata e le sue funzioni
    void updateGUI();

    //Usato ad ogni frame nella schermata di calibrazione per
    //gestire il funzionamento
    void calibrationHandler();

    //Usata durante l'acquisizione per calcolare gli FPS
    //di acquisizione
    void calculateFPS();

public:
    //Costruttore
    explicit MainWindow(QWidget *parent = 0);

    //Distruttore
    ~MainWindow();

private slots:
    //Menu principale, pulsante di inizio monitoraggio premuto
    void on_pb_openAcq_clicked();

    //Schermata di monitoraggio, pulsante per tornare al menu premuto
    void on_pb_backToMenu_clicked();

    //Schermata delle impostazioni, pulsante per tornare al menu premuto
    void on_pb_backToMenu2_clicked();

    //Menu principale, pulsante delle impostazioni premuto
    void on_pb_openSettings_clicked();

    //Menu principale, pulsante della calibrazione premuto
    void on_pb_openCalibration_clicked();

    //Schermata di calibrazione, pulsante per avviare la calibrazione premuto
    void on_pb_GetCalibrationImage_clicked();

    //Schermata di calibrazione, pulsante per tornare al menu premuto
    void on_pb_backToMenu3_clicked();

    //Schermata delle impostazioni, toggle della checkbox per l'attivazione del
    //salvataggio di filmati e foto
    void on_cb_SaveMedia_toggled(bool checked);

    //Schermata delle impostazioni, pulsante per la scelta della cartella
    //di salvataggio premuto
    void on_pb_chooseFile_clicked();

    //Schermata di monitoraggio, doppio click su un elemento della lista delle
    //intrusioni
    void on_listOut_intrusions_itemDoubleClicked(QListWidgetItem *item);

    //Schermata delle impostazioni, pulsante per la conferma delle impostazioni
    //premuto
    void on_pb_confirm_clicked();

    void on_pb_TestMail_clicked();

    void on_cb_SendMails_toggled(bool checked);

    void on_strIn_EmailAddress_textEdited();

    void on_pb_getSnapshot_clicked();

    void on_pb_refreshWebcamList_clicked();

private:
    Ui::MainWindow *ui;

    //Metodo per la conversione da Mat (libreria OpenCV) a QImage (Qt)
    QImage convertOpenCVMatToQtQImage(cv::Mat mat);

    //Metodo per la rilevazione di movimento
    bool ObjDetect(cv::Mat ImgRefCurrent, cv::Mat ImgRefBackground,
                    int *IntArrayMinMaxValues);

    //Metodo per la calibrazione
    float pixelCmCalibration(cv::Mat ImgSrc, float FloatCm);

    //Metodo per impostare l'immagine considerata di sfondo
    void setBkgImage();

    //Metodo per fermare i timer quando il programma viene terminato
    void exitProgram();

    //Metodo per far partire il salvataggio di un filmato
    void recordStart();

    //Metodo per fermare il salvataggio di un filmato
    void recordStop();

    //Invio di una email
    bool sendEmail(QString StringMailAddress, QString StringObject, 
        QString StringText);

    //Verifica validità mail
    bool isMailValid(QString StringMailAddress);

private:
    //Timer
    QTimer *timer;
    QTimer *one_sec;

    //Oggetti per l'acquisizione ed il salvataggio video
    cv::VideoCapture webcam;
    cv::VideoWriter video;

    // IMMAGINI //
    //Sfondo
    cv::Mat ImgBackground;

    //Immagine mostrata all'utente
    cv::Mat ImgShow;

    //Array contenente le ultime 10 immagini acquisite
    cv::Mat ArrayImgOld[10];

    // VARIABILI //
    //Contatore per l'aggiornamento dell'immagine di sfondo
    int passedFrames;

    //Booleano utilizzato per segnalare il movimento in corso
    bool BoolAlarm;

    //Utilizzato per attivare un fronte di salita nel salvataggio
    //del filmato
    bool BoolMovementActive;

    //Contatore per fermare il salvataggio del filmato qualche
    //secondo dopo la fine dell'intrusione
    int IntDelayFrames;

    //Rapporto tra pixel e centimetri
    float FloatPxCmRatio;

    //Nome del file del filmato che sta venendo salvato in quel momento
    QString StringCurrentSavingFileName;

    // IMPOSTAZIONI	 //

    //Index della webcam in uso
    int IntWebcamIndex;

    //Indirizzo di salvataggio dei filmati
    QString StringSavePath;

    //Abilitazione al salvataggio dei filmati
    bool BoolSaveVideo;

    //Abilitazione all'invio di email
    bool BoolSendMail;

    //Indirizzo email destinatario
    QString StringRecipient;

    //Casi di invio email
    bool BoolSendMailIfMovement;
    bool BoolSendMailIfAddressModified;
    bool BoolSendMailIfSessionEnded;

    //Invio di email
    SmtpClient *SmtpMail;

    QString StringTestMail;
    QString StringDetectedIntrusion;
    QString StringMailChanged;
    QString StringSessionEnded;
};

#endif // MAINWINDOW_H

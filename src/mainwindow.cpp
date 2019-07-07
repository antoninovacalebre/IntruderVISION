// INCLUSIONE LIBRERIE //

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
#include <stdio.h>

#include <QMessageBox>
#include <QDialog>
#include <QtCore>
#include <QCameraInfo>
#include <QDateTime>
#include <QFileDialog>
#include <QDesktopServices>
#include <QColor>

// COSTRUTTORE E DISTRUTTORE //

//Contruttore
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //Inizializzazione degli elementi della UI
    ui->setupUi(this);

    //Imposto una dimensione fissa per la finestra
    setFixedWidth(830);
    setFixedHeight(600);

    //Imposto il tab a quello del menu iniziale
    ui->tab_UI->setCurrentIndex(0);

    //Inizializzo l'indirizzo di salvataggio a quello
    //di sistema dei video
    ui->strIn_savePath->setText(
        QStandardPaths::standardLocations
                (QStandardPaths::MoviesLocation).last() );

    //Creo un elemento fittizio per evitare un bug nella
    //selezione della webcam (selezione vuota al primo avvio)
    ui->cb_selectWebcam->addItem("Placeholder");
    ui->cb_selectWebcam->setCurrentIndex(0);

    //Imposto di default il salvataggio come non attivo
    //E rendo invisibili i relativi controlli e indicatori
    ui->cb_SaveMedia->setChecked(false);
    ui->strIn_savePath->setVisible(false);
    ui->lblTxt_SavePath->setVisible(false);
    ui->pb_chooseFile->setVisible(false);

    //Imposto di default l'invio delle email come non attivo
    //e rendo invisibili i relativi controlli ed indicatori
    ui->cb_SendMails->setChecked(false);
    ui->strIn_EmailAddress->setText("");
    ui->strIn_EmailAddress->setVisible(false);
    ui->cb_MailAddressChanged->setVisible(false);
    ui->cb_MailMovementDetected->setVisible(false);
    ui->cb_MailSessionEnded->setVisible(false);
    ui->lblTxt_InserMailAddress->setVisible(false);
    ui->lblTxt_SendMailIf->setVisible(false);
    ui->pb_TestMail->setVisible(false);
    ui->lblTxt_isMailValid->setVisible(false);

    //Inizializzo le diverse variabili
    IntWebcamIndex = 0;
    IntDelayFrames = 0;
    BoolAlarm = false;
    BoolMovementActive = false;

    //Rapporto pixel/cm impostato a -1 per essere sicuri
    //se il sistema è stato calibrato o meno
    FloatPxCmRatio = -1;

    //Inizializzazione per emails
    SmtpMail = new SmtpClient("smtp.gmail.com", 465, 
        SmtpClient::SslConnection);

	///////////////////////////////////
    // Modify here to set email notifications
	SmtpMail->setUser("MAILHERE");
    SmtpMail->setPassword("PASSWORDHERE");
	///////////////////////////////////
	
    StringTestMail = "Mail di prova.";
    StringDetectedIntrusion = "Rilevato un movimento nell'area controllata.";
    StringSessionEnded = "La sessione di monitoraggio è stata terminata.";
    StringMailChanged = "L'indirizzo email di notifica è stato cambiato.";

    //Imposto il colore del testo di segnalazione come rosso
    ui->txte_errors->setTextColor( QColor::fromRgb(255, 0, 0, 255) );

    //Rendo invisibili gli indicatori degli fps (debug)
    ui->lblTxt_fps->setVisible(false);
    ui->sb_fpsCounter->setVisible(false);
}

//Distruttore
MainWindow::~MainWindow()
{
    delete ui;
}

// SELEZIONE DELLE SCHERMATE DAL MENU PRINCIPALE //

//Apertura della schermata di monitoraggio
void MainWindow::on_pb_openAcq_clicked()
{
    //Imposto il tab a quello di monitoraggio
    ui->tab_UI->setCurrentIndex(1);

    //Nascondo o rendo visibili gli elementi relativi
    //alle dimensioni degli oggetti
    if( FloatPxCmRatio == -1 )
    {
        ui->lblTxt_cmSQ->setVisible(false);
        ui->lblTxt_MovingArea->setVisible(false);
        ui->sb_DoubleArea->setVisible(false);
    }
    else
    {
        ui->lblTxt_cmSQ->setVisible(true);
        ui->lblTxt_MovingArea->setVisible(true);
        ui->sb_DoubleArea->setVisible(true);
    }

    // INIZIALIZZAZIONE DELL'ACQUISIZIONE //
    //Avvio l'oggetto della webcam
    webcam.open( ui->cb_selectWebcam->currentIndex() );

    //Verifico se ci sono problemi con la webcam
    if(webcam.isOpened() == false) {
        ui->cb_selectWebcam->setCurrentIndex(0);
        webcam.open(0);

        if(webcam.isOpened() == false)
        {
            QMessageBox::information(this, "", "Nessuna webcam rilevata.");
        }
    }

    //Inizializzo le variabili relative al monitoraggio
    setBkgImage();
    passedFrames = 0;
    BoolMovementActive = false;
    StringCurrentSavingFileName = "";

    // TIMER PER IL CALCOLO DEGLI FPS //

    //Creo il timer
    one_sec = new QTimer(this);

    //Collego il timer al relativo metodo
    connect(one_sec, SIGNAL(timeout()), this, SLOT(calculateFPS()));

    //Avvio il timer (1 s per calcolare gli FPS effettivi)
    one_sec->start(1000);

    // TIMER PER L'ACQUISIZIONE //

    //Creo il timer
    timer = new QTimer(this);

    //Collego il timer al relativo metodo
    connect(timer, SIGNAL(timeout()), this, SLOT(updateGUI()));

    //Avvio il timer (33 ms per avere 30 FPS)
    timer->start(33);
}

//Apertura della schermata delle impostazioni
void MainWindow::on_pb_openSettings_clicked()
{
    //Azzero il numero di webcam disponibili ed imposto quella
    //attuale come attiva.
    //Viene fatto per evitare che venga azzerata la selezione ogni
    //volta che si aprono le impostazioni
    int IntCamNumber = 0;
    int IntCurrentIndex = ui->cb_selectWebcam->currentIndex();

    //Imposto il tab a quello delle impostazioni
    ui->tab_UI->setCurrentIndex(2);

    //Imposto il sotto-tab a quello delle notifiche
    ui->tab_settings->setCurrentIndex(0);

    //Verifico quante e quali webcam sono disponibili
    //E le aggiungo al menu a tendina per selezionarle
    ui->cb_selectWebcam->clear();

    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    foreach (const QCameraInfo &cameraInfo, cameras)
    {
        ui->cb_selectWebcam->addItem(cameraInfo.description());
        IntCamNumber++;
    }

    if( IntCamNumber >= IntCurrentIndex )
    {
        ui->cb_selectWebcam->setCurrentIndex( IntCurrentIndex );
    }
    else
    {
        ui->cb_selectWebcam->setCurrentIndex(0);
    }

    //Nascondo o mostro l'indicatore della mail non valida
    if( !isMailValid( ui->strIn_EmailAddress->text() ) && 
        ui->cb_SendMails->isChecked() )
    {
        ui->lblTxt_isMailValid->setVisible(true);
    }
    else
    {
        ui->lblTxt_isMailValid->setVisible(false);
    }

    //Viene effettuato un backup delle impostazioni
    StringSavePath = ui->strIn_savePath->text();
    BoolSaveVideo = ui->cb_SaveMedia->isChecked();
    IntWebcamIndex = ui->cb_selectWebcam->currentIndex();

    BoolSendMail = ui->cb_SendMails->isChecked();
    StringRecipient = ui->strIn_EmailAddress->text();
    BoolSendMailIfMovement = ui->cb_MailMovementDetected->isChecked();
    BoolSendMailIfAddressModified = ui->cb_MailAddressChanged->isChecked();
    BoolSendMailIfSessionEnded = ui->cb_MailSessionEnded->isChecked();
}

//Apertura della schermata di calibrazione
void MainWindow::on_pb_openCalibration_clicked()
{
    //Imposto il tab a quello di calibrazione
    ui->tab_UI->setCurrentIndex(3);

    // INIZIALIZZAZIONE DELL'ACQUISIZIONE //
    //Avvio l'oggetto della webcam
    webcam.open( ui->cb_selectWebcam->currentIndex() );

    //Verifico se ci sono problemi con la webcam
    if(webcam.isOpened() == false) {
        ui->cb_selectWebcam->setCurrentIndex(0);
        webcam.open(0);

        if(webcam.isOpened() == false)
        {
            QMessageBox::information(this, "", "Nessuna webcam rilevata.");
        }
    }

    // TIMER DI ACQUISIZIONE //

    //Creo il timer
    timer = new QTimer(this);

    //Collego il timer al relativo metodo
    connect(timer, SIGNAL(timeout()), this, SLOT( calibrationHandler() ));

    //Avvio il timer (33 ms per avere 30 FPS)
    timer->start(33);
}

// METODI UTILIZZATI NELLE IMPOSTAZIONI //

//Toggle del checkbox per l'attivazione del salvataggio
void MainWindow::on_cb_SaveMedia_toggled(bool checked)
{
    //Rendo visibili od invisibili i relativi controlli
    //ed indicatori in base al nuovo stato del
    //checkbox
    ui->strIn_savePath->setVisible( checked );
    ui->lblTxt_SavePath->setVisible( checked );
    ui->pb_chooseFile->setVisible( checked );
}

//Toggle del checkbox per l'attivazione delle email
void MainWindow::on_cb_SendMails_toggled(bool checked)
{
    ui->cb_SendMails->setChecked(checked);
    ui->strIn_EmailAddress->setText("");
    ui->strIn_EmailAddress->setVisible(checked);
    ui->cb_MailAddressChanged->setVisible(checked);
    ui->cb_MailMovementDetected->setVisible(checked);
    ui->cb_MailSessionEnded->setVisible(checked);
    ui->lblTxt_InserMailAddress->setVisible(checked);
    ui->lblTxt_SendMailIf->setVisible(checked);
    ui->pb_TestMail->setVisible(checked);
    ui->lblTxt_isMailValid->setVisible(checked);
}

//Pulsante per la selezione della cartella di salvataggio premuto
void MainWindow::on_pb_chooseFile_clicked()
{
    //Apertura di una finestra per fare selezionare all'utente la cartella
    //di salvataggio
    QString QStringSavePath = QFileDialog::getExistingDirectory(this,
        tr("Open Directory"), "/home",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    //Verifico che la cartella selezionata sia valida e, se si,
    //la inserisco nel relativo indicatore
    if( QStringSavePath != "" )
    {
        ui->strIn_savePath->setText( QStringSavePath );
    }
}

//Pulsante per la conferma delle impostazioni premuto
void MainWindow::on_pb_confirm_clicked()
{
    //Verifico che la mail sia valida
    if( ui->cb_SendMails->isChecked() && !isMailValid
        ( ui->strIn_EmailAddress->text() ) )
    {
        //Torno al tab delle notifiche
        ui->tab_settings->setCurrentIndex(0);

        //Segnalo all'utente
        QMessageBox::warning(this, "Impostazioni",
            "L'indirizzo email inserito non è valido, "
            "l'invio delle email verrà ora disattivato.");

        //Disattivo l'invio delle email
        //Il resto delle disattivazioni avviene nell'evento "toggle"
        ui->cb_SendMails->toggle();
    }
    else
    {
        //Torna al tab del menu principale
        ui->tab_UI->setCurrentIndex(0);
    }

    //Si verifica se l'indirizzo email è stato cambiato, e
    //viene inviata una mail di segnalazione alla mail precedente
    //(se l'opzione è attivata)
    if( StringRecipient == ui->strIn_EmailAddress->text() &&
        ui->cb_SendMails->isChecked() &&
        ui->cb_MailAddressChanged->isChecked() &&
        isMailValid( StringRecipient ))
    {
        sendEmail( StringRecipient, "Email cambiata", StringMailChanged );
    }
}

//Pulsante per l'invio della mail di prova premuto
void MainWindow::on_pb_TestMail_clicked()
{
    if( isMailValid(ui->strIn_EmailAddress->text() ) )
    {
        if( sendEmail(ui->strIn_EmailAddress->text(),
                  "Email di prova", StringTestMail) )
        {
            QMessageBox::warning(this, "Invio email",
                "Connessione non riuscita.");
        }
}
    else
    {
        QMessageBox::warning(this, "Invio email di prova",
            "Indirizzo email non valido, "
            "impossibile inviare la mail di prova.");
    }
}

//Indirizzo email modificato dall'utente
void MainWindow::on_strIn_EmailAddress_textEdited()
{
    if( !isMailValid( ui->strIn_EmailAddress->text() ) )
    {
        ui->lblTxt_isMailValid->setVisible(true);
    }
    else
    {
        ui->lblTxt_isMailValid->setVisible(false);
    }
}

//Pulsante per l'aggiornamento della lista delle
//webcam
void MainWindow::on_pb_refreshWebcamList_clicked()
{
    //Azzero il numero di webcam disponibili ed imposto quella
    //attuale come attiva.
    //Viene fatto per evitare che venga azzerata la selezione ogni
    //volta che si aprono le impostazioni
    int IntCamNumber = 0;
    int IntCurrentIndex = ui->cb_selectWebcam->currentIndex();

    //Verifico quante e quali webcam sono disponibili
    //E le aggiungo al menu a tendina per selezionarle
    ui->cb_selectWebcam->clear();

    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    foreach (const QCameraInfo &cameraInfo, cameras)
    {
        ui->cb_selectWebcam->addItem(cameraInfo.description());
        IntCamNumber++;
    }

    if( IntCamNumber >= IntCurrentIndex )
    {
        ui->cb_selectWebcam->setCurrentIndex( IntCurrentIndex );
    }
    else
    {
        ui->cb_selectWebcam->setCurrentIndex(0);
    }
}

// METODI UTILIZZATI NEL MONITORAGGIO //

//Gestione del monitoragigo (ogni 33 ms)
void MainWindow::updateGUI()
{
    //Dichiarazione dell'immagine corrente
    //e dell'immagine di sfondo precedente
    cv::Mat ImgCurrent;
    cv::Mat ImgOldBkg;

    //Inizializzo i valori estremi del rettangolo che
    //indica l'area in movimento (valori molto alti
    //per i più piccoli, valori molto piccoli per i più grandi)
    int RectanglePoint[4] = {10000, 10000, 0, 0};

    //Acquisisco il frame corrente
    bool blnFrameReadSuccessfully = webcam.read(ImgCurrent);

    //Verifico che l'acquisizione sia avvenuta con successo ed eventualmente
    //lo si segnala all'utente
    if (!blnFrameReadSuccessfully || ImgCurrent.empty()) {
        QMessageBox::warning(this, "",
            "Errore nell'acquisizione, monitoraggio annullato.");
        webcam.release();

        //Se attivato, viene inviata una email di segnalazione
        if( ui->cb_MailSessionEnded->isChecked() && 
            ui->cb_SendMails->isChecked() )
        {
            sendEmail( ui->strIn_EmailAddress->text(),
                       "IntruderVISION: Errore acquisizione",
                       StringDetectedIntrusion);
        }
    }

    //Shifto le 10 immagini precendenti presenti nell'array
    //per salvare quella nuova
    for(int i = 0; i < 9; i++)
    {
        ArrayImgOld[i + 1].copyTo( ArrayImgOld[i] );
    }
    ImgCurrent.copyTo( ArrayImgOld[9] );

    //Copio l'immagine corrente in quella da mostrare all'utente
    //in modo che l'elaborazione non influisca su quello che viene
    //mostrato
    ImgCurrent.copyTo( ImgShow );

    //Se il numero di frame trascorsi dall'ultimo aggiornamento dello
    //sfondo è divisibile per 11 (ovvero ogni 11 frame), aggiorno
    //l'immagine di sfondo
    if( passedFrames % 11 == 0 )
    {
        ImgBackground.copyTo(ImgOldBkg);
        ImgCurrent.copyTo(ImgBackground);

        //Testo la situazione di allarme continuando ad utilizzare
        //l'immagine di sfondo precendente per non far credere al
        //programma che non ci sia più movimento
        BoolAlarm = ObjDetect(ImgCurrent, ImgOldBkg, RectanglePoint);
    }
    else
    {
        //Testo la situazione di allarme utilizzato l'immagine di sfondo
        //più aggiornata
        BoolAlarm = ObjDetect(ImgCurrent, ImgBackground, RectanglePoint);
    }

    //Incremento il numero di frame trascorsi
    passedFrames++;

    //Modifica dell'immagine da mostrare all'utente ed eventuale
    //salvataggio del filmato

    //Se c'è un movimento in corso...
    if( BoolAlarm )
    {
        //Mostra con un rettangolo verde l'area in movimento all'utente
        cv::rectangle(ImgShow, cv::Point(RectanglePoint[0], 
            RectanglePoint[1]), cv::Point(RectanglePoint[2], 
            RectanglePoint[3]), cv::Scalar(55,255,0), 5);

        //Se è stata effettuata una calibrazione, calcola l'area in cm^2
        //del corpo in movimento
        if( FloatPxCmRatio != -1 )
        {
            ui->sb_DoubleArea->setValue( 
                (RectanglePoint[3] - RectanglePoint[1]) * 
                (RectanglePoint[2] - RectanglePoint[0]) / 
                (FloatPxCmRatio * FloatPxCmRatio) );
        }

        //Gestione del salvataggio dei filmati
        if( BoolMovementActive == false )
        {
            StringCurrentSavingFileName =
                QDateTime::currentDateTime().toString("dd-MM-yy hh.mm.ss");

            ui->listOut_intrusions->insertItem(0,
                StringCurrentSavingFileName );

            if( ui->cb_SendMails->isChecked() && 
                ui->cb_MailMovementDetected->isChecked() )
            {
                sendEmail(ui->strIn_EmailAddress->text(), 
                    "IntruderVISION: Movimento individuato", 
                    StringDetectedIntrusion);
            }

            if( ui->cb_SaveMedia->isChecked() )
            {
                ui->listOut_intrusions->item(0)->setTextColor
                    (QColor::fromRgb(255, 0, 0, 255));
                
                recordStart();
            }

            BoolMovementActive = true;
        }

        IntDelayFrames = 0;
    }
    //Se non sono rilevati dei movimenti...
    else
    {
        //Imposto a zero l'area dell'oggetto in movimento
        ui->sb_DoubleArea->setValue(0);

        //Gestione del salvataggio dei filmati
        if( BoolMovementActive == true )
        {
            if(IntDelayFrames > 40)
            {
                BoolMovementActive = false;
                if( ui->cb_SaveMedia->isChecked() )
                {
                    recordStop();
                }
            }
            else    IntDelayFrames++;
        }
    }

    //Se necessario aggiungere il frame attuale al video che
    //sta venendo salvato
    if( video.isOpened() && ui->cb_SaveMedia->isChecked() )
    {
        video.write( ArrayImgOld[9] );
    }

    //Converto l'immagine da mostrare da Mat (formato di OpenCV)
    //in QImage, per poterlo mostrare nelle label di Qt
    QImage qImgShow = convertOpenCVMatToQtQImage(ImgShow);

    //Mostro l'immagine convertita all'utente
    ui->lbl_original->setPixmap(QPixmap::fromImage(qImgShow));
}

//Rilevazione di movimenti
bool MainWindow::ObjDetect(cv::Mat ImgRefCurrent, cv::Mat ImgRefBackground,
    int *IntArrayMinMaxValues)
{
    //Dichiaro l'immagine differenza
    cv::Mat ImgDiff;

    //Inizializzo i valori minimi e massimi del
    //rettangolo che indica dove avviene il movimento
    *IntArrayMinMaxValues = 10000;          //Min X
    *(IntArrayMinMaxValues + 1) = 10000;    //Min Y
    *(IntArrayMinMaxValues + 2) = 0;        //Max X
    *(IntArrayMinMaxValues + 3) = 0;        //Max Y

    //Inizializzo e dichiaro il numero di pixel bianchi
    int WhitePixels = 0;

    //Calcolo la differenza in valore assoluto tra l'immagine
    //corrente e quella di sfondo
    cv::absdiff(ImgRefBackground, ImgRefCurrent, ImgDiff);

    //Converto in bianco e nero l'immagine differenza ottenuta
    cv::cvtColor(ImgDiff, ImgDiff, CV_BGR2GRAY);

    //Applico un threshold all'immagine per ottenere solo pixel
    //completamente bianchi o completamente neri
    cv::threshold(ImgDiff, ImgDiff,35, 255, 0);

    //Conto il numero di pixel bianchi presenti nell'immagine e
    //verifico i valori minimi e massimi delle x e delle y di dove
    //si trovano
    for(int x = 0; x < ImgDiff.cols; x++)
    {
        for(int y = 0; y < ImgDiff.rows; y++)
        {
            cv::Scalar intensity = ImgDiff.at<uchar>(y, x);
            if( intensity.val[0] == 255 )
            {
                //Incremento il numero di pixel bianchi contati
                WhitePixels++;

                //Aggiorno le posizioni minime e massime dei
                //pixel bianchi trovati
                if(x > *(IntArrayMinMaxValues + 2))
                        *(IntArrayMinMaxValues + 2) = x;

                if(x < *IntArrayMinMaxValues)
                        *IntArrayMinMaxValues = x;

                if(y > *(IntArrayMinMaxValues + 3))
                        *(IntArrayMinMaxValues + 3) = y;

                if(y < *(IntArrayMinMaxValues + 1))
                        *(IntArrayMinMaxValues + 1) = y;
            }
        }
    }

    //Se ci sono più di un certo numero di pixel,
    //si segnala la presenza di movimento e si ritorna il valore
    //true, altrimenti false
    if(WhitePixels > 50)
    {

        ui->txte_errors->setText("Intrusione in corso!");
        return true;
    }
    else
    {
        ui->txte_errors->setText("");
        return false;
    }
}

//Doppio click su un elemento della lista delle intrusioni
void MainWindow::on_listOut_intrusions_itemDoubleClicked
    (QListWidgetItem *item)
{
    //Segnalazione di un errore se si sta cercando di aprire un file
    //mentre sta ancora venendo salvato
    if( QString::compare(item->text(),
        StringCurrentSavingFileName) == 0 && video.isOpened() )
    {
        QMessageBox::warning(this, "Filmato intrusione",
            "Non è possibile accedere al filmato durante il "
            "suo salvataggio!", QMessageBox::Ok );
    }
    //Se non ci sono problemi, si apre una finestra per 
    //visualizzare il filmato con il player video
    //di default del sistema
    else
    {
        QDesktopServices::openUrl(QUrl("file:" + 
            ui->strIn_savePath->text() + "/" + item->text() +
            ".avi", QUrl::TolerantMode));
    }
}

//Pulsante "Scatta foto" premuto
void MainWindow::on_pb_getSnapshot_clicked()
{
    if( webcam.isOpened() )
    {
        QString ImageSavePath = QStandardPaths::standardLocations
                (QStandardPaths::PicturesLocation).last();

        cv::imwrite( ImageSavePath.append("/").append
                     (QDateTime::currentDateTime().toString
                     ("dd-MM-yy hh.mm.ss")).append
                     (".png").toUtf8().constData(), ArrayImgOld[9]);
    }
}

//Impostazione dell'immagine di sfondo (usato solo al primo avvio)
void MainWindow::setBkgImage()
{
    //Acquisisco l'immagine attuale
    webcam.read(ImgBackground);

    //Inizializzo tutti gli elementi dell'array
    //con l'immagine di background
    for(int i = 0; i < 10; i++)
    {
        ImgBackground.copyTo( ArrayImgOld[i] );
    }
}

//Calcolo degli FPS (ogni secondo)
void MainWindow::calculateFPS()
{
    //Aggiorno il numero di FPS contati durante
    //il secondo
    ui->sb_fpsCounter->setValue( passedFrames );

    //Azzero il contatore
    passedFrames = 0;
}

//Avvio del salvataggio del filmato
void MainWindow::recordStart()
{
    //Avvio il salvataggio in un file denominato con la data e l'ora
    //attuali
    video.open( ui->strIn_savePath->text().append("/").append
        (QDateTime::currentDateTime().toString("dd-MM-yy hh.mm.ss")).append
        (".avi").toUtf8().constData() , CV_FOURCC('I','Y','U','V'),
        10, cv::Size(640, 480), true);

    //Vengono salvati i 10 frame precedenti all'avvio del
    //salvataggio
    for(int i = 0; i < 10; i++)
    {
        video.write( ArrayImgOld[i] );
    }
}

//Termine del salvataggio del filmato
void MainWindow::recordStop()
{
    //Viene terminato il salvataggio
    video.release();

    //Viene colorato di blu l'elemento della lista
    //delle intrusioni relativo al file salvato
    if( ui->cb_SaveMedia->isChecked() )
    {
        ui->listOut_intrusions->item(0)->setTextColor(
            QColor::fromRgb(0, 0, 255, 255));
    }
}

// METODI UTILIZZATI NELLA CALIBRAZIONE //

//Pulsante per avviare la calibrazione premuto
void MainWindow::on_pb_GetCalibrationImage_clicked()
{
    //Viene chiamato il metodo per la calibrazione
    FloatPxCmRatio = pixelCmCalibration(ImgBackground, 28.0f);

    //Se il metodo ritorna un valore valido, viene segnalato
    //all'utente l'avvenuta calibrazione e si torna al menu
    //principale
    if( FloatPxCmRatio != -1 )
    {
        QMessageBox::information(this, "Calibrazione",
            "Calibrazione riuscita!", QMessageBox::Ok );

        //Si ferma il timer
        if(timer->isActive()) timer->stop();

        //Si ferma l'acquisizione
        if(webcam.isOpened())   webcam.release();

        //Imposto il tab a quello del menu principale
        ui->tab_UI->setCurrentIndex(0);
    }
    else
    {
        QMessageBox::warning(this, "Calibrazione",
            "Pattern di calibrazione utilizzato non valido!");
    }
}

//Calcolo del rapporto tra pixel e centimetri
float MainWindow::pixelCmCalibration(cv::Mat ImgSrc, float FloatCm)
{
    //Dichiarazione del vettore dove salvare le informazioni
    //sui cerchi rilevati
    std::vector<cv::Vec3f> Vec3fCirclesFound;

    //Dichiarazione delle due variabili usate nei calcoli
    float FloatDistancePx;
    float FloatPixelCmRatio;

    //Viene convertita l'immagine in ingresso in scala di grigi
    cv::cvtColor(ImgSrc, ImgSrc, CV_BGR2GRAY);

    //Viene sfocata l'immagine per eliminare rumori
    cv::GaussianBlur(ImgSrc, ImgSrc, cv::Size(5, 5), 1.5);

    //Si cercano i cerchi presenti nell'immagine acquisita
    cv::HoughCircles(ImgSrc, Vec3fCirclesFound, CV_HOUGH_GRADIENT,
        1, 10, 100,30, 1, 30);

    //Si controlla che il numero di cerchi individuato sia 2
    //(fare riferimento al foglio di calibrazione)
    //Se si, si calcola la distanza in pixel tra i due centri, e lo
    //si divide per la distanza in cm fornita come argomento del metodo
    if( Vec3fCirclesFound.size() == 2)
    {
        cv::Point2f PointCircleA( Vec3fCirclesFound[0][0],
            Vec3fCirclesFound[0][1] );

        cv::Point2f PointCircleB( Vec3fCirclesFound[1][0],
            Vec3fCirclesFound[1][1] );

        cv::Point PointDiff(PointCircleA - PointCircleB);

        FloatDistancePx = std::sqrt(PointDiff.x * PointDiff.x +
            PointDiff.y * PointDiff.y);

        FloatPixelCmRatio = FloatDistancePx / FloatCm;

        return FloatPixelCmRatio;
    }
    //Altrimenti si segnala l'errore e si ritorna -1
    else
    {
        return -1.0f;
    }
}

//Gestione della calibrazione (ogni 33 ms)
void MainWindow::calibrationHandler()
{
    //Acquisizione dell'immagine corrente e sua copia
    //nell'immagine da mostrare all'utente
    bool blnFrameReadSuccessfully = webcam.read(ImgShow);
    ImgShow.copyTo(ImgBackground);

    //Verifico che l'immagine sia stata acquisita correttamente
    if (!blnFrameReadSuccessfully || ImgShow.empty()) {
        QMessageBox::information(this, "",
            "unable to read from webcam \n\n exiting program\n");
        exitProgram();
        return;
    }

    //Converto l'immagine nel formato QImage
    QImage qImgShow = convertOpenCVMatToQtQImage(ImgShow);

    //Mostro all'utente l'immagine convertita
    ui->lbl_CalibPreview->setPixmap(QPixmap::fromImage(qImgShow));
}

// PULSANTI PER TORNARE AL MENU PRINCIPALE //

//Dalla schermata di monitoraggio
void MainWindow::on_pb_backToMenu_clicked()
{
    //Vengono fermati i timer
    if(timer->isActive()) timer->stop();
    if(one_sec->isActive())  one_sec->stop();

    //Chiuse le sessioni di acquisizione e salvataggio
    if(webcam.isOpened())   webcam.release();
    if(video.isOpened())    recordStop();

    //Impostato il tab a quello del menu principale
    ui->tab_UI->setCurrentIndex(0);
}

//Dalla schermata delle impostazioni
void MainWindow::on_pb_backToMenu2_clicked()
{
    //Imposto il tab a quello del menu
    ui->tab_UI->setCurrentIndex(0);

    //Dato che il pulsante "Conferma" non è stato premuto, vengono
    //reimpostati i valori delle impostazioni a prima che venissero
    //modificate
    ui->strIn_savePath->setText( StringSavePath );
    ui->cb_SaveMedia->setChecked( BoolSaveVideo );
    ui->cb_selectWebcam->setCurrentIndex( IntWebcamIndex );
    ui->cb_SendMails->setChecked( BoolSendMail );
    ui->strIn_EmailAddress->setText( StringRecipient );
    ui->cb_MailMovementDetected->setChecked( BoolSendMailIfMovement );
    ui->cb_MailAddressChanged->setChecked( BoolSendMailIfAddressModified );
    ui->cb_MailSessionEnded->setChecked( BoolSendMailIfSessionEnded );
}

//Dalla schermata di calibrazione
void MainWindow::on_pb_backToMenu3_clicked()
{
    //Fermo il timer
    if(timer->isActive()) timer->stop();

    //Chiudo la sessione di acquisizione
    if(webcam.isOpened())   webcam.release();

    //Imposto il tab a quello del menu
    ui->tab_UI->setCurrentIndex(0);
}

// USO GENERICO //

//Conversione da Mat (formato usato dalla libreria OpenCV)
//in QImage (formato usato da Qt per gestire le immagini)
QImage MainWindow::convertOpenCVMatToQtQImage(cv::Mat mat)
{
    //Se in bianco e nero...
    if(mat.channels() == 1) {
        return QImage((uchar*)mat.data, mat.cols,
            mat.rows, mat.step, QImage::Format_Indexed8);
    }
    //Se a colori...
    else if(mat.channels() == 3) {
        //Viene prima convertita l'immagine da BGR a RGB
        cv::cvtColor(mat, mat, CV_BGR2RGB);
        return QImage((uchar*)mat.data, mat.cols, mat.rows, 
                    mat.step, QImage::Format_RGB888);
    }
    //Se nessuno dei due...
    else {
        qDebug() << "Errore nella conversione: l'immagine non"
                    "è né in bianco e nero né a colori";
    }
    return QImage();
}

//Invio di email
bool MainWindow::sendEmail(QString StringMailAddress, QString StringObject,
                           QString StringText)
{
    //Creo la mail stessa
    MimeMessage SmtpMessage;

    EmailAddress sender("acquisizione.dati.ad@gmail.com", "IntruderVISION");
    SmtpMessage.setSender(&sender);

    EmailAddress to(StringMailAddress, StringMailAddress);
    SmtpMessage.addRecipient(&to);

    SmtpMessage.setSubject("Notifica IntruderVISION");

    //Creo ed imposto il testo della email
    MimeText text;

    text.setText(StringText);

    //Aggiungo il testo alla mail completa
    SmtpMessage.addPart(&text);

    //Invio la mail ed eventualmente segnalo la presenza di un errore
    if ( !SmtpMail->connectToHost() )
    {
        qDebug() << "Failed to connect to host!" << endl;
        return true;
    }
    else if (!SmtpMail->login())
    {
        qDebug() << "Failed to login!" << endl;
        return true;
    }
    else if (!SmtpMail->sendMail( SmtpMessage ))
    {
        qDebug() << "Failed to send mail!" << endl;
        return true;
    }
    else
    {
        SmtpMail->quit();
        return true;
    }
}

//Verifica validità email
bool MainWindow::isMailValid(QString StringMailAddress)
{
    //Dichiaro un pattern per verificare la validità di una email
    QRegularExpression validMailExpression
        ( "^[0-9a-zA-Z]+([0-9a-zA-Z]*[-._+])*"
          "[0-9a-zA-Z]+@[0-9a-zA-Z]+([-.][0-9a-zA-Z]+)*"
          "([0-9a-zA-Z]*[.])[a-zA-Z]{2,6}$" );

    //Verifico le corrispondenze con la mail in ingresso
    if( validMailExpression.match(StringMailAddress).hasMatch() )
    {
        return true;
    }
    else
    {
        return false;
    }
}

//Chiusura applicativo
void MainWindow::exitProgram()
{
    //Vengono fermati i timer
    if(timer->isActive()) timer->stop();
    if(one_sec->isActive())  one_sec->stop();

    //Viene terminata l'applicazione
    QApplication::quit();
}



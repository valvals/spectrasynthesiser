#include "SpectraSynthesizer.h"
#include "qjsonarray.h"
#include "qjsonobject.h"
#include "ui_SpectraSynthesizer.h"
#include "QVBoxLayout"
#include "QLabel"
#include "qslider.h"
#include "QDebug"
#include "QMessageBox"
#include "DBJson.h"

QHash<QString,QSlider*> m_sliders;
QStringList lambdas;
//constexpr char serial_number[] = "551393135353517061C2";

SpectraSynthesizer::SpectraSynthesizer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SpectraSynthesizer)
{
    ui->setupUi(this);
    QJsonObject jo;
    db_json::getJsonObjectFromFile("config.json",jo);
    QJsonArray ja;
    ja = jo.value("pins_array").toArray();
    qDebug()<<"ja size: "<<ja.size();
    const QString serial_number = jo.value("serial_id").toString();
    qDebug()<<"json_test: "<<serial_number;
    auto available_ports = m_serial_port_info.availablePorts();
    qDebug()<< available_ports.size();
    bool isDeviceConnected = false;
    for(int i=0;i<available_ports.size();++i){
        qDebug()<<available_ports[i].serialNumber()
                <<available_ports[i].portName();
        if(serial_number == available_ports[i].serialNumber()){
            m_serial_port.setPort(available_ports[i]);
            m_serial_port.open(QIODevice::ReadWrite);
            isDeviceConnected = true;
            connect(&m_serial_port,SIGNAL(readyRead()),this,SLOT(readData()));
            break;
        }
    }
    if(!isDeviceConnected){//  TO CHANGE
    lambdas<<"397.7" // a1
           <<"404.5" // a2
           <<"412.7" // a3
           <<"427.2" // a4
           <<"433.7" // a5
           <<"445.5" // a6
           <<"452.7"  // a7
           <<"461.9" // a8
           <<"473.6" // a9
           <<"505.0" // a10
           <<"534.2" // a11
           <<"549.2" // a12
           <<"560.5" // a13
           <<"588.9" // a14
           <<"595.5" // a15
           <<"607.5" // a16
           <<"627.1" // a17
           <<"640.1" // a18
           <<"658.7" // a19
           <<"679.0" // a20
           <<"725.1" // a21
           <<"745.8" // a22
           <<"775.0" // a23
           <<"800.6" // a24
           <<"820.0" // a25
           <<"839.1" // a26
           <<"893.4" // a27
           <<"931.4" // a28
           <<"967.1" // a29
           <<"1014.6";// a30


    for(int i=0;i<ja.size();++i){
        auto slider = new QSlider;
        slider->setObjectName(QString("qslider_")+QString::number(i+1));
        QVBoxLayout* vbl = new QVBoxLayout;
        auto wave = ja[i].toObject().value("wave").toString();
        vbl->addWidget(new QLabel(wave));
        auto max_value = ja[i].toObject().value("max_value").toInt();
        slider->setMaximum(max_value);
        vbl->addWidget(slider);
        ui->horizontalLayout->addLayout(vbl);
        m_sliders.insert(slider->objectName(),slider);
        connect(slider,&QSlider::sliderReleased,[i,slider, this](){
            qDebug()<<""<<slider->objectName();
            QString style1 = R"(<html><head/><body><p><span style=" font-size:28pt;">)";
            QString style2 = lambdas.at(i)+" --> ";
            style2.append(QString::number(slider->value()));
            QString style3 = R"(</span></p></body></html>)";
            slider->setToolTip(style1+style2+style3);
            qDebug()<<slider->objectName();
            sendDataToComDevice(QString("a") + QString::number(i+1)+"\n" +
                                QString("v") + (QString::number(slider->value())));
        });
    }
    }else{
        QMessageBox mb;
        mb.setIcon(QMessageBox::Warning);
        mb.setText("Устройство не подключено!");
        mb.exec();
    }
}

SpectraSynthesizer::~SpectraSynthesizer()
{
    delete ui;
}

void SpectraSynthesizer::readData()
{
    const QByteArray data = m_serial_port.readAll();
    qDebug()<<"recieved data: --> "<<data;
}

void SpectraSynthesizer::sendDataToComDevice(QString command)
{

    qDebug()<<"Test data before sending: "<<command;
    m_serial_port.write(command.toLatin1());
}



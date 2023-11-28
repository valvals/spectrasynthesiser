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

SpectraSynthesizer::SpectraSynthesizer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SpectraSynthesizer)
{
    ui->setupUi(this);
    db_json::getJsonObjectFromFile("config.json",m_json_config);
    QJsonArray ja;
    ja = m_json_config.value("pins_array").toArray();
    qDebug()<<"ja size: "<<ja.size();
    const QString serial_number = m_json_config.value("serial_id").toString();
    auto mode = m_json_config.value("mode").toString();
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
    if(isDeviceConnected || mode == "developing"){

    for(int i=0;i<ja.size();++i){
        auto slider = new QSlider;
        slider->setObjectName(QString("qslider_")+QString::number(i+1));
        QVBoxLayout* vbl = new QVBoxLayout;
        auto wave = ja[i].toObject().value("wave").toString();
        vbl->addWidget(new QLabel(wave));
        auto max_value = ja[i].toObject().value("max_value").toInt();
        slider->setMaximum(max_value);
        vbl->addWidget(slider);
        auto color = ja[i].toObject().value("color").toString();
        slider->setStyleSheet(QString("QSlider::handle:vertical {border-radius:5px;background:%1;}").arg(color));
        ui->horizontalLayout->addLayout(vbl);
        m_sliders.insert(slider->objectName(),slider);
        connect(slider,&QSlider::sliderReleased,[i,slider,ja,this](){

            QString style1 = R"(<html><head/><body><p><span style=" font-size:28pt;">)";
            QString style2 = ja[i].toObject().value("wave").toString()+" --> ";
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



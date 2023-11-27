#include "SpectraSynthesizer.h"
#include "ui_SpectraSynthesizer.h"
#include "QHBoxLayout"
#include "qslider.h"
#include "QDebug"

QHash<QString,QSlider*> m_sliders;
QStringList lambdas;

SpectraSynthesizer::SpectraSynthesizer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SpectraSynthesizer)
{
    ui->setupUi(this);
    auto available_ports = m_serial_port_info.availablePorts();
    qDebug()<< available_ports.size();
    for(int i=0;i<available_ports.size();++i){
        qDebug()<<available_ports[i].manufacturer()<<available_ports[i].portName();
    }
    if(available_ports.size()>0)m_serial_port.setPort(available_ports[0]);
    m_serial_port.open(QIODevice::ReadWrite);
    m_serial_port.write("Hello");
    m_device.setPort(available_ports[1]);

    lambdas<<"595.54318" // a1
          <<"505.065795" // a2
         <<"588.983265" // a3
        <<"534.276005" // a4
       <<"560.515685" // a5
      <<"549.252425" // a6
     <<"745.80248" // a7
    <<"725.132545"// a8
    <<"461.993115"// a9
    <<"775.012685"// a10
    <<"627.10506" // a11
    <<"607.549075"// a12
    <<"658.79071" // a13
    <<"640.101125"// a14
    <<"473.627685"// a15
    <<"679.08933" // a16
    <<"" // a17
    <<"" // a18
    <<"427.21316" // a19
    <<"433.77308" // a20
    <<"412.73183" // a21
    <<"445.531425" // a22
    <<"397.755405" // a23
    <<"404.562875" // a24
    <<"820.06572" // a25
    <<"800.63351" // a26
    <<"893.46256" // a27
    <<"839.12662" // a28
    <<"967.106945" // a29
    <<"931.46059" // a30
    <<"452.71021" // a31
    <<"1014.63542"; // a32
    connect(this,SIGNAL(sendData(QString)),this,SLOT(sendDataToComDevice(QString)));
    for(int i=0;i<32;++i){
        auto slider = new QSlider;
        slider->setObjectName(QString("qslider_")+QString::number(i+1));
        ui->horizontalLayout->addWidget(slider);
        m_sliders.insert(slider->objectName(),slider);
        connect(slider,&QSlider::sliderReleased,[i,slider, this](){
            qDebug()<<""<<slider->objectName();
            slider->setToolTip(lambdas.at(i));
            qDebug()<<m_sliders.value(slider->objectName())->value();
            emit sendData(QString("a")+QString::number(i+1)+QString("v")+(QString::number(slider->value())));
        });
    }
}

SpectraSynthesizer::~SpectraSynthesizer()
{
    delete ui;
}


void SpectraSynthesizer::on_pushButton_clicked()
{
    m_serial_port.write("Hello");
    qDebug()<<"test";
}

void SpectraSynthesizer::readData()
{
    const QByteArray data = m_device.readAll();
}

void SpectraSynthesizer::sendDataToComDevice(QString command)
{

    qDebug()<<"Test data before sending: "<<command;
    m_serial_port.write(command.toLatin1());
}


void SpectraSynthesizer::sliderValueChanged(int value, QString objName)
{

}


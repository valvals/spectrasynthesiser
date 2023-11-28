#include "SpectraSynthesizer.h"
#include "ui_SpectraSynthesizer.h"
#include "QHBoxLayout"
#include "qslider.h"
#include "QDebug"
#include "QMessageBox"

QHash<QString,QSlider*> m_sliders;
QStringList lambdas;
constexpr char serial_number[] = "551393135353517061C2";

SpectraSynthesizer::SpectraSynthesizer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SpectraSynthesizer)
{
    ui->setupUi(this);
    auto available_ports = m_serial_port_info.availablePorts();
    qDebug()<< available_ports.size();
    bool isDeviceConnected = false;
    for(int i=0;i<available_ports.size();++i){
        qDebug()<<available_ports[i].serialNumber()
                <<available_ports[i].portName();
        if(available_ports[i].serialNumber() == serial_number){
            m_serial_port.setPort(available_ports[i]);
            m_serial_port.open(QIODevice::ReadWrite);
            isDeviceConnected = true;
        }
    }
    if(!isDeviceConnected){//  TEMP
    lambdas<<"397.755" // a1
           <<"404.563" // a2
           <<"412.732" // a3
           <<"427.213" // a4
           <<"433.773" // a5
           <<"445.531" // a6
           <<"452.71"  // a7
           <<"461.993" // a8
           <<"473.628" // a9
           <<"505.066" // a10
           <<"534.276" // a11
           <<"549.252" // a12
           <<"560.516" // a13
           <<"588.983" // a14
           <<"595.543" // a15
           <<"607.549" // a16
           <<"627.105" // a17
           <<"640.101" // a18
           <<"658.791" // a19
           <<"679.089" // a20
           <<"725.133" // a21
           <<"745.802" // a22
           <<"775.013" // a23
           <<"800.634" // a24
           <<"820.066" // a25
           <<"839.127" // a26
           <<"893.463" // a27
           <<"931.461" // a28
           <<"967.107" // a29
           <<"1014.64";// a30


    for(int i=0;i<30;++i){
        auto slider = new QSlider;
        slider->setObjectName(QString("qslider_")+QString::number(i+1));
        ui->horizontalLayout->addWidget(slider);
        m_sliders.insert(slider->objectName(),slider);
        connect(slider,&QSlider::sliderReleased,[i,slider, this](){
            qDebug()<<""<<slider->objectName();
            QString style1 = R"(<html><head/><body><p><span style=" font-size:28pt;">)";
            QString style2 = lambdas.at(i)+" --> ";
            style2.append(QString::number(slider->value()));
            QString style3 = R"(</span></p></body></html>)";
            slider->setToolTip(style1+style2+style3);
            qDebug()<<m_sliders.value(slider->objectName())->value();
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


void SpectraSynthesizer::sliderValueChanged(int value, QString objName)
{

}


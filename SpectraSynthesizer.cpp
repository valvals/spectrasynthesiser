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
#include "windows.h"

QVector<QSlider*> m_sliders;
QHash<QString,size_t> lambdas_indexes;
const char styleSlider[]=R"(
QSlider::groove:vertical {
    background: #404244;
    position: absolute;
    left: 4px; right: 4px;
}

QSlider::handle:vertical {
    height: 30px;
    border-radius: 10px;
    background: %1;
    margin: 0 -4px;
}

QSlider::add-page:vertical {
    background: %2;
}
)";

SpectraSynthesizer::SpectraSynthesizer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SpectraSynthesizer)
{
    ui->setupUi(this);
    db_json::getJsonObjectFromFile("config.json",m_json_config);
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
        slider->setMinimumWidth(30);
        QVBoxLayout* vbl = new QVBoxLayout;
        auto wave = ja[i].toObject().value("wave").toString();
        ui->comboBox_waves->addItem(wave);
        lambdas_indexes.insert(wave,i);
        vbl->addWidget(new QLabel(wave));
        auto max_value = ja[i].toObject().value("max_value").toInt();
        slider->setMaximum(max_value);
        vbl->addWidget(slider);
        auto color = ja[i].toObject().value("color").toString();
        slider->setStyleSheet(QString(styleSlider).arg(color,color));//QString("QSlider::handle {width:50px;height:50px;border-radius:5px;background:%1;}").arg(color));
        ui->horizontalLayout->addLayout(vbl);
        m_sliders.push_back(slider);
        connect(slider,&QSlider::sliderReleased,[i,slider,this](){

            QString style1 = R"(<html><head/><body><p><span style=" font-size:28pt;">)";
            QString style2 = ja[i].toObject().value("wave").toString()+" --> ";
            style2.append(QString::number(slider->value()));
            QString style3 = R"(</span></p></body></html>)";
            slider->setToolTip(style1+style2+style3);
            qDebug()<<slider->objectName();
            sendDataToComDevice(QString("a") + QString::number(i+1) + "\n");
            Sleep(200);
            sendDataToComDevice(QString("v") + (QString::number(slider->value()) + "\n"));
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

void SpectraSynthesizer::on_pushButton_reset_to_zero_clicked()
{
  sendDataToComDevice("f\n");
  for(auto &&it:m_sliders)it->setValue(0);
}

void SpectraSynthesizer::on_pushButton_apply_clicked()
{
    auto index = lambdas_indexes.value(ui->comboBox_waves->currentText());
    auto value = ui->spinBox_bright_value->value();
    m_sliders[index]->setValue(value);
    sendDataToComDevice(QString("a%1\n").arg(QString::number(index)));
}


void SpectraSynthesizer::on_comboBox_waves_currentTextChanged(const QString &arg1)
{
    auto index = lambdas_indexes.value(arg1);
    auto max = ja[index].toObject().value("max_value").toInt();
    ui->spinBox_bright_value->setMaximum(max);
    ui->label_value->setToolTip(QString("макс: %1").arg(QString::number(max)));
}


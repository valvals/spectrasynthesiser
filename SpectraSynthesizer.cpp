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
#include "QrcFilesRestorer.h"
#include "windows.h"
#include "style_sheets.h"

SpectraSynthesizer::SpectraSynthesizer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SpectraSynthesizer)
{
    ui->setupUi(this);
    if(!db_json::getJsonObjectFromFile("config.json",m_json_config)){
        qDebug()<<"Config file was not found on the disk...";
       db_json::getJsonObjectFromFile(":/config.json",m_json_config);
       QrcFilesRestorer::restoreFilesFromQrc(":/");
    };
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
        slider->setStyleSheet(QString(styles::slider).arg(color,color));
        ui->horizontalLayout->addLayout(vbl);
        m_sliders.push_back(slider);
        connect(slider,&QSlider::sliderReleased,[i,slider,this](){
            setTooltipForSlider(i,slider->value());
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

void SpectraSynthesizer::setTooltipForSlider(const int& index, const int& value)
{
    QString waveStr = ja[index].toObject().value("wave").toString();
    QString valueStr = (QString::number(value));
    m_sliders[index]->setToolTip(QString(styles::tooltip).arg(waveStr,valueStr));
}

void SpectraSynthesizer::on_pushButton_reset_to_zero_clicked()
{
  sendDataToComDevice("f\n");
  for(int i=0;i<m_sliders.size();++i){
      m_sliders[i]->setValue(1);
      setTooltipForSlider(i,1);
  }
}

void SpectraSynthesizer::on_pushButton_apply_clicked()
{
    auto index = lambdas_indexes.value(ui->comboBox_waves->currentText());
    auto value = ui->spinBox_bright_value->value();
    m_sliders[index]->setValue(value);
    setTooltipForSlider(index,value);
    sendDataToComDevice(QString("a") + QString::number(index+1) + "\n");
    Sleep(200);
    sendDataToComDevice(QString("v") + (QString::number(value) + "\n"));
}

void SpectraSynthesizer::on_comboBox_waves_currentTextChanged(const QString &arg1)
{
    auto index = lambdas_indexes.value(arg1);
    auto max = ja[index].toObject().value("max_value").toInt();
    ui->spinBox_bright_value->setMaximum(max);
    ui->label_value->setToolTip(QString("макс: %1").arg(QString::number(max)));
}

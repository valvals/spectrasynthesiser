#ifndef SPECTRASYNTHESIZER_H
#define SPECTRASYNTHESIZER_H

#include <QMainWindow>
#include "QSerialPort"
#include "QSerialPortInfo"
#include "qjsonarray.h"
#include "qjsonobject.h"


QT_BEGIN_NAMESPACE
namespace Ui { class SpectraSynthesizer; }
QT_END_NAMESPACE

class SpectraSynthesizer : public QMainWindow
{
    Q_OBJECT

public:
    SpectraSynthesizer(QWidget *parent = nullptr);
    ~SpectraSynthesizer();

signals:
    void sendData(QString);

private slots:
    void readData();

    void on_pushButton_reset_to_zero_clicked();

    void on_pushButton_apply_clicked();

    void on_comboBox_waves_currentTextChanged(const QString &arg1);

private:
    Ui::SpectraSynthesizer *ui;
    QJsonObject m_json_config;
    QJsonArray ja;
    QSerialPort m_serial_port;
    QSerialPortInfo m_serial_port_info;
    void sendDataToComDevice(const QString command);
};
#endif // SPECTRASYNTHESIZER_H

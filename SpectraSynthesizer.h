#ifndef SPECTRASYNTHESIZER_H
#define SPECTRASYNTHESIZER_H

#include <QMainWindow>
#include "QSerialPort"
#include "QSerialPortInfo"


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

private:
    Ui::SpectraSynthesizer *ui;
    QSerialPort m_serial_port;
    QSerialPortInfo m_serial_port_info;
    void sliderValueChanged(int value, QString objName);
    void sendDataToComDevice(const QString command);
};
#endif // SPECTRASYNTHESIZER_H

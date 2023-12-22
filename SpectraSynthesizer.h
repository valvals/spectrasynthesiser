#ifndef SPECTRASYNTHESIZER_H
#define SPECTRASYNTHESIZER_H

#include <QMainWindow>
#include "QSerialPort"
#include "QSerialPortInfo"
#include "qjsonarray.h"
#include "qjsonobject.h"
#include "qslider.h"

#pragma pack(push,1)
struct SpectrumData
{
    unsigned short  dummy1[14];     // 32 байта (нужно 16, заменено для STM)
    short int       black1[13];     // 26 байт
    unsigned short  dummy2[3];      // 6 байт
    short int       spectrum[3648]; // 7296 байт
    unsigned short  dummy[14];      // 28 байт
};  //!< Spectrum data structure
Q_DECLARE_METATYPE(SpectrumData)
#pragma pack(pop)


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
    void readStmData();
    void on_pushButton_reset_to_zero_clicked();
    void on_pushButton_apply_clicked();
    void on_comboBox_waves_currentTextChanged(const QString& arg1);
    //spectrReadyToShow(QVector<double> data, double max, bool isNeedToUpdate);
    void show_stm_spectr(QVector<double> data, double max, bool isNeedToUpdate);

    void on_pushButton_clicked();

private:
    Ui::SpectraSynthesizer* ui;
    QJsonObject m_json_config;
    QJsonArray ja;
    QVector<QSlider*> m_sliders;
    QHash<QString,int> lambdas_indexes;
    QSerialPort* m_serial_diods_controller;
    QSerialPort* m_serial_stm_spectrometr;
    QSerialPortInfo m_serial_port_info;
    void sendDataToComDevice(const QString& command);
    void setTooltipForSlider(const int& index, const int& value);
};
#endif // SPECTRASYNTHESIZER_H

#ifndef SPECTRASYNTHESIZER_H
#define SPECTRASYNTHESIZER_H

#include <QMainWindow>
#include "QSerialPort"
#include "QSerialPortInfo"
#include "QTimer"
#include "qjsonarray.h"
#include "qjsonobject.h"
#include "qslider.h"
#include "debug_console.h"
#include "QElapsedTimer"
#include "qcustomplot.h"


const uint16_t spectr_values_size = 3648;


;// hack unterminated warning pack
#pragma pack(push,1)
struct SpectrumData {
  unsigned short  dummy1[14];                   // 32 байта (нужно 16, заменено для STM)
  short int       black1[13];                   // 26 байт
  unsigned short  dummy2[3];                    // 6 байт
  short int       spectrum[spectr_values_size]; // 7296 байт
  unsigned short  dummy[14];                    // 28 байт
};  //!< Spectrum data structure
Q_DECLARE_METATYPE(SpectrumData)
#pragma pack(pop)


QT_BEGIN_NAMESPACE
namespace Ui { class SpectraSynthesizer; }
QT_END_NAMESPACE

class SpectraSynthesizer : public QMainWindow {
  Q_OBJECT

 public:
  SpectraSynthesizer(QWidget* parent = nullptr);
  ~SpectraSynthesizer();

 signals:
  void sendData(QString);

 private slots:
  void readDiodsData();
  void readStmData();
  void show_stm_spectr(QVector<double> data, double max);
  void changeWidgetState();
  void updatePowerStat();
  void copyPowerStatToClipboard();
  void createSamplesJson();
  void on_pushButton_reset_to_zero_clicked();
  void on_pushButton_apply_clicked();
  void on_comboBox_waves_currentTextChanged(const QString& arg1);
  void on_pushButton_update_stm_spectr_clicked();
  void on_pushButton_exposition_clicked();
  void on_pushButton_sound_switcher_toggled(bool checked);
  void on_pushButton_water_clicked();
  void on_comboBox_etalons_currentIndexChanged(const QString& arg1);

 private:
  Ui::SpectraSynthesizer* ui;
  DebugConsole* m_debug_console;
  QJsonObject m_etalons;
  QJsonObject m_json_config;
  QJsonArray m_power_tracker;
  QJsonArray m_pins_json_array;
  QVector<QSlider*> m_sliders;
  QVector<uint16_t> m_sliders_previous_values;
  QVector<QElapsedTimer> m_elapsed_timers;
  QVector<double>m_etalons_grid;
  QHash<QString, int> lambdas_indexes;
  QSerialPort* m_serial_diods_controller;
  QSerialPort* m_serial_stm_spectrometr;
  QSerialPortInfo m_serial_port_info;
  QTimer m_timer;
  QCustomPlot* m_power_stat_plot;
  QVector<QCPBars*> m_power_bars;
  QCPBars* m_hours_bar;
  QVector<double> m_power_ticks;
  QVector<QString> m_power_labels;
  QCustomPlot* m_hours_stat_plot;
  void sendDataToComDevice(const QString& command);
  void setTooltipForSlider(const int& index, const int& value);
  QString getGroupID(const double& value);
  void setupPowerStatPlot();
  void savePowerParams(const int& index, const int& value);
  void showCurrentEtalon();
  void loadEtalons();
  // QWidget interface
 protected:
  void closeEvent(QCloseEvent* event) override;
};
#endif // SPECTRASYNTHESIZER_H

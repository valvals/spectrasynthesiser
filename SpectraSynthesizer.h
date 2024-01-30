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

enum class view {
  PVD_AZP,
  PVD_SPEYA,
  ETALON_PVD
};

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
  void show_stm_spectr(QVector<double> channels, QVector<double> data, double max);
  void changeWidgetState();
  void updatePowerStat();
  void copyPowerStatToClipboard();
  void createSamplesJson();
  void reset_all_diods_to_zero();
  void mayBeHideEtalon(bool isHide);
  void update_stm_spectr();
  void copy_spectr_to_clipboard();
  void copy_etalon_to_clipboard();
  void copy_data_plot_to_clipboard(QSharedPointer<QCPGraphDataContainer>data);
  void load_pvd_calibr();

  void on_pushButton_apply_clicked();
  void on_comboBox_waves_currentTextChanged(const QString& arg1);
  void on_comboBox_etalons_currentIndexChanged(const QString& arg1);
  void on_spinBox_exposition_valueChanged(int arg1);
  void on_pushButton_stop_start_update_stm_spectr_toggled(bool checked);

 private:
  Ui::SpectraSynthesizer* ui;
  bool m_is_show_etalon;
  bool m_is_stm_spectr_update;
  bool m_is_stm_exposition_changed;
  bool m_is_stm_spectrometr_connected;
  bool m_is_diods_arduino_connected;
  view m_view;
  DebugConsole* m_debug_console;
  QJsonObject m_etalons;
  QJsonObject m_pvd_calibr;
  QJsonObject m_json_config;
  QJsonArray m_power_tracker;
  QJsonArray m_pins_json_array;
  QVector<QSlider*> m_sliders;
  QVector<uint16_t> m_sliders_previous_values;
  QVector<QElapsedTimer> m_elapsed_timers;
  QVector<double> m_etalons_grid;
  QVector<int> m_short_pvd_grid_indexes;
  QHash<QString, int> lambdas_indexes;
  QSerialPort* m_serial_diods_controller;
  QSerialPort* m_serial_stm_spectrometr;
  QSerialPortInfo m_serial_port_info;
  QTimer m_timer_water_cooler_warning;
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

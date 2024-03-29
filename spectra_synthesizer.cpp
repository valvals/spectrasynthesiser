#include "spectra_synthesizer.h"
#include "ui_spectra_synthesizer.h"

#include "qjsonarray.h"
#include "qjsonobject.h"
#include "QVBoxLayout"
#include "QLabel"
#include "qslider.h"
#include "QDebug"
#include "QMessageBox"
#include "windows.h"
#include "QFile"
#include "QDir"
#include "QClipboard"
#include "limits"

#include "fitting/fitSpectr.h"
#include "json_utils.h"
#include "style_sheets.h"
#include "version.h"


const int mira_packet_size = 8;
const uint16_t expo_packet_size = 4;
const uint16_t spectr_packet_size = 7384;
const char power_dir[] = "diods_tracker";
const char tracker_full_path[] = "diods_tracker/diods_tracker.json";
const uchar packetBack[mira_packet_size] = {0xA5, 'b', 0, 0, 0, 0, 0x5A, 97};
const uchar packetForward[mira_packet_size] = {0xA5, 'f', 0, 0, 0, 0, 0x5A, 101};
const uchar packetGetPosition[mira_packet_size] = {0xA5, 'l', 0, 0, 0, 0, 0x5A, 107};
const uchar packetStop[mira_packet_size] = {0xA5, '_', 0, 0, 0, 0, 0x5A, 0};


SpectraSynthesizer::SpectraSynthesizer(QWidget* parent)
  : QMainWindow(parent)
  , ui(new Ui::SpectraSynthesizer) {
  QCoreApplication::instance()->installEventFilter(this);
  ui->setupUi(this);
  ui->spinBox_exposition->setVisible(false);
  m_isUpdateSpectrForFitter = new std::atomic<bool>();
  m_isSetValuesForSliders = new std::atomic<bool>();
  m_isUpdateSpectrForFitter->store(false);
  m_isSetValuesForSliders->store(false);
  m_shared_desired_sliders_positions = new QVector<double>();
  m_shared_spectral_data = new QVector<double>();
  m_voice_informator = new VoiceInformator(":/sounds");

  m_is_show_etalon = false;
  m_is_stm_spectr_update = true;
  m_is_stm_spectrometr_connected = false;
  m_is_diods_arduino_connected = false;
  m_camera_module = new CameraModule;
  m_player = new QMediaPlayer;
  connect(ui->action_show_camera, SIGNAL(triggered(bool)), m_camera_module, SLOT(mayBeShowCamera(bool)));
  connect(m_camera_module, &CameraModule::cameraWindowClosed, [this]() {ui->action_show_camera->setChecked(false);});

  jsn::getJsonObjectFromFile("etalons.json", m_etalons);
  loadEtalons();
  load_pvd_calibr();
  QDir dir;
  if (!dir.exists(power_dir)) {
    dir.mkdir(power_dir);
  }
  this->setWindowTitle(QString("СПЕКТРАСИНТЕЗАТОР %1").arg(VER_PRODUCTVERSION_STR));
  ui->comboBox_spectrometr_type->addItems({"ПВД", "ПИК"});
  QAction* copy_stm_spectr = new QAction;
  QAction* copy_etalon_spectr = new QAction;
  copy_stm_spectr->setText("копировать в буфер спектр");
  copy_etalon_spectr->setText("копировать в буфер эталон");
  ui->widget_plot->addAction(copy_stm_spectr);
  ui->widget_plot->addAction(copy_etalon_spectr);
  ui->widget_plot->setContextMenuPolicy(Qt::ActionsContextMenu);
  connect(copy_stm_spectr, SIGNAL(triggered()), SLOT(copy_spectr_to_clipboard()));
  connect(copy_etalon_spectr, SIGNAL(triggered()), SLOT(copy_etalon_to_clipboard()));
  ui->widget_plot->setBackground(QBrush(QColor(64, 66, 68)));
  ui->widget_plot->yAxis->setTickLabelColor(Qt::white);
  ui->widget_plot->xAxis->setTickLabelColor(Qt::white);
  ui->widget_plot->xAxis->setLabelColor(Qt::white);
  ui->widget_plot->yAxis->setLabelColor(Qt::white);
  ui->widget_plot->addGraph(); // 0 - спектрометр
  ui->widget_plot->addGraph(); // 1 - эталон
  ui->widget_plot->addGraph(); // 2 - sensor functions
  ui->widget_plot->addGraph(); // 3 - sensor functions normal
  ui->widget_plot->addGraph(); // 4 - combo for fitter
  m_power_stat_plot = new QCustomPlot;
  m_hours_stat_plot = new QCustomPlot;
  m_diod_models = new QCustomPlot;

  QPen graphPen(QColor(13, 160, 5));
  ui->widget_plot->graph(0)->setPen(graphPen);
  QPen graphPenEtalon(QColor(255, 255, 0));
  ui->widget_plot->graph(1)->setPen(graphPenEtalon);
  QPen graphPenApparatNormal(QColor(200, 200, 200));
  graphPenApparatNormal.setWidth(2);
  ui->widget_plot->graph(2)->setPen(graphPenApparatNormal);
  QPen graphPenCombo(QColor(167, 103, 245));
  graphPenCombo.setWidth(2);
  ui->widget_plot->graph(4)->setPen(graphPenCombo);

  m_serial_diods_controller = new QSerialPort;
  m_serial_stm_spectrometr = new QSerialPort;
  m_serial_mira = new QSerialPort;
  m_net_powers = new PowerSupplyManager;
  if (!jsn::getJsonObjectFromFile("config.json", m_json_config)) {
    //qDebug() << "Config file was not found on the disk...";
    jsn::getJsonObjectFromFile(":/config.json", m_json_config);
  };
  jsn::getJsonObjectFromFile("ir_lamps.json", m_ir_lamps);
  m_is_show_funny_video =  m_json_config.value("is_show_funny_video").toBool();
  m_is_show_light_model =  m_json_config.value("is_show_light_model").toBool();
  m_average_count_for_fitter = m_json_config.value("average_spectr_count_for_fitter").toInt();
  m_set_sliders_finalize_delay_ms = m_json_config.value("set_sliders_finalize_delay_ms").toInt();
  m_set_sliders_delay = m_json_config.value("set_sliders_interval_delay_ms").toInt();
  m_finite_derivative_step = m_json_config.value("fitting_finite_derivative_step").toDouble();
  m_pins_json_array = m_json_config.value("pins_array").toArray();
  m_is_first_previous_for_fitter = true;
  m_relax_filter_percent = m_json_config.value("relax_filter_percent").toDouble();
  m_slider_step_for_fitter = m_json_config.value("slider_step_for_fitter").toInt();
  m_ftol_for_fitter = m_json_config.value("ftol_for_fitter").toDouble();
  m_xtol_for_fitter = m_json_config.value("xtol_for_fitter").toDouble();
  m_gtol_for_fitter = m_json_config.value("gtol_for_fitter").toDouble();

  const QString serial_diods_number = m_json_config.value("serial_diods_controller_id").toString();
  const QString serial_stm_number = m_json_config.value("serial_stm_controller_id").toString();
  const QString mode = m_json_config.value("mode").toString();
  auto available_ports = m_serial_port_info.availablePorts();

  for (int i = 0; i < available_ports.size(); ++i) {
    //qDebug() << available_ports[i].serialNumber() << available_ports[i].portName();
    if (serial_diods_number == available_ports[i].serialNumber()) {
      m_serial_diods_controller->setPort(available_ports[i]);
      m_serial_diods_controller->open(QIODevice::ReadWrite);
      m_is_diods_arduino_connected = true;
      connect(m_serial_diods_controller, SIGNAL(readyRead()), this, SLOT(readDiodsData()));
    }
    if (serial_stm_number == available_ports[i].serialNumber()) {
      m_serial_stm_spectrometr->setPort(available_ports[i]);
      m_serial_stm_spectrometr->open(QIODevice::ReadWrite);
      connect(m_serial_stm_spectrometr, SIGNAL(readyRead()), this, SLOT(readStmData()));
      m_serial_stm_spectrometr->write("e5000\n");
      m_serial_stm_spectrometr->waitForBytesWritten(1000);
      m_is_stm_spectrometr_connected = true;
    }
    if (available_ports[i].description() == "USB-SERIAL CH340") {

      m_serial_mira->setBaudRate(9600);
      m_serial_mira->setPort(available_ports[i]);
      bool isMira = m_serial_mira->open(QIODevice::ReadWrite);
      std::ignore = isMira;
      //qDebug() << "is Mira: " << isMira;
    }

  }
  m_sliders_previous_values.resize(m_pins_json_array.size());
  m_prev_sliders_states.resize(m_pins_json_array.size());
  m_elapsed_timers.resize(m_pins_json_array.size());
  bool isInitialTrackerFileExists = QFile(tracker_full_path).exists();
  if (isInitialTrackerFileExists)
    jsn::getJsonArrayFromFile(tracker_full_path, m_power_tracker);
  if (m_is_diods_arduino_connected || mode == "developing") {

    for (int i = 0; i < m_pins_json_array.size(); ++i) {
      auto slider = new QSlider;
      slider->setCursor(Qt::PointingHandCursor);
      m_power_ticks.push_back(i + 1);
      m_power_labels.push_back(QString::number(i + 1));
      slider->setObjectName(QString("qslider_") + QString::number(i + 1));
      slider->setMinimumWidth(30);
      slider->setMinimumHeight(100);
      slider->setMaximumHeight(200);
      m_sliders_previous_values[i] = 1;
      QVBoxLayout* vbl = new QVBoxLayout;
      auto waveValue = m_pins_json_array[i].toObject().value("wave").toDouble();
      auto wave = QString::number(qCeil(waveValue));
      ui->comboBox_waves->addItem(wave);
      lambdas_indexes.insert(wave, i);
      auto label = new QLabel(wave);
      label->setMaximumHeight(50);
      vbl->addWidget(label);
      auto max_value = m_pins_json_array[i].toObject().value("max_value").toInt();

      if (!isInitialTrackerFileExists) {
        QJsonObject obj;
        obj.insert("time", 0);
        obj.insert("current", 0);
        obj.insert("name", m_pins_json_array[i].toObject().value("name").toString());
        obj.insert("wave", m_pins_json_array[i].toObject().value("wave").toString());
        for (int i = 0; i < 10; ++i) {
          obj.insert(QString(QString::number((i + 1) * 10)), 0);
        }
        m_power_tracker.append(obj);
      }

      slider->setMaximum(max_value);
      slider->setMinimum(1);
      vbl->addWidget(slider);
      auto color = m_pins_json_array[i].toObject().value("color").toString();
      slider->setStyleSheet(QString(styles::slider).arg(color, color));
      ui->horizontalLayout->addLayout(vbl);
      m_sliders.push_back(slider);
      connect(slider, &QSlider::sliderReleased, this, [i, slider, this]() {
        setTooltipForSlider(i, slider->value());
        sendDataToDiodsComDevice(QString("a%1_%2\n").arg(QString::number(i + 1), QString::number(slider->value())));
        ui->comboBox_waves->setCurrentIndex(i);
        ui->spinBox_bright_value->setValue(slider->value());
        savePowerParams(i, slider->value());
        updatePowerStat();
        if (m_is_show_light_model) {
          createLightModel();
        }
      }, Qt::DirectConnection);
    }
  } else {
    QMessageBox mb;
    mb.setIcon(QMessageBox::Warning);
    mb.setText("Устройство не подключено!");
    mb.exec();
  }
  m_debug_console = new DebugConsole;
  connect(ui->action_show_debug_console, SIGNAL(triggered()), m_debug_console, SLOT(show()));
  connect(&m_timer_water_cooler_warning, SIGNAL(timeout()), SLOT(changeWidgetState()));
  connect(ui->action_water_cooler_warning, &QAction::triggered, [this]() {
    if (ui->action_water_cooler_warning->isChecked()) {
      m_timer_water_cooler_warning.start(1000);
      sendDataToDiodsComDevice("m\n");
      m_voice_informator->playSound("warning_signal_switched_off.mp3");
    } else {
      sendDataToDiodsComDevice("u\n");
      m_timer_water_cooler_warning.stop();
      ui->label_info->setStyleSheet("background-color: rgb(31, 31, 31);color: rgb(0, 170, 0);");
      ui->label_info->setText("");
      m_voice_informator->playSound("warning_signal_switched_on.mp3");
    }
  });

  if (!isInitialTrackerFileExists) {
    jsn::saveJsonArrayToFile(tracker_full_path, m_power_tracker, QJsonDocument::Indented);
  }
  setupPowerStatPlot();
  updatePowerStat();
  connect(ui->action_show_power_stat, SIGNAL(triggered()), m_power_stat_plot, SLOT(show()));
  connect(ui->action_hours_stat, SIGNAL(triggered()), m_hours_stat_plot, SLOT(show()));
  connect(ui->action_copy_power_stat_to_buffer, SIGNAL(triggered()), SLOT(copyPowerStatToClipboard()));
  connect(ui->action_add_etalon, SIGNAL(triggered()), SLOT(createSamplesJson()));
  connect(ui->action_set_all_diods_to_zero, SIGNAL(triggered()), SLOT(reset_all_diods_to_zero()));
  connect(ui->action_hide_etalon, SIGNAL(triggered(bool)), SLOT(mayBeHideEtalon(bool)));
  connect(ui->action_etalon_pvd, SIGNAL(triggered()), SLOT(switchSpeyaEtalon_pvd()));
  connect(ui->action_azp_pvd, SIGNAL(triggered()), SLOT(switchAZP_pvd()));
  connect(ui->action_speya_pvd, SIGNAL(triggered()), SLOT(switchSpeya_pvd()));
  ui->action_etalon_pvd->setChecked(true);
  switchSpeyaEtalon_pvd();

  connect(ui->action_cycleMoveMira, SIGNAL(triggered()), SLOT(mayBeStartCycleMovingMira()));
  connect(m_serial_mira, SIGNAL(readyRead()), SLOT(readMiraAnswer()));
  prepareDiodModels();
  connect(ui->action_show_diod_models, SIGNAL(triggered()), m_diod_models, SLOT(show()));
  m_ormin_device = new OrminDevice(0);
  connect(m_ormin_device,
          SIGNAL(spectralDataRecieved(QVector<double>, double, double)),
          SLOT(recieveIrData(QVector<double>, double, double)));
  connect(ui->action_start_fitting_analytical, SIGNAL(triggered()), SLOT(fitSignalToEtalonALL_analytical()));
  connect(ui->action_fit_etalon_max_analytical, SIGNAL(triggered()), SLOT(fitSignalToEtalonMAX_analytical()));
  connect(ui->action_start_fitting_by_spectrometer, SIGNAL(triggered()), SLOT(fitSignalToEtalonALL_bySpectrometer()));
  connect(ui->action_fit_etalon_max_by_spectrometer, SIGNAL(triggered()), SLOT(fitSignalToEtalonMAX_bySpectrometer()));
  sendDataToDiodsComDevice("u\n");

  QVBoxLayout* m_ir_lamps_layout = new QVBoxLayout;
  //QHBoxLayout* hl
  auto ir_lamps = m_ir_lamps["lamps"].toArray();
  for (int i = 0; i < ir_lamps.size(); ++i) {
    auto slider = new QSlider;
    slider->setOrientation(Qt::Horizontal);

    slider->setMinimumHeight(50);
    slider->setStyleSheet(QString(styles::slider_ir));
    slider->setSingleStep(1);
    slider->setMaximum(100);
    m_ir_lamps_layout->addWidget(slider);
    connect(slider, &QSlider::sliderReleased, this, [i, slider, this]() {
      slider->setToolTip(QString("Ток (A) %1").arg(slider->value() / 10.0));
      std::ignore = i;
      std::ignore = this;
      //setTooltipForSlider(i, slider->value());
      //sendDataToDiodsComDevice(QString("a%1_%2\n").arg(QString::number(i + 1), QString::number(slider->value())));
      //ui->comboBox_waves->setCurrentIndex(i);
      //ui->spinBox_bright_value->setValue(slider->value());
      //savePowerParams(i, slider->value());
      //updatePowerStat();
      //if(m_is_show_light_model){
      //createLightModel();
      //}
    }, Qt::DirectConnection);

  }
  ui->verticalLayout_ir_sliders->addLayout(m_ir_lamps_layout);
  ui->widget_ir_sliders->setVisible(false);
  // DRAFT this code will be used to update calibr_lists

  /*
  QDir dir_calibrs("calibrs");
  QStringList file_names = dir_calibrs.entryList(QDir::NoDotAndDotDot | QDir::Files);
  //qDebug()<<file_names;
  QJsonArray pvd_clibrs;

  for(int i=0;i<file_names.size();++i){
  auto fn = file_names[i];
  fn.remove(".txt");
  QString expo = QString::number(fn.toInt());
  qDebug()<<expo;
  QJsonObject obj;
  obj["expo"] = expo;
  QFile file("calibrs/"+file_names[i]);
  file.open(QIODevice::ReadOnly);
  QString line;
  QJsonArray jarr_waves;
  QJsonArray jarr_values;
  QTextStream ts(&file);
  while(ts.readLineInto(&line)){
  QStringList values = line.split("\t");
  if(values.size()==2){
  jarr_waves.append(values[0].toDouble());
  jarr_values.append(values[1].toDouble());
  }
  }
  obj["waves"] = jarr_waves;
  obj["values"] = jarr_values;
  pvd_clibrs.append(obj);
  }

  db_json::saveJsonArrayToFile("pvd_calibr_list.json",pvd_clibrs,QJsonDocument::Indented);
  */
  findApparatMaximus();
  /*m_net_powers->getVoltage(0);
  m_net_powers->getVoltage(1);
  m_net_powers->getVoltage(2);
  m_net_powers->getVoltage(3);

  m_net_powers->setCurrentLimit(0,10);
  m_net_powers->setCurrentLimit(1,10);
  m_net_powers->setCurrentLimit(2,10);
  m_net_powers->setCurrentLimit(3,10);

  m_net_powers->getCurrentLimit(0);
  m_net_powers->getCurrentLimit(1);
  m_net_powers->getCurrentLimit(2);
  m_net_powers->getCurrentLimit(3);*/
  //m_net_powers->switchOnAllUnits();
  //m_net_powers->switchOffAllUnits();
}

SpectraSynthesizer::~SpectraSynthesizer() {
  delete ui;
}

void SpectraSynthesizer::readDiodsData() {
  static QByteArray buffer;
  const QByteArray data = m_serial_diods_controller->readAll();
  buffer.append(data);
  if (data[data.size() - 1] == '\n') {
    m_debug_console->add_message("recieved from diods controller: " + QString(buffer), dbg::DIODS_CONTROLLER);
    buffer.clear();
  }
}

void SpectraSynthesizer::finishFitting() {
  ui->widget_video->hide();
  m_player->stop();
  this->setEnabled(true);
  ui->label_info->setText("");
  m_is_first_previous_for_fitter = true;
  m_voice_informator->playSound("fitting_by_spectrometr_finished_.mp3");
}

void SpectraSynthesizer::readStmData() {

  static double prev_azp_max = 0;
  const double azp_delta_max = 100.0;
  static double prev_speya_max = 0;
  const double speya_delta_max = 2e7;
  const double TOP_MARGIN_COEFF = 1.1;

  if (ui->comboBox_spectrometr_type->currentText() != "ПВД") {
    m_serial_stm_spectrometr->readAll();
    return;
  }

  if (m_serial_stm_spectrometr->bytesAvailable() == expo_packet_size) {
    auto expo = m_serial_stm_spectrometr->readAll();
    m_is_stm_exposition_changed = false;
    if (m_is_stm_spectr_update) {
      update_stm_spectr();
    }
    m_debug_console->add_message("expo packet recieved from stm: " +
                                 QString::number(expo.toInt()) + "\n", dbg::STM_CONTROLLER);
    return;
  } else if (m_serial_stm_spectrometr->bytesAvailable() != spectr_packet_size) {
    //qDebug() << "spectr packet recieved" << m_serial_stm_spectrometr->bytesAvailable();
    if (m_serial_stm_spectrometr->bytesAvailable() > spectr_packet_size) {
      qDebug() << "BAD CASE ----> data will never be processing";
      m_serial_stm_spectrometr->readAll();
      update_stm_spectr();
    }
    return;
  }
  auto ba = m_serial_stm_spectrometr->readAll();
  if (!m_is_stm_spectr_update) {
    return;
  }
  SpectrumData spectrumData;
  memcpy(&spectrumData, ba, sizeof(spectrumData));
  QVector<double> values;
  QVector<double> channels;
  double max = 0;
  double average_black = 0.0;
  int black_sum = 0;
  int black_array_size = sizeof(spectrumData.black1);
  for (int i = 0; i < black_array_size; ++i) {
    black_sum += spectrumData.black1[i];
  }
  average_black = (double)black_sum / (double)black_array_size;
  switch (m_view) {
    case view::PVD_AZP:
      // PVD_AZP case
      prev_speya_max = 0;
      for (size_t i = 0; i < spectr_values_size; ++i) {
        channels.push_back(i + 1);
        values.push_back(spectrumData.spectrum[i] - average_black);
        if (max < spectrumData.spectrum[i])
          max = spectrumData.spectrum[i];
      };
      if (max < azp_delta_max) {
        prev_azp_max = max;
      }
      if (qAbs(max - prev_azp_max) > azp_delta_max) {
        prev_azp_max = max;
      } else {
        max = prev_azp_max;
      }
      max = max * TOP_MARGIN_COEFF;
      break;
    case view::PVD_SPEYA:
      // PVD_SPEYA case
      prev_azp_max = 0;
      for (int i = 0; i < spectr_values_size; ++i) {
        auto wave = m_pvd_calibr["waves"].toArray()[i].toDouble();
        if (wave < 400)
          continue;
        auto value = m_pvd_calibr["values"].toArray()[i].toDouble() * (spectrumData.spectrum[i] - average_black);
        channels.push_back(wave);
        values.push_back(value);
        if (max < value) {
          max = value;
        }
        if (wave >= 900.0) {
          if (qAbs(max - prev_speya_max) > speya_delta_max) {
            prev_speya_max = max;
          } else {
            max = prev_speya_max * TOP_MARGIN_COEFF;
          }
          break;
        }
      };
      break;

    case view::ETALON_PVD:
      for (int i = 0; i < m_short_pvd_grid_indexes.size(); ++i) {
        auto index = m_short_pvd_grid_indexes[i];
        auto wave = m_pvd_calibr["waves"].toArray()[index].toDouble();
        auto value = m_pvd_calibr["values"].toArray()[index].toDouble() * (spectrumData.spectrum[index] - average_black);
        channels.push_back(wave);
        values.push_back(value);
      };
      max = m_etalons_maximums[ui->comboBox_etalons->currentText()] * TOP_MARGIN_COEFF;
      break;
  }
  static quint8 fitter_counter = 0;
  static QVector<double> average(values.size(), 0);
  static QVector<double> prev_average = average;
  static QVector<double> combo(average.size(), 0);
  static bool is_smart_range_mode = m_json_config["is_combinated_range_spectr_for_fitter"].toBool();
  if (!m_is_stm_exposition_changed) {
    if (m_isUpdateSpectrForFitter->load()) {

      ++fitter_counter;
      for (int i = 0; i < average.size(); ++i) {
        average[i] += values[i] / m_average_count_for_fitter;
      }
      if (fitter_counter == m_average_count_for_fitter) {

        if (m_is_first_previous_for_fitter) {
          *m_shared_spectral_data = average;
          m_is_first_previous_for_fitter = false;
        } else {
          // Implement spectr combinator
          if (is_smart_range_mode) {
            combinateSpectralData(average, prev_average, combo);
          } else {
            combo = average;
          }
          *m_shared_spectral_data = combo;
          showComboGraph(channels, combo);
        }
        prev_average = combo;
        fitter_counter = 0;
        if (is_smart_range_mode) {
          QVector<double> difference(average.size());
          for (int j = 0; j < average.size(); ++j) {
            difference[j] = average[j] - prev_average[j];
          }
          //qDebug() << "difference: ---> " << difference;
        }
        average.fill(0);
        m_isUpdateSpectrForFitter->store(false);
        m_fitter->isBlocked.store(false);
      }
    }
    if (m_isSetValuesForSliders->load()) {
      //m_prev_sliders_states
      Q_ASSERT(m_prev_sliders_states.size() == m_sliders.size());
      for (int i = 0; i < m_prev_sliders_states.size(); ++i) {
        m_prev_sliders_states[i] = m_sliders[i]->value();
      }
      setValuesForSlidersBlocked(*m_shared_desired_sliders_positions, m_prev_sliders_states);
      m_isSetValuesForSliders->store(false);
      m_fitter->isBlocked = false;
    }
    show_stm_spectr(channels, values, max);
  } else {
    auto expo_value = ui->comboBox_expositions->currentText().toDouble();
    auto expo_command = QString("e%1\n").arg(expo_value * 1000);
    m_serial_stm_spectrometr->write(expo_command.toLatin1());
    m_serial_stm_spectrometr->waitForBytesWritten(1000);
  }
}

void SpectraSynthesizer::sendDataToDiodsComDevice(const QString& command) {
  m_debug_console->add_message("send to diods controller: " + command.toLatin1(), dbg::DIODS_CONTROLLER);
  if (m_serial_diods_controller->isOpen()) {
    m_serial_diods_controller->write(command.toLatin1());
    Sleep(50);
  }
}

void SpectraSynthesizer::setTooltipForSlider(const int& index, const int& value) {
  QString waveStr = QString::number(m_pins_json_array[index].toObject().value("wave").toDouble());
  QString valueStr = (QString::number(value));
  m_sliders[index]->setToolTip(QString(styles::tooltip).arg(waveStr, valueStr));
}

QString SpectraSynthesizer::getGroupID(const double& value) {
  Q_ASSERT(value > 0 || value < 100);
  if (value >= 0 && value <= 10)
    return "10";
  if (value > 10 && value <= 20)
    return "20";
  if (value > 20 && value <= 30)
    return "30";
  if (value > 30 && value <= 40)
    return "40";
  if (value > 40 && value <= 50)
    return "50";
  if (value > 50 && value <= 60)
    return "60";
  if (value > 60 && value <= 70)
    return "70";
  if (value > 70 && value <= 80)
    return "80";
  if (value > 80 && value <= 90)
    return "90";
  if (value > 90 && value <= 100)
    return "100";
  return "";
}

void SpectraSynthesizer::setupPowerStatPlot() {
  QLinearGradient gradient(0, 0, 0, 400);
  gradient.setColorAt(0, QColor(90, 90, 90));
  gradient.setColorAt(0.38, QColor(105, 105, 105));
  gradient.setColorAt(1, QColor(70, 70, 70));
  m_power_stat_plot->setBackground(QBrush(gradient));
  m_hours_stat_plot->setBackground(QBrush(gradient));

  auto min_size = QSize(900, 600);
  m_power_stat_plot->setMinimumSize(min_size);
  m_hours_stat_plot->setMinimumSize(min_size);
  QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
  textTicker->addTicks(m_power_ticks, m_power_labels);
  m_power_stat_plot->xAxis->setTicker(textTicker);
  m_hours_stat_plot->xAxis->setTicker(textTicker);
  m_power_stat_plot->yAxis->setPadding(5);
  m_hours_stat_plot->yAxis->setPadding(5);
  m_power_stat_plot->yAxis->setLabel("Ampere * hour");
  m_hours_stat_plot->yAxis->setLabel("Hours");
  m_power_stat_plot->yAxis->setBasePen(QPen(Qt::white));
  m_hours_stat_plot->yAxis->setBasePen(QPen(Qt::white));
  m_power_stat_plot->yAxis->setTickPen(QPen(Qt::white));
  m_hours_stat_plot->yAxis->setTickPen(QPen(Qt::white));
  m_power_stat_plot->yAxis->setTickLabelColor(Qt::white);
  m_hours_stat_plot->yAxis->setTickLabelColor(Qt::white);
  m_power_stat_plot->yAxis->setLabelColor(Qt::white);
  m_hours_stat_plot->yAxis->setLabelColor(Qt::white);
  m_power_stat_plot->xAxis->setBasePen(QPen(Qt::white));
  m_hours_stat_plot->xAxis->setBasePen(QPen(Qt::white));
  m_power_stat_plot->xAxis->setTickPen(QPen(Qt::white));
  m_hours_stat_plot->xAxis->setTickPen(QPen(Qt::white));
  m_power_stat_plot->xAxis->setTickLabelColor(Qt::white);
  m_hours_stat_plot->xAxis->setTickLabelColor(Qt::white);
  m_power_stat_plot->xAxis->setLabelColor(Qt::white);
  m_hours_stat_plot->xAxis->setLabelColor(Qt::white);
  m_power_stat_plot->xAxis->setLabel("Diod number");
  m_hours_stat_plot->xAxis->setLabel("Diod number");
  m_power_stat_plot->legend->setVisible(true);
  m_power_stat_plot->plotLayout()->addElement(0, 1, m_power_stat_plot->legend);
  m_power_stat_plot->plotLayout()->setColumnStretchFactor(1, 0.005);
  for (int i = 0; i < 10; ++i) {
    auto bar = new QCPBars(m_power_stat_plot->xAxis, m_power_stat_plot->yAxis);
    bar->setName(QString(QString::number((i + 1) * 10)) + " %");
    m_power_bars.push_back(bar);
  }
  m_hours_bar = new QCPBars(m_hours_stat_plot->xAxis, m_hours_stat_plot->yAxis);
  m_power_stat_plot->xAxis->setRange(0, 31);
  m_hours_stat_plot->xAxis->setRange(0, 31);
  //m_test_plot->xAxis->setTickLabelRotation(70);

  QVector<QColor>colors = {{71, 107, 76},
    {37, 200, 56},
    {8, 246, 186},
    {227, 255, 0},
    {255, 163, 0},
    {255, 100, 0},
    {180, 40, 0},
    {148, 46, 179},
    {230, 40, 165},
    {255, 0, 0}
  };
  for (int i = 0; i < m_power_bars.size(); ++i) {
    if (i != m_power_bars.size() - 1) {
      m_power_bars[i + 1]->moveAbove(m_power_bars[i]);
    }
    m_power_bars[i]->setStackingGap(1);
    m_power_bars[i]->setAntialiased(false);
    m_power_bars[i]->setBrush(colors[i]);
    m_power_bars[i]->setPen(QPen(QColor(3, 252, 240).lighter(150)));
  }
  m_hours_bar->setStackingGap(1);
  m_hours_bar->setAntialiased(false);
  m_hours_bar->setBrush(colors[0]);
  m_hours_bar->setPen(QPen(QColor(3, 252, 240).lighter(150)));

}

void SpectraSynthesizer::savePowerParams(const int& index, const int& value) {
  auto prev_value = m_sliders_previous_values[index];
  auto max_value = m_pins_json_array[index].toObject().value("max_value").toInt();
  auto max_current = m_pins_json_array[index].toObject().value("max_current").toDouble();
  auto x_current = (double)(max_current * prev_value) / max_value;
  auto group = ((double)x_current / max_current) * 100;
  //qDebug() << "group: " << group << x_current;
  QString groupID = getGroupID(group);
  m_sliders_previous_values[index] = value;
  if (prev_value == 1) {
    m_elapsed_timers[index].restart();
  } else {
    auto prev_object = m_power_tracker[index].toObject();
    double hours = (double)m_elapsed_timers[index].restart() / 1000.0 / 60.0 / 60.0;
    auto prev_time_value = prev_object["time"].toDouble();
    auto prev_current_value = prev_object["current"].toDouble();
    auto prev_group_value = prev_object[groupID].toDouble();
    double ampers_hours = x_current * hours;
    prev_object["time"] = prev_time_value + hours;
    prev_object["current"] = ampers_hours + prev_current_value;
    prev_object[groupID] = ampers_hours + prev_group_value;
    m_power_tracker[index] = prev_object;
    jsn::saveJsonArrayToFile(tracker_full_path, m_power_tracker, QJsonDocument::Indented);
  }
}

void SpectraSynthesizer::createSamplesJson() {
  QFileDialog dialog(this);
  dialog.setFileMode(QFileDialog::ExistingFiles);
  dialog.setNameFilter("*.txt");
  QStringList files = dialog.getOpenFileNames(this, "Select spectral samples", "", "*.txt");
  if (files.size() == 0)
    return;
  QJsonObject root;
  jsn::getJsonObjectFromFile("etalons.json", root);
  QJsonArray objects = root["_objects"].toArray();
  QString etalon_name;
  for (int i = 0; i < files.size(); ++i) {
    QFile file(files[i]);
    etalon_name = file.fileName().split('/').last().split('.')[0];
    if (objects.contains(etalon_name)) {
      qDebug() << "ignore this file...";
      continue;
    }
    objects.push_back(etalon_name);
    file.open(QIODevice::ReadOnly);
    QJsonArray values;
    QTextStream qts(&file);
    QString line;
    while (qts.readLineInto(&line)) {
      auto var = line.split("\t");
      if (var.size() > 1) {
        values.push_back(var[1].toDouble());
      } else {
        values.push_back(var[0].toDouble());
      }
    }
    file.close();
    root.insert(etalon_name, values);
  }
  root["_objects"] = objects;
  jsn::saveJsonObjectToFile("etalons.json", root, QJsonDocument::Indented);
  m_etalons = root;
  m_etalons_grid.clear();
  ui->comboBox_etalons->clear();
  loadEtalons();
}

void SpectraSynthesizer::loadEtalons() {
  auto objects = m_etalons["_objects"].toArray();
  auto grid = m_etalons["_grid"].toArray();
  for (int i = 0; i < grid.size(); ++i) {
    m_etalons_grid.push_back(grid[i].toDouble());
  }
  for (int i = 0; i < objects.size(); ++i) {
    const QString object_name = objects[i].toString();
    ui->comboBox_etalons->addItem(object_name);
    auto etalon = m_etalons[object_name].toArray();
    double max = 0;
    for (int j = 0; j < etalon.size(); ++j) {
      if (etalon[j].toDouble() > max) {
        max =  etalon[j].toDouble();
      }
    }
    m_etalons_maximums.insert(object_name, max);
    //qDebug() << object_name << max;
  }
  showCurrentEtalon();
}



void SpectraSynthesizer::showCurrentEtalon() {
  if (!m_is_show_etalon)
    return;
  QVector<double>current_etalon;
  auto sample = m_etalons[ui->comboBox_etalons->currentText()].toArray();
  double max = 0;
  for (int i = 0; i < sample.size(); ++i) {
    double value = sample[i].toDouble();
    if (max < value)
      max = value;
    current_etalon.push_back(value);
  }
  ui->widget_plot->graph(1)->setData(m_etalons_grid, current_etalon);
  ui->widget_plot->xAxis->setRange(400, 900);
  ui->widget_plot->yAxis->setRange(0, max * 1.1);
  ui->widget_plot->replot();
}

void SpectraSynthesizer::on_comboBox_etalons_currentIndexChanged(const QString& arg1) {
  Q_UNUSED(arg1)
  showCurrentEtalon();
}

void SpectraSynthesizer::mayBeHideEtalon(bool isHide) {
  if (isHide) {
    ui->widget_plot->graph(1)->setData({}, {});
    ui->widget_plot->replot();
    m_is_show_etalon = false;
  } else {
    m_is_show_etalon = true;
    showCurrentEtalon();
  }
}

void SpectraSynthesizer::copyPowerStatToClipboard() {
  QString copy_stat;

  for (int i = 0; i < m_power_tracker.size(); ++i) {
    auto object = m_power_tracker[i].toObject();
    copy_stat.append(object["name"].toString());
    copy_stat.append("\t");
    copy_stat.append(object["wave"].toString());
    copy_stat.append("\t");
    copy_stat.append(QString::number(object["time"].toDouble()));
    copy_stat.append("\t");
    copy_stat.append(QString::number(object["current"].toDouble()));
    copy_stat.append("\t");
    for (int j = 0; j < 10; ++j) {
      copy_stat.append(QString::number(object[QString::number((j + 1) * 10)].toDouble()));
      if (j != 9)
        copy_stat.append("\t");
    }
    copy_stat.append("\n");
  }
  auto clip = QGuiApplication::clipboard();
  clip->setText(copy_stat);

}

void SpectraSynthesizer::updatePowerStat() {
  double max = 0.0;
  for (int i = 0; i < m_power_bars.size(); ++i) {
    QVector<double>data;
    for (int j = 0; j < m_pins_json_array.size(); ++j) {
      data.push_back(m_power_tracker[j].toObject().value(QString::number((i + 1) * 10)).toDouble());
      auto current = m_power_tracker[j].toObject().value("current").toDouble();
      if (max < current)
        max = current;
    }
    m_power_bars[i]->setData(m_power_ticks, data);
  }
  m_power_stat_plot->yAxis->setRange(0, max);
  m_power_stat_plot->replot();

  max = 0.0;
  QVector<double>hours;
  for (int j = 0; j < m_pins_json_array.size(); ++j) {

    auto time = m_power_tracker[j].toObject().value("time").toDouble();
    hours.push_back(time);
    if (max < time)
      max = time;
  }
  m_hours_bar->setData(m_power_ticks, hours);
  m_hours_stat_plot->yAxis->setRange(0, max);
  m_hours_stat_plot->replot();
}

void SpectraSynthesizer::closeEvent(QCloseEvent* event) {
  event->ignore();
  sendDataToDiodsComDevice("u\n");
  reset_all_diods_to_zero();
  m_player->stop();
  event->accept();
}

bool SpectraSynthesizer::eventFilter(QObject* watched, QEvent* event) {

  if (watched->objectName().contains("qslider")) {
    if (m_view == view::ETALON_PVD) {
      if (event->type() == QEvent::HoverEnter || event->type() == QEvent::HoverLeave) {
        if (event->type() == QEvent::HoverEnter) {
          auto objName = watched->objectName();
          auto parts = objName.split("_");
          Q_ASSERT(parts.size() == 2);
          objName = parts[1];
          auto arr = m_json_config["pins_array"].toArray();
          auto model = arr[objName.toInt() - 1].toObject()["model"].toObject();
          QPen graphPenApparatFunction(QColor(arr[objName.toInt() - 1].toObject()["color"].toString()));
          graphPenApparatFunction.setWidth(3);
          ui->widget_plot->graph(3)->setPen(graphPenApparatFunction);
          auto values = model["values"].toArray();
          auto waves = model["waves"].toArray();
          QVector<double> vals;
          QVector<double> wavs;
          auto apparat_maxs = m_apparat_maximus[watched->objectName()];
          auto et_max = m_etalons_maximums.value(ui->comboBox_etalons->currentText());
          for (int i = 0; i < values.size(); ++i) {
            auto val = (et_max / apparat_maxs.second) * values.at(i).toDouble();
            vals.push_back(val);
            wavs.push_back(waves.at(i).toDouble());
          }
          ui->widget_plot->graph(3)->setData(wavs, vals);
          ui->widget_plot->graph(2)->setData({apparat_maxs.first, apparat_maxs.first}, {0, et_max});

        } else {
          ui->widget_plot->graph(2)->setData({}, {});
          ui->widget_plot->graph(3)->setData({}, {});
        }
      }
      ui->widget_plot->replot();
    }
  }
  return false;
}


void SpectraSynthesizer::reset_all_diods_to_zero() {

  sendDataToDiodsComDevice("f\n");
  for (int i = 0; i < m_sliders.size(); ++i) {
    if (m_sliders[i]->value() > 1) {
      savePowerParams(i, 1);
    }
    m_sliders[i]->setValue(1);
    setTooltipForSlider(i, 1);

  }
  ui->spinBox_bright_value->setValue(1);
  updatePowerStat();
}

void SpectraSynthesizer::on_pushButton_apply_clicked() {
  auto index = lambdas_indexes.value(ui->comboBox_waves->currentText());
  auto value = ui->spinBox_bright_value->value();
  m_sliders[index]->setValue(value);
  setTooltipForSlider(index, value);
  sendDataToDiodsComDevice(QString("a%1_%2\n").arg(QString::number(index + 1), QString::number(value)));
}

void SpectraSynthesizer::on_comboBox_waves_currentTextChanged(const QString& arg1) {
  auto index = lambdas_indexes.value(arg1);
  auto max = m_pins_json_array[index].toObject().value("max_value").toInt();
  ui->spinBox_bright_value->setMaximum(max);
  ui->label_value->setToolTip(QString("макс: %1").arg(QString::number(max)));
}

void SpectraSynthesizer::show_stm_spectr(QVector<double> channels,
                                         QVector<double> data,
                                         double max) {
  ui->widget_plot->graph(0)->setData(channels, data);
  ui->widget_plot->xAxis->setRange(channels[0], channels[channels.size() - 1]);
  ui->widget_plot->yAxis->setRange(0, max);
  ui->widget_plot->replot();
  if (ui->comboBox_spectrometr_type->currentText() == "ПВД") {
    QTimer::singleShot(100, this, SLOT(update_stm_spectr()));
  } else {
    QTimer::singleShot(100, m_ormin_device, SIGNAL(requestSpectr()));
  }
}

void SpectraSynthesizer::changeWidgetState() {
  static bool state;
  if (state) {
    ui->label_info->setStyleSheet("background-color:rgb(230, 31, 31);color:rgb(255,255,255)");
    ui->label_info->setText("Предупреждающий сигнал отключен!");
    state = false;
  } else {
    ui->label_info->setStyleSheet("background-color:rgb(31, 31, 31)");
    state = true;
  }
  ui->label_info->repaint();
  ui->label_info->update();
}

void SpectraSynthesizer::update_stm_spectr() {
  if (!m_is_stm_spectrometr_connected)
    return;
  m_serial_stm_spectrometr->write("r\n");
  m_serial_stm_spectrometr->waitForBytesWritten(1000);
  Sleep(50);
}

void SpectraSynthesizer::copy_spectr_to_clipboard() {
  copy_data_plot_to_clipboard(ui->widget_plot->graph(0)->data());
}

void SpectraSynthesizer::copy_etalon_to_clipboard() {
  copy_data_plot_to_clipboard(ui->widget_plot->graph(1)->data());
}

void SpectraSynthesizer::copy_data_plot_to_clipboard(QSharedPointer<QCPGraphDataContainer> data) {
  auto size = data->size();
  QString values;
  for (int i = 0; i < size; i++) {
    values.append(QString::number(data->at(i)->value));
    values.append("\n");
  }
  QClipboard* clipboard = QGuiApplication::clipboard();
  clipboard->setText(values);
}

void SpectraSynthesizer::load_pvd_calibr() {

  QJsonObject jo;
  QJsonArray arr;
  jsn::getJsonObjectFromFile("pvd_calibr_list.json", jo);
  arr = jo["calibrs"].toArray();
  Q_ASSERT(arr.size() > 0);
  m_pvd_calibr = arr[0].toObject();
  m_pvd_calibr["waves"] = jo["waves"].toArray();
  auto wave_array = jo["waves"].toArray();
  auto bright_array = m_pvd_calibr["values"].toArray();
  int counter = 0;
  int wave_start = 400;

  for (int i = 0; i < arr.size(); ++i) {
    auto temp = arr[i].toObject();
    auto bright_array = temp["values"].toArray();
    //qDebug() << i << bright_array.size();
    Q_ASSERT(bright_array.size() == spectr_values_size);
  }
  for (int i = 1; i < wave_array.size(); ++i) {
    if (m_etalons_grid[counter] > 900) {
      break;
    }
    auto new_delta = m_etalons_grid[counter] - wave_array[i].toDouble();
    if (new_delta > 0) {
      continue;
    } else {
      ++counter;
      m_short_pvd_grid_indexes.push_back(i);
      m_short_grid_lambda_to_real_indexes.insert(wave_start, i); // REFACTOR !!!!!!
      ++wave_start;
    }
  }

  for (int i = 0; i < arr.size(); ++i) {
    ui->comboBox_expositions->addItem(arr[i].toObject()["expo"].toString());
  }
}

void SpectraSynthesizer::switchAZP_pvd() {
  //qDebug() << "AЦП режим.....";
  m_view = view::PVD_AZP;
  ui->action_speya_pvd->setChecked(false);
  ui->action_etalon_pvd->setChecked(false);
  mayBeHideEtalon(true);
  ui->action_hide_etalon->setChecked(true);
  ui->widget_plot->yAxis->setLabel("отсчёты АЦП");
  ui->widget_plot->xAxis->setLabel("номер канала");
  ui->widget_plot->replot();
}

void SpectraSynthesizer::switchSpeya_pvd() {
  m_view = view::PVD_SPEYA;
  ui->action_etalon_pvd->setChecked(false);
  ui->action_azp_pvd->setChecked(false);
  ui->widget_plot->yAxis->setLabel("СПЭЯ (Вт/(м3 * ср))");
  ui->widget_plot->xAxis->setLabel("длина волны (нм)");
  ui->widget_plot->replot();
}

void SpectraSynthesizer::switchSpeyaEtalon_pvd() {
  m_view = view::ETALON_PVD;
  ui->action_azp_pvd->setChecked(false);
  ui->action_speya_pvd->setChecked(false);
  mayBeHideEtalon(false);
  ui->action_hide_etalon->setChecked(false);
  ui->widget_plot->yAxis->setLabel("СПЭЯ (Вт/(м3 * ср))");
  ui->widget_plot->xAxis->setLabel("длина волны (нм)");
  ui->widget_plot->replot();
}

void SpectraSynthesizer::on_spinBox_exposition_valueChanged(int arg1) {
  Q_UNUSED(arg1)
  if (!m_is_stm_spectrometr_connected)
    return;
  m_is_stm_exposition_changed = true;

}

void SpectraSynthesizer::on_pushButton_stop_start_update_stm_spectr_toggled(bool checked) {
  m_is_stm_spectr_update = checked;
  if (m_is_stm_spectr_update) {
    ui->pushButton_stop_start_update_stm_spectr->setText("пауза");
    update_stm_spectr();
  } else {
    ui->pushButton_stop_start_update_stm_spectr->setText("обновлять");
  }
}

uchar SpectraSynthesizer::getCS(const uchar* data, int size) {
  uchar CS = 0;
  for (int i = 0; i < size - 1; ++i) {
    CS += data[i];
  }
  return CS;
}

void SpectraSynthesizer::sendDataToMiraComDevice(const uchar* packet, int size) {
  QByteArray ba;
  for (int i = 0; i < size; ++i) {
    ba.append(static_cast<char>(packet[i]));
  }
  m_serial_mira->write(ba);
  m_serial_mira->waitForBytesWritten();
}

void SpectraSynthesizer::readMiraAnswer() {
  auto resp = m_serial_mira->readAll();
  Q_ASSERT(resp.size() >= 8);
  if (resp[1] == 'l') {
    QByteArray value;
    value.append(resp[2]);
    byte flag = value[0];
    if (ui->action_cycleMoveMira->isChecked()) {
      if (flag == 2) {
        sendDataToMiraComDevice(packetForward, mira_packet_size);
      } else if (flag == 1) {
        sendDataToMiraComDevice(packetForward, mira_packet_size);
      } else if (flag == 0) {
        sendDataToMiraComDevice(packetBack, mira_packet_size);
      } else if (flag == 3) {
        // TODO warning Switches were broken
      }
    }
  }
}

void SpectraSynthesizer::recieveIrData(QVector<double> sumSpectr,
                                       double maxValue,
                                       double minValue) {
  //qDebug() << sumSpectr.size();
  Q_UNUSED(minValue);
  QVector<double>channels;
  for (int i = 0; i < sumSpectr.size(); ++i) {
    channels.push_back(i);
  }
  show_stm_spectr(channels, sumSpectr, maxValue);
}

void SpectraSynthesizer::mayBeStartCycleMovingMira() {
  auto isMiraChecked = ui->action_cycleMoveMira->isChecked();
  //qDebug() << "Mira is checked: " << isMiraChecked;
  if (!m_serial_mira->isOpen()) {
    return;
  }
  if (!isMiraChecked) {
    sendDataToMiraComDevice(packetStop, mira_packet_size);
    return;
  }
  sendDataToMiraComDevice(packetGetPosition, mira_packet_size);
}


void SpectraSynthesizer::on_comboBox_spectrometr_type_currentIndexChanged(const QString& arg1) {

  static bool isFirstRun = true;
  if (isFirstRun) {
    isFirstRun = false;
    return;
  }
  if (arg1 == "ПВД") {
    m_voice_informator->playSound("pvd_sensor.mp3");
    ui->widget_visual_range_sliders->show();
    ui->widget_ir_sliders->hide();
    update_stm_spectr();
  } else {
    ui->widget_ir_sliders->show();
    ui->widget_visual_range_sliders->hide();
    m_voice_informator->playSound("pik_sensor.mp3");
    emit m_ormin_device->requestSpectr();
  }

}

void SpectraSynthesizer::prepareDiodModels() {
  m_diod_models->setMinimumSize(QSize(500, 500));
  QLinearGradient gradient(0, 0, 0, 400);
  gradient.setColorAt(0, QColor(90, 90, 90));
  gradient.setColorAt(0.38, QColor(105, 105, 105));
  gradient.setColorAt(1, QColor(70, 70, 70));
  m_diod_models->setBackground(QBrush(gradient));
  //m_diod_models->setBackground(QBrush(QColor(64, 66, 68)));
  auto arr = m_json_config["pins_array"].toArray();
  double max = 0;
  double wave_start = MAXUINT;
  double wave_end = 0;
  for (int i = 0; i < arr.size(); ++i) {
    m_diod_models->addGraph();
    auto model = arr[i].toObject()["model"].toObject();
    auto values = model["values"].toArray();
    auto waves = model["waves"].toArray();
    auto color = QColor(arr[i].toObject()["color"].toString());
    QPen graphPen(color);
    graphPen.setWidth(2);
    m_diod_models->graph(i)->setPen(graphPen);
    QVector<double>d_values;
    QVector<double>d_waves;
    Q_ASSERT(values.size() == waves.size());
    for (int j = 0; j < values.size(); ++j) {
      auto value = values[j].toDouble();
      if (max < value) {
        max = value;
      }
      d_values.push_back(value);
      auto wave = waves[j].toDouble();
      if (wave_start > wave && wave != 0) {
        wave_start = wave;
      }
      if (wave_end < wave) {
        wave_end = wave;
      }
      d_waves.push_back(wave);
    }

    m_diod_models->graph(i)->setData(d_waves, d_values);
  }
  //qDebug() << wave_start << wave_end;
  m_diod_models->xAxis->setRange(200, 1200);//200 - 1200 looks better then wave_start wave_end
  m_diod_models->yAxis->setRange(0, max);
  m_diod_models->yAxis->setLabel("СПЭЯ (Вт/(м3 * ср))");
  m_diod_models->xAxis->setLabel("длина волны (нм)");
  m_diod_models->yAxis->setTickLabelColor(Qt::white);
  m_diod_models->xAxis->setTickLabelColor(Qt::white);
  m_diod_models->xAxis->setLabelColor(Qt::white);
  m_diod_models->yAxis->setLabelColor(Qt::white);
}

// Fitting module

void SpectraSynthesizer::fitSignalToEtalonALL_analytical() {
  fitSignalToEtalon_analytical(FitSettings::FIT_ALL);
}

void SpectraSynthesizer::fitSignalToEtalonMAX_analytical() {
  fitSignalToEtalon_analytical(FitSettings::FIT_BY_MAXIMUMS);
}

void SpectraSynthesizer::fitSignalToEtalonALL_bySpectrometer() {
  if (m_is_show_funny_video) {
    showFunnyVideo();
  }
  fitSignalToEtalon_bySpectrometer(FitSettings::FIT_ALL);
}

void SpectraSynthesizer::fitSignalToEtalonMAX_bySpectrometer() {
  if (m_is_show_funny_video) {
    showFunnyVideo();
  }
  fitSignalToEtalon_bySpectrometer(FitSettings::FIT_BY_MAXIMUMS);
}

void SpectraSynthesizer::showFunnyVideo() {
  QDir dir(QDir::currentPath() + "/video/");
  dir.setFilter(QDir::NoDotAndDotDot | QDir::Files);
  int totalFiles = dir.count();
  int nCommonVideos = totalFiles - 1; //число обычных видео
  int percentOfRare = 5;
  srand(time(0));
  int videoNum = rand() % nCommonVideos + 1;

  bool isRare = !(rand() % (100 / percentOfRare));
  if (isRare) {
    videoNum = 0;
  }
  QString path = QString(QDir::currentPath() + "/video/7.mp4").arg(videoNum);
  //qDebug() << "playing video: " << path;
  QMediaPlayer* player = m_player;
  player->setMedia(QUrl::fromLocalFile(path));
  player->setVolume(100);
  QVideoWidget* videoWidget = ui->widget_video;
  player->setVideoOutput(videoWidget);
  videoWidget->show();
  player->play();
  this->setEnabled(false);
}

void SpectraSynthesizer::fitSignalToEtalon_analytical(const FitSettings& fitSet) {

  //qDebug() << "fit signal to etalon process have been started....";
  m_voice_informator->playSound("run_analytical_fitter.mp3");
  double wavesStep = 1;
  FitSettings emuleSettings = fitSet;
  QVector<lampInfo> diods(m_pins_json_array.size());
  for (int i = 0; i < m_pins_json_array.size(); ++i) {
    //bright_deps
    auto bright_deps = m_pins_json_array[i].toObject()["bright_deps"].toObject();
    diods[i].a = bright_deps["a"].toDouble();
    diods[i].b = bright_deps["b"].toDouble();
    diods[i].c = bright_deps["c"].toDouble();
    diods[i].max_slider_value = m_pins_json_array[i].toObject()["max_value"].toInt();
    diods[i].min_slider_value = m_pins_json_array[i].toObject()["start_light_value"].toInt();

    //qDebug()<<"a: "<<diods[i].a;
    auto values = m_pins_json_array[i].toObject()["model"].toObject()["values"].toArray();
    auto waves = m_pins_json_array[i].toObject()["model"].toObject()["waves"].toArray();
    Q_ASSERT(values.size() == waves.size());
    for (int j = 0; j < values.size(); ++j) {
      diods[i].waves.push_back(waves[j].toDouble());
      diods[i].speya.push_back(values[j].toDouble());
    }

  }
  QVector<double>etalon_speya;
  QVector<double>etalon_grid;
  auto sample = m_etalons[ui->comboBox_etalons->currentText()].toArray();
  auto grid = m_etalons["_grid"].toArray();

  for (int i = 0; i < sample.size(); ++i) {
    auto wave = grid[i].toDouble();
    if (wave > 900)
      break;
    if (wave >= 400) {
      auto speya = sample[i].toDouble();
      etalon_speya.push_back(speya);
      etalon_grid.push_back(wave);
    }
  }

  QVector<double> diod_spea_coefs = find_diod_spea_coefs(etalon_grid,
                                                         etalon_speya,
                                                         wavesStep,
                                                         diods,
                                                         emuleSettings);

  QVector<double> diod_sliders = find_sliders_from_coefs(diod_spea_coefs, diods);

  setValuesForSliders(diod_sliders);
  QTimer::singleShot(m_set_sliders_delay * (diod_sliders.size() + 2),
                     this,
  [this]() {m_voice_informator->playSound("analytical_fitting_finished.mp3");});
}

void SpectraSynthesizer::fitSignalToEtalon_bySpectrometer(const FitSettings& fitSet) {
  m_is_first_previous_for_fitter = true;
  ui->label_info->setText("Процесс автоматического подбора...");
  m_voice_informator->playSound("run_fitting_by_spectrometr.mp3");
  FitSettings emuleSettings = fitSet;
  double wavesStep = 1;
  QVector<double>etalon_speya;
  QVector<double>etalon_grid;
  auto sample = m_etalons[ui->comboBox_etalons->currentText()].toArray();
  auto grid = m_etalons["_grid"].toArray();

  for (int i = 0; i < sample.size(); ++i) {
    auto wave = grid[i].toDouble();
    if (wave > 900)
      break;
    if (wave >= 400) {
      auto speya = sample[i].toDouble();
      etalon_speya.push_back(speya);
      etalon_grid.push_back(wave);
    }
  }
  QVector<lampInfo> diods(m_pins_json_array.size());
  for (int i = 0; i < m_pins_json_array.size(); ++i) {
    //bright_deps
    auto bright_deps = m_pins_json_array[i].toObject()["bright_deps"].toObject();
    diods[i].a = bright_deps["a"].toDouble();
    diods[i].b = bright_deps["b"].toDouble();
    diods[i].c = bright_deps["c"].toDouble();
    diods[i].max_slider_value = m_pins_json_array[i].toObject()["max_value"].toInt();
    diods[i].min_slider_value = m_pins_json_array[i].toObject()["start_light_value"].toInt();

    //qDebug()<<"a: "<<diods[i].a;
    auto values = m_pins_json_array[i].toObject()["model"].toObject()["values"].toArray();
    auto waves = m_pins_json_array[i].toObject()["model"].toObject()["waves"].toArray();
    Q_ASSERT(values.size() == waves.size());
    for (int j = 0; j < values.size(); ++j) {
      diods[i].waves.push_back(waves[j].toDouble());
      diods[i].speya.push_back(values[j].toDouble());
    }
  }

  QVector<double> diod_sliders(m_sliders.size(), 0);
  for (int i = 0; i < m_sliders.size(); ++i) {

    diod_sliders[i] = m_sliders.at(i)->value();
  }

  // delay scenario
  QTimer::singleShot(m_set_sliders_delay * (m_sliders.size() + 1) + m_set_sliders_finalize_delay_ms,
                     this, [this, diod_sliders,
                            etalon_grid, etalon_speya,
  wavesStep, diods, emuleSettings]() {
    m_fitter = new fitterBySpectometer(diod_sliders,
                                       etalon_grid,
                                       etalon_speya,
                                       wavesStep,
                                       diods,
                                       emuleSettings,
                                       m_shared_spectral_data,
                                       m_shared_desired_sliders_positions,
                                       m_isUpdateSpectrForFitter,
                                       m_isSetValuesForSliders,
                                       m_finite_derivative_step,
                                       m_relax_filter_percent,
                                       m_slider_step_for_fitter,
                                       m_ftol_for_fitter,
                                       m_xtol_for_fitter,
                                       m_gtol_for_fitter);

    connect(m_fitter, SIGNAL(workIsFinished()), this, SLOT(finishFitting()));
    QThreadPool* thread_pool = QThreadPool::globalInstance();
    thread_pool->start(m_fitter);
  });
}

void SpectraSynthesizer::setValuesForSliders(const QVector<double>& diod_sliders) {
  Q_ASSERT(m_sliders.size() == diod_sliders.size());
  for (int i = 0; i < diod_sliders.size(); ++i) {
    QTimer::singleShot(m_set_sliders_delay * (i + 1), this, [diod_sliders, i, this]() {
      m_sliders[i]->setValue(diod_sliders[i]);
      emit m_sliders[i]->sliderReleased();
    });
  }
}

void SpectraSynthesizer::setValuesForSliders(const QVector<double>& diod_sliders,
                                             const QVector<double>& diod_sliders_previous) {
  Q_ASSERT(m_sliders.size() == diod_sliders.size());
  Q_ASSERT(m_sliders.size() == diod_sliders_previous.size());
  QVector<int> slidersChanged;
  for (int i = 0; i < diod_sliders.size(); ++i) {
    if (diod_sliders[i] != diod_sliders_previous[i]) {
      slidersChanged.append(i);
    }
  }
  for (int i = 0; i < slidersChanged.size(); ++i) {
    {
      QTimer::singleShot(m_set_sliders_delay * (i + 1), this, [slidersChanged, diod_sliders, i, this]() {
        const auto index = slidersChanged[i];
        m_sliders[index]->setValue(diod_sliders[index]);
        emit m_sliders[index]->sliderReleased();

        if (i == slidersChanged.size() - 1) {
          QTimer::singleShot(100, this, [this] { //время контроллеру на обработку команды
            m_isSetValuesForSliders->store(false);
            m_fitter->isBlocked = false;
            //qDebug() << "UNLOCK AFTER LAST SLIDER WAS SETTED .....\n";
          });

        }
      });

    }
  }


}

void SpectraSynthesizer::setValuesForSlidersBlocked(const QVector<double>& diod_sliders,
                                                    const QVector<double>& diod_sliders_previous) {
  Q_ASSERT(m_sliders.size() == diod_sliders.size());
  QVector<int> slidersChanged;
  for (int i = 0; i < diod_sliders.size(); ++i) {
    if (diod_sliders[i] != diod_sliders_previous[i]) {
      slidersChanged.append(i);
    }
  }
  for (int i = 0; i < slidersChanged.size(); ++i) {
    const auto index = slidersChanged[i];
    m_sliders[index]->setValue(diod_sliders[index]);
    emit m_sliders[index]->sliderReleased();
    Sleep(50);
  }
}

void SpectraSynthesizer::findApparatMaximus() {
  auto arr = m_json_config["pins_array"].toArray();
  Q_ASSERT(m_sliders.size() == arr.size());
  for (int i = 0; i < m_sliders.size(); ++i) {
    auto model = arr[i].toObject()["model"].toObject();
    double maximum = 0;
    double wave = 0;
    auto values = model["values"].toArray();
    auto waves = model["waves"].toArray();
    for (int j = 0; j < values.size(); ++j) {
      auto value = values[j].toDouble();
      if (maximum < value) {
        maximum = value;
        wave = waves[j].toDouble();
      }
    }
    m_apparat_maximus.insert(m_sliders[i]->objectName(), {wave, maximum});
  }
  //qDebug() << m_apparat_maximus;

}

void SpectraSynthesizer::on_comboBox_expositions_currentIndexChanged(int index) {
  if (!m_is_stm_spectrometr_connected)
    return;
  QJsonObject obj;
  jsn::getJsonObjectFromFile("pvd_calibr_list.json", obj);
  m_pvd_calibr = obj["calibrs"].toArray()[index].toObject();
  m_pvd_calibr["waves"] = obj["waves"].toArray();
  m_is_stm_exposition_changed = true;
}

void SpectraSynthesizer::combinateSpectralData(const QVector<double>& currentSpectr,
                                               const QVector<double>& prevSpectr,
                                               QVector<double>& combinatedSpectr) {
  // 1 проверка какие слайдеры были изменены
  QVector<int>changed_sliders;
  for (int i = 0; i < m_sliders.size(); ++i) {
    if (m_sliders[i]->value() != m_prev_sliders_states[i]) {
      changed_sliders.push_back(i);// добавление индекса изменённого слайдера
    }
  }
  if (changed_sliders.size() == 0) { // по идее такого случая не должно быть
    combinatedSpectr = prevSpectr;
    return;
  }
  if (changed_sliders.size() == m_sliders.size()) { // когда все слайдеры изменены возвращаем весь спектр
    combinatedSpectr = currentSpectr;
    return;
  }
  combinatedSpectr = prevSpectr;// комбинировный спектр - на основе предыдущего спектра и нового спектра
  // из нового спектра -> только те участки для которых менялись слайдеры
  auto arr = m_json_config["pins_array"].toArray();
  for (int i = 0; i < changed_sliders.size(); ++i) {
    auto waves = arr[i].toObject()["model"].toObject()["waves"].toArray();
    auto start = waves.first().toDouble();
    auto end = waves.last().toDouble();
    if (start > 900)
      break;
    auto s = m_short_grid_lambda_to_real_indexes.value(start);
    auto e = m_short_grid_lambda_to_real_indexes.value(end);
    if (s < 0 || s > combinatedSpectr.size() - 1) {
      //qDebug() << "START RANGE WRONG CASE..." << s;
      continue;
    }
    if (e < 0 || e > combinatedSpectr.size() - 1) {
      //qDebug() << "END OUT OF RANGE CASE..." << e;
      e = combinatedSpectr.size() - 1;
    }
    for (int ii = s; ii < e; ++ii) {
      combinatedSpectr[ii] = currentSpectr[ii];
    }
  }
}

void SpectraSynthesizer::showComboGraph(QVector<double>& combo_waves,
                                        QVector<double>& combo_values) {
  ui->widget_plot->graph(4)->setData(combo_waves, combo_values);
  ui->widget_plot->replot();
}

void SpectraSynthesizer::createLightModel() {
  auto arr = m_json_config["pins_array"].toArray();
  static QVector<double>m_light_model = QVector<double>(501);
  m_light_model.fill(0);
  QVector<double>f_waves;
  for (int i = 400; i <= 900; ++i) {
    f_waves.push_back(i);
  }
  int slider_index = 0;
  for (int jj = 0; jj < m_sliders.size(); ++jj) {
    if (m_sliders[jj]->value() == 1) {
      continue;
    } else {
      slider_index = jj;
    }
    auto model = arr[slider_index].toObject()["model"].toObject();
    auto bright_deps = arr[slider_index].toObject()["bright_deps"].toObject();
    auto values = model["values"].toArray();
    auto waves = model["waves"].toArray();
    auto coeffs = model["bright_deps"].toObject();
    double wave_start = MAXUINT;
    double wave_end = 0;
    QVector<double>d_values;
    QVector<double>d_waves;
    auto a = bright_deps["a"].toDouble();
    auto b = bright_deps["b"].toDouble();
    auto c = bright_deps["c"].toDouble();
    auto slider_value = m_sliders[slider_index]->value();
    auto slider_max = m_sliders[slider_index]->maximum();
    double speya_c = a * slider_value * slider_value + slider_value * b + c;
    double speya_m = a * slider_max * slider_max + slider_max * b + c;
    double coeff = (double)speya_c / (double)speya_m;
    Q_ASSERT(values.size() == waves.size());
    for (int j = 0; j < values.size(); ++j) {
      auto value = values[j].toDouble();
      auto wave = waves[j].toDouble();
      if (wave < 400 || wave > 900)
        continue;
      value = coeff * value;
      d_values.push_back(value);

      if (wave_start > wave && wave != 0) {
        wave_start = wave;
      }
      if (wave_end < wave) {
        wave_end = wave;
      }
      d_waves.push_back(wave);
    }

    for (int i = 0; i < d_values.size(); ++i) {
      auto index = d_waves[i] - 400;
      if (index > m_light_model.size() - 1)
        break;
      if (index < 0)
        break;
      m_light_model[index] += d_values[i];
    }
  }
  ui->widget_plot->graph(4)->setData(f_waves, m_light_model);
  ui->widget_plot->replot();
}

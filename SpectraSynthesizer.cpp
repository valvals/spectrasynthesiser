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
#include "style_sheets.h"
#include "windows.h"
#include "Version.h"
#include "QFile"
#include "QDir"
#include "QClipboard"

const uint16_t expo_packet_size = 4;
const uint16_t spectr_packet_size = 7384;
const char power_dir[] = "diods_tracker";
const char tracker_full_path[] = "diods_tracker/diods_tracker.json";


SpectraSynthesizer::SpectraSynthesizer(QWidget* parent)
  : QMainWindow(parent)
  , ui(new Ui::SpectraSynthesizer) {
  ui->setupUi(this);
  m_is_show_etalon = false;
  m_is_stm_spectr_update = true;
  m_is_stm_spectrometr_connected = false;
  m_is_diods_arduino_connected = false;
  QrcFilesRestorer::restoreFilesFromQrc(":/");
  QDir dir;
  if (!dir.exists(power_dir)) {
    dir.mkdir(power_dir);
  }
  this->setWindowTitle(QString("СПЕКТРАСИНТЕЗАТОР %1").arg(VER_PRODUCTVERSION_STR));
  QAction* copy_stm_spectr = new QAction;
  copy_stm_spectr->setText("копировать в буфер спектр");
  QAction* copy_etalon_spectr = new QAction;
  copy_etalon_spectr = new QAction;
  copy_etalon_spectr->setText("копировать в буфер эталон");
  ui->widget_plot->addAction(copy_stm_spectr);
  ui->widget_plot->addAction(copy_etalon_spectr);
  ui->widget_plot->setContextMenuPolicy(Qt::ActionsContextMenu);
  connect(copy_stm_spectr, SIGNAL(triggered()), SLOT(copy_spectr_to_clipboard()));
  connect(copy_etalon_spectr, SIGNAL(triggered()), SLOT(copy_etalon_to_clipboard()));
  ui->widget_plot->setBackground(QBrush(QColor(64, 66, 68)));
  ui->widget_plot->addGraph(); // 0 - спектрометр
  ui->widget_plot->addGraph(); // 1 - эталон
  db_json::getJsonObjectFromFile("etalons.json", m_etalons);
  loadEtalons();
  m_power_stat_plot = new QCustomPlot;
  m_hours_stat_plot = new QCustomPlot;
  QPen graphPen(QColor(13, 160, 5));
  ui->widget_plot->graph(0)->setPen(graphPen);
  QPen graphPenEtalon(QColor(130, 255, 5));
  ui->widget_plot->graph(1)->setPen(graphPenEtalon);
  m_serial_diods_controller = new QSerialPort;
  m_serial_stm_spectrometr = new QSerialPort;

  if (!db_json::getJsonObjectFromFile("config.json", m_json_config)) {
    qDebug() << "Config file was not found on the disk...";
    db_json::getJsonObjectFromFile(":/config.json", m_json_config);
  };
  m_pins_json_array = m_json_config.value("pins_array").toArray();
  const QString serial_diods_number = m_json_config.value("serial_diods_controller_id").toString();
  const QString serial_stm_number = m_json_config.value("serial_stm_controller_id").toString();
  const QString mode = m_json_config.value("mode").toString();
  auto available_ports = m_serial_port_info.availablePorts();

  for (int i = 0; i < available_ports.size(); ++i) {
    qDebug() << available_ports[i].serialNumber() << available_ports[i].portName();
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
      m_serial_stm_spectrometr->write("e15000\n");
      m_serial_stm_spectrometr->waitForBytesWritten(1000);
      m_is_stm_spectrometr_connected = true;
    }
  }
  m_sliders_previous_values.reserve(m_pins_json_array.size());
  m_elapsed_timers.reserve(m_pins_json_array.size());
  bool isInitialTrackerFileExists = QFile(tracker_full_path).exists();
  if (isInitialTrackerFileExists)
    db_json::getJsonArrayFromFile(tracker_full_path, m_power_tracker);
  if (m_is_diods_arduino_connected || mode == "developing") {

    for (int i = 0; i < m_pins_json_array.size(); ++i) {
      auto slider = new QSlider;
      m_power_ticks.push_back(i + 1);
      m_power_labels.push_back(QString::number(i + 1));
      slider->setObjectName(QString("qslider_") + QString::number(i + 1));
      slider->setMinimumWidth(30);
      slider->setMinimumHeight(100);
      slider->setMaximumHeight(200);
      m_sliders_previous_values[i] = 1;
      QVBoxLayout* vbl = new QVBoxLayout;
      auto wave = m_pins_json_array[i].toObject().value("wave").toString();
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
      connect(slider, &QSlider::sliderReleased, [i, slider, this]() {
        setTooltipForSlider(i, slider->value());
        sendDataToComDevice(QString("a%1_%2\n").arg(QString::number(i + 1), QString::number(slider->value())));
        ui->comboBox_waves->setCurrentIndex(i);
        ui->spinBox_bright_value->setValue(slider->value());
        savePowerParams(i, slider->value());
        updatePowerStat();
      });
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
      sendDataToComDevice("m\n");
    } else {
      sendDataToComDevice("u\n");
      m_timer_water_cooler_warning.stop();
      ui->centralwidget->setStyleSheet("background-color: rgb(31, 31, 31);color: rgb(0, 170, 0);");
    }
  });

  if (!isInitialTrackerFileExists) {
    db_json::saveJsonArrayToFile(tracker_full_path, m_power_tracker, QJsonDocument::Indented);
  }
  setupPowerStatPlot();
  updatePowerStat();
  connect(ui->action_show_power_stat, SIGNAL(triggered()), m_power_stat_plot, SLOT(show()));
  connect(ui->action_hours_stat, SIGNAL(triggered()), m_hours_stat_plot, SLOT(show()));
  connect(ui->action_copy_power_stat_to_buffer, SIGNAL(triggered()), SLOT(copyPowerStatToClipboard()));
  connect(ui->action_add_etalon, SIGNAL(triggered()), SLOT(createSamplesJson()));
  connect(ui->action_set_all_diods_to_zero, SIGNAL(triggered()), SLOT(reset_all_diods_to_zero()));
  connect(ui->action_hide_etalon, SIGNAL(triggered(bool)), SLOT(mayBeHideEtalon(bool)));
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

void SpectraSynthesizer::readStmData() {
  if (m_serial_stm_spectrometr->bytesAvailable() == expo_packet_size) {
    auto expo = m_serial_stm_spectrometr->readAll();
    m_is_stm_exposition_changed = false;
    if (m_is_stm_spectr_update) {
      update_stm_spectr_clicked();
    }
    m_debug_console->add_message("expo packet recieved from stm: " + QString::number(expo.toInt()) + "\n", dbg::STM_CONTROLLER);
    return;
  } else if (m_serial_stm_spectrometr->bytesAvailable() != spectr_packet_size) {
    qDebug() << "spectr packet recieved" << m_serial_stm_spectrometr->bytesAvailable();
    if (m_serial_stm_spectrometr->bytesAvailable() > spectr_packet_size) {
      qDebug() << "BAD CASE ----> data will never be processing";
      m_serial_stm_spectrometr->readAll();
      update_stm_spectr_clicked();
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
  int max = 0;
  for (size_t i = 0; i < spectr_values_size; ++i) {
    channels.push_back(i + 1);
    values.push_back(spectrumData.spectrum[i]);
    if (max < spectrumData.spectrum[i])
      max = spectrumData.spectrum[i];
  };
  if (!m_is_stm_exposition_changed) {
    show_stm_spectr(values, max);
  } else {
    auto expo_command = QString("e%1\n").arg(ui->spinBox_exposition->value() * 1000);
    m_serial_stm_spectrometr->write(expo_command.toLatin1());
    m_serial_stm_spectrometr->waitForBytesWritten(1000);
  }
}

void SpectraSynthesizer::sendDataToComDevice(const QString& command) {
  m_debug_console->add_message("send to diods controller: " + command.toLatin1(), dbg::DIODS_CONTROLLER);
  if (m_serial_diods_controller->isOpen()) {
    m_serial_diods_controller->write(command.toLatin1());
    Sleep(50);
  }
}

void SpectraSynthesizer::setTooltipForSlider(const int& index, const int& value) {
  QString waveStr = m_pins_json_array[index].toObject().value("wave").toString();
  QString valueStr = (QString::number(value));
  m_sliders[index]->setToolTip(QString(styles::tooltip).arg(waveStr, valueStr));
}

QString SpectraSynthesizer::getGroupID(const double& value) {
  Q_ASSERT(value < 0 || value > 100);
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
  qDebug() << "group: " << group << x_current;
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
    qDebug() << "save: " << db_json::saveJsonArrayToFile(tracker_full_path, m_power_tracker, QJsonDocument::Indented);
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
  db_json::getJsonObjectFromFile("etalons.json", root);
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
  db_json::saveJsonObjectToFile("etalons.json", root, QJsonDocument::Indented);
  m_etalons = root;
  m_etalons_grid.clear();
  ui->comboBox_etalons->clear();
  loadEtalons();
}

void SpectraSynthesizer::loadEtalons() {
  auto objcts = m_etalons["_objects"].toArray();
  auto grid = m_etalons["_grid"].toArray();
  for (int i = 0; i < grid.size(); ++i) {
    m_etalons_grid.push_back(grid[i].toDouble());
  }
  for (int i = 0; i < objcts.size(); ++i) {
    ui->comboBox_etalons->addItem(objcts[i].toString());
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
  ui->widget_plot->xAxis->setRange(400, 2500);
  ui->widget_plot->yAxis->setRange(0, max);
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
  reset_all_diods_to_zero();
  event->accept();
}

void SpectraSynthesizer::reset_all_diods_to_zero() {

  sendDataToComDevice("f\n");
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
  sendDataToComDevice(QString("a%1_%2\n").arg(QString::number(index + 1), QString::number(value)));
}

void SpectraSynthesizer::on_comboBox_waves_currentTextChanged(const QString& arg1) {
  auto index = lambdas_indexes.value(arg1);
  auto max = m_pins_json_array[index].toObject().value("max_value").toInt();
  ui->spinBox_bright_value->setMaximum(max);
  ui->label_value->setToolTip(QString("макс: %1").arg(QString::number(max)));
}

void SpectraSynthesizer::show_stm_spectr(QVector<double> data, double max) {
  QVector<double> waves(spectr_values_size);
  for (int i = 0; i < spectr_values_size; ++i)
    waves[i] = i + 1;
  ui->widget_plot->graph(0)->setData(waves, data);
  ui->widget_plot->xAxis->setRange(1, spectr_values_size);
  ui->widget_plot->yAxis->setRange(0, max);
  ui->widget_plot->replot();
  QTimer::singleShot(100, this, SLOT(update_stm_spectr_clicked()));
}

void SpectraSynthesizer::changeWidgetState() {
  static bool state;
  if (state) {
    ui->centralwidget->setStyleSheet("background-color:rgb(128, 31, 31)");
    state = false;
  } else {
    ui->centralwidget->setStyleSheet("background-color:rgb(31, 31, 31)");
    state = true;
  }
  ui->centralwidget->repaint();
  ui->centralwidget->update();
}

void SpectraSynthesizer::update_stm_spectr_clicked() {
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
    update_stm_spectr_clicked();
  } else {
    ui->pushButton_stop_start_update_stm_spectr->setText("обновлять");
  }
}

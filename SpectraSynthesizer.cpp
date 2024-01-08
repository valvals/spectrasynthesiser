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
    QDir dir;
    if (!dir.exists(power_dir))
        dir.mkdir(power_dir);
    this->setWindowTitle(QString("СПЕКТРАСИНТЕЗАТОР %1").arg(VER_PRODUCTVERSION_STR));
    ui->widget_plot->setBackground(QBrush(QColor(64, 66, 68)));
    ui->widget_plot->addGraph();
    m_power_stat_plot = new QCustomPlot;
    m_hours_stat_plot = new QCustomPlot;
    QPen graphPen(QColor(13, 160, 5));
    ui->widget_plot->graph(0)->setPen(graphPen);
    m_serial_diods_controller = new QSerialPort;
    m_serial_stm_spectrometr = new QSerialPort;
    if (!db_json::getJsonObjectFromFile("config.json", m_json_config)) {
        qDebug() << "Config file was not found on the disk...";
        db_json::getJsonObjectFromFile(":/config.json", m_json_config);
        QrcFilesRestorer::restoreFilesFromQrc(":/");
    };
    m_pins_json_array = m_json_config.value("pins_array").toArray();
    const QString serial_diods_number = m_json_config.value("serial_diods_controller_id").toString();
    const QString serial_stm_number = m_json_config.value("serial_stm_controller_id").toString();
    const QString mode = m_json_config.value("mode").toString();
    auto available_ports = m_serial_port_info.availablePorts();
    bool isDeviceConnected = false;
    for (int i = 0; i < available_ports.size(); ++i) {
        qDebug() << available_ports[i].serialNumber() << available_ports[i].portName();
        if (serial_diods_number == available_ports[i].serialNumber()) {
            m_serial_diods_controller->setPort(available_ports[i]);
            m_serial_diods_controller->open(QIODevice::ReadWrite);
            isDeviceConnected = true;
            connect(m_serial_diods_controller, SIGNAL(readyRead()), this, SLOT(readData()));
        }
        if (serial_stm_number == available_ports[i].serialNumber()) {
            m_serial_stm_spectrometr->setPort(available_ports[i]);
            m_serial_stm_spectrometr->open(QIODevice::ReadWrite);
            connect(m_serial_stm_spectrometr, SIGNAL(readyRead()), this, SLOT(readStmData()));
            m_serial_stm_spectrometr->write("e150\n");
            m_serial_stm_spectrometr->waitForBytesWritten(1000);
        }
    }
    m_sliders_previous_values.reserve(m_pins_json_array.size());
    m_elapsed_timers.reserve(m_pins_json_array.size());
    bool isInitialTrackerFileExists = QFile(tracker_full_path).exists();
    if (isInitialTrackerFileExists)
        db_json::getJsonArrayFromFile(tracker_full_path, m_power_tracker);
    if (isDeviceConnected || mode == "developing") {

        for (int i = 0; i < m_pins_json_array.size(); ++i) {
            auto slider = new QSlider;
            m_power_ticks.push_back(i+1);
            m_power_labels.push_back(QString::number(i+1));
            slider->setObjectName(QString("qslider_") + QString::number(i + 1));
            slider->setMinimumWidth(30);
            slider->setMinimumHeight(100);
            m_sliders_previous_values[i] = 1;
            QVBoxLayout* vbl = new QVBoxLayout;
            auto wave = m_pins_json_array[i].toObject().value("wave").toString();
            ui->comboBox_waves->addItem(wave);
            lambdas_indexes.insert(wave, i);
            vbl->addWidget(new QLabel(wave));
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
    connect(&m_timer, SIGNAL(timeout()), SLOT(changeWidgetState()));

    if (!isInitialTrackerFileExists) {
        db_json::saveJsonArrayToFile(tracker_full_path, m_power_tracker, QJsonDocument::Indented);
    }
    setupPowerStatPlot();
    updatePowerStat();
    connect(ui->action_show_power_stat,SIGNAL(triggered()),m_power_stat_plot,SLOT(show()));
    connect(ui->action_hours_stat,SIGNAL(triggered()),m_hours_stat_plot,SLOT(show()));
    connect(ui->action_copy_power_stat_to_buffer,SIGNAL(triggered()),SLOT(copyPowerStatToClipboard()));
}

SpectraSynthesizer::~SpectraSynthesizer() {
    delete ui;
}

void SpectraSynthesizer::readData() {
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
        m_debug_console->add_message("recieved from stm: " + QString::number(expo.toInt()) + "\n", dbg::STM_CONTROLLER);
        return;
    } else if (m_serial_stm_spectrometr->bytesAvailable() != spectr_packet_size) {
        qDebug() << "spectr packet recieved";
        return;
    }
    auto ba = m_serial_stm_spectrometr->readAll();
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
    show_stm_spectr(values, max);
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

void SpectraSynthesizer::setupPowerStatPlot()
{
    QLinearGradient gradient(0, 0, 0, 400);
    gradient.setColorAt(0, QColor(90, 90, 90));
    gradient.setColorAt(0.38, QColor(105, 105, 105));
    gradient.setColorAt(1, QColor(70, 70, 70));
    m_power_stat_plot->setBackground(QBrush(gradient));
    m_hours_stat_plot->setBackground(QBrush(gradient));

    auto min_size = QSize(900,600);
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
    for(int i=0;i<10;++i){
        auto bar = new QCPBars(m_power_stat_plot->xAxis, m_power_stat_plot->yAxis);
        bar->setName(QString(QString::number((i+1)*10))+ " %");
        m_power_bars.push_back(bar);
    }
    m_hours_bar = new QCPBars(m_hours_stat_plot->xAxis, m_hours_stat_plot->yAxis);
    m_power_stat_plot->xAxis->setRange(0,31);
    m_hours_stat_plot->xAxis->setRange(0,31);
    //m_test_plot->xAxis->setTickLabelRotation(70);

    QVector<QColor>colors = {{71,107,76},
                             {37,200,56},
                             {8,246,186},
                             {227,255,0},
                             {255,163,0},
                             {255,100,0},
                             {180,40,0},
                             {148,46,179},
                             {230,40,165},
                             {255,0,0}
                            };
    for(int i=0;i<m_power_bars.size();++i){
         if(i!=m_power_bars.size()-1){
             m_power_bars[i+1]->moveAbove(m_power_bars[i]);
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

void SpectraSynthesizer::copyPowerStatToClipboard()
{
    QString copy_stat;

    for(int i=0;i<m_power_tracker.size();++i){
     auto object = m_power_tracker[i].toObject();
     copy_stat.append(object["name"].toString());
     copy_stat.append("\t");
     copy_stat.append(object["wave"].toString());
     copy_stat.append("\t");
     copy_stat.append(QString::number(object["time"].toDouble()));
     copy_stat.append("\t");
     copy_stat.append(QString::number(object["current"].toDouble()));
     copy_stat.append("\t");
     for(int j=0;j<10;++j){
     copy_stat.append(QString::number(object[QString::number((j+1)*10)].toDouble()));
     if(j!=9)copy_stat.append("\t");
     }
     copy_stat.append("\n");
    }
    auto clip = QGuiApplication::clipboard();
    clip->setText(copy_stat);

}

void SpectraSynthesizer::updatePowerStat()
{
    double max = 0.0;
    for(int i=0;i<m_power_bars.size();++i){
         QVector<double>data;
         for(int j=0;j<m_pins_json_array.size();++j){
           data.push_back(m_power_tracker[j].toObject().value(QString::number((i+1)*10)).toDouble());
           auto current = m_power_tracker[j].toObject().value("current").toDouble();
           if(max<current)max =current;
         }
         m_power_bars[i]->setData(m_power_ticks,data);
    }
    m_power_stat_plot->yAxis->setRange(0,max);
    m_power_stat_plot->replot();

    max=0.0;
    QVector<double>hours;
    for(int j=0;j<m_pins_json_array.size();++j){

      auto time = m_power_tracker[j].toObject().value("time").toDouble();
      hours.push_back(time);
      if(max<time)max = time;
    }
    m_hours_bar->setData(m_power_ticks,hours);
    m_hours_stat_plot->yAxis->setRange(0,max);
    m_hours_stat_plot->replot();
}

void SpectraSynthesizer::closeEvent(QCloseEvent *event)
{
    event->ignore();
    on_pushButton_reset_to_zero_clicked();
    event->accept();
}

void SpectraSynthesizer::on_pushButton_reset_to_zero_clicked() {

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

void SpectraSynthesizer::on_pushButton_update_stm_spectr_clicked() {
    m_serial_stm_spectrometr->write("r\n");
    m_serial_stm_spectrometr->waitForBytesWritten(1000);
    Sleep(50);
}

void SpectraSynthesizer::on_pushButton_exposition_clicked() {
    m_serial_stm_spectrometr->write("e150\n");
    m_serial_stm_spectrometr->waitForBytesWritten(1000);
}

void SpectraSynthesizer::on_pushButton_sound_switcher_toggled(bool checked) {
    if (checked) {
        ui->pushButton_sound_switcher->setText("Включить звук");
        sendDataToComDevice("m\n");
        m_timer.start(1000);
    } else {
        ui->pushButton_sound_switcher->setText("Выключить звук");
        sendDataToComDevice("u\n");
        m_timer.stop();
        ui->centralwidget->setStyleSheet("background-color: rgb(31, 31, 31);color: rgb(0, 170, 0);");
    }
}

void SpectraSynthesizer::on_pushButton_water_clicked() {
    sendDataToComDevice("w\n");
}

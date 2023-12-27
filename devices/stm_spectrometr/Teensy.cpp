#include "Teensy.h"
#include <QDebug>
#include "math/RelaxMediumAlg.h"
#include "math.h"
#include <QDateTime>

Teensy::Teensy(QString comPortName, int baudRate): BaseSpectroDevice() {
  m_isNeedToSaveNow = false;
  m_isNeedToUpdate = true;
  m_isSpectrumRequested = false;
  m_isExpositionChanging = false;
  m_expositionValue = 15;
  m_bandsCount = 3648;
  m_comPortManager = new ComPortManager(comPortName, baudRate);
  connect(m_comPortManager, SIGNAL(dataIsReady(QByteArray)), this, SLOT(parseComPortData(QByteArray)));
  connect(this, SIGNAL(writeCommandToComPort(QByteArray)), m_comPortManager, SLOT(writeCommand(QByteArray)), Qt::QueuedConnection);

  m_experimentTimeMs = QDateTime::currentDateTime().currentMSecsSinceEpoch();
}

Teensy::~Teensy() {
  delete m_comPortManager;
}

bool Teensy::isConnected() const {
  return m_comPortManager->connectionState();
}

void Teensy::captureSpectr() {
  m_maxValueInSpectrum = 0;
  m_spectrData.clear();
  m_specString.clear();
  m_isSpectrumRequested = true;
  if (!m_isExpositionChanging) {
//        qDebug()<<"Capturing Time:"<<QDateTime::currentDateTime().currentMSecsSinceEpoch() - m_experimentTimeMs;
    m_experimentTimeMs = QDateTime::currentDateTime().currentMSecsSinceEpoch();
    emit writeCommandToComPort(QByteArray::fromStdString("r\n"));
  }
}

void Teensy::setExpositionValue(double expositionValue) {
  qDebug() << "Capturing Time:" << QDateTime::currentDateTime().currentMSecsSinceEpoch();
  //    qDebug()<<"setExpositionValue"<<expositionValue;
  if (expositionValue > 14.99) {
    m_isNeedToUpdate = false;
    m_tempExpositionValue = expositionValue;
    if (m_comPortManager->currentCommandType() == NO_COMMAND) {
      m_expositionValue = m_tempExpositionValue;
      requestExpositionChanging();
    }
  }
}

void Teensy::parseComPortData(QByteArray comAnswer) {
  qDebug() << "Answer: ....." << comAnswer.size();
  if (m_comPortManager->currentCommandType() == CHANGING_EXPOSITION) {
    qDebug() << "expo recieved:" << comAnswer;
    m_expositionValue = m_tempExpositionValue;
    emit expositionWasChanged(m_expositionValue);
    m_isExpositionChanging = false;
    m_isNeedToUpdate = true;
    if (m_isSpectrumRequested)
      captureSpectr();
  } else if (m_comPortManager->currentCommandType() == GETTING_SPECTRUM) {
    SpectrumData spectrumData;
    memcpy(&spectrumData, comAnswer.data(), sizeof(spectrumData));
    //minusDarkPixelsAndSend(spectrumData);
    QVector<double>values(3648);
    double max = 0;
    for (size_t i = 0; i < 3648; ++i) {
      values[i] = spectrumData.spectrum[i];
      if (max < values[i])
        max = values[i];
    }
    emit spectrReadyToShow(values, max, true);
  }
}

void Teensy::minusDarkPixelsAndSend(SpectrumData& spectrumData) {
  double spectrumNoise = 0;
  for (int i = 0; i < 13; i++) {
    spectrumNoise += spectrumData.black1[i];
    m_blackPixels.append(spectrumData.black1[i]);
  }
  spectrumNoise = spectrumNoise / 13.0;

  addSpectrometerStatesToString();

  m_maxValueInSpectrum = 0;
  for (int i = 0; i < 3648; i++) {
    double spectrumValue = spectrumData.spectrum[i] - spectrumNoise;
//        if(spectrumValue < 0)
//            spectrumValue = 0;
    m_spectrData.append(spectrumValue);
    if (m_maxValueInSpectrum < spectrumData.spectrum[i] - static_cast<int>(spectrumNoise))
      m_maxValueInSpectrum = spectrumData.spectrum[i] - static_cast<int>(spectrumNoise);

    if (m_bands.isEmpty())
      m_specString.append(QString::number(i + 1) + "\t" + QString::number(m_spectrData[i]) + "\n");
    else
      m_specString.append(QString::number(m_bands.at(i)) + "\t" + QString::number(m_spectrData[i]) + "\n");
  }

  if (m_isNeedToSaveNow) {
    saveSpectrum();
  }

  if (m_maxValueInSpectrum < 150) {
    m_spectrData = RelaxMediumAlg::getFilteredSpectrum(m_spectrData, 90);
  }

  if (m_spectrMode != SINGLE_MODE)
    m_isNeedToUpdate = true;
  m_isSpectrumRequested = false;
  qDebug() << "Capturing Time:" << QDateTime::currentDateTime().currentMSecsSinceEpoch() - m_experimentTimeMs;
  emit spectrReadyToShow(m_spectrData, m_maxValueInSpectrum, m_isNeedToUpdate);

  if (!m_isNeedToUpdate) {
    m_isSpectrumRequested = true;
    requestExpositionChanging();
  }

}

void Teensy::requestExpositionChanging() {
//    qDebug()<<"requestExpositionChanging"<<m_expositionValue;
  m_isExpositionChanging = true;
  QString command("e" + QString::number(static_cast<int>(ceil(m_tempExpositionValue))) + "\n");
  emit writeCommandToComPort(QByteArray::fromStdString(command.toStdString()));
}

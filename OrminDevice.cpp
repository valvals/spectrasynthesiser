#include "OrminDevice.h"
#include "qdebug.h"
#include <QDir>
#include <QDateTime>
#include <Windows.h>
#include <QMessageBox>


OrminDevice::OrminDevice(int id) {
  m_sensor = new HamamatsuApi;
  m_sensor->InitInstance();
  m_isNeedSave = false;
  m_isNeedSaveBlackSpectr = false;
  m_isNeedMinusBlackFromSpectr = false;
  m_exposition = 1;
  m_blackExposition = 0;
  m_counterSavedSpectrs = 0;
  m_horizontalPixels = 0;
  m_verticalPixels = 0;
  m_startSpectrLine = 0;
  m_endSpectrLine = 0;
  m_isNeedSave = false;
  m_isNeedChangeExposition = false;
  m_isNeedMatrixShow = false;
  m_isNeedCalculateMean = true;
  isNeedToChangeSensorMode = true;
  m_modeToChange = SPECTROSCOPE;
  m_isConnected = false;
  m_isConnected = m_sensor->attach(id);
  qDebug() << "Подключение к сенсору: " << id << m_isConnected;

  if (m_isConnected) {
    m_sensor->resetDevice();
    m_sensor->setSensivity(HIGH_SENSIVITY);
    m_sensor->setSensorMode(m_modeToChange);
    m_sensor->setExposition(static_cast<float>(m_exposition));
    m_sensor->getParameters();
    m_sensor->prepareMemory();

    m_horizontalPixels = m_sensor->m_pixelsInRow;
    m_verticalPixels = m_sensor->m_pixelsInColumn;
    m_endSpectrLine = m_verticalPixels;
    qDebug() << "Количество горизонтальных пикселей: " << m_horizontalPixels;
    qDebug() << "Количество вертикальных пикселей:" << m_verticalPixels;
    connect(this, SIGNAL(requestSpectr()), this, SLOT(shootSpectrum()));
  }

}

OrminDevice::~OrminDevice() {

}

void OrminDevice::shootSpectrum() {

  if (m_exposition >= 0 && m_exposition <= 2000 && m_isNeedChangeExposition) {

    m_sensor->setExposition(static_cast<float>(m_exposition));
    m_isNeedChangeExposition = false;

  }
  if (isNeedToChangeSensorMode) {

    qDebug() << "Setting sensor mode:" << m_modeToChange;
    m_sensor->setSensorMode(m_modeToChange);
    isNeedToChangeSensorMode = false;
    m_sensor->getParameters();

  }

  if (m_isNeedSaveBlackSpectr) {

    m_blackExposition = m_exposition;
    emit blackExpositionChanged(&m_blackExposition);
  }

  double sum = 0;
  m_maxValueInSpectr = 0;
  m_minValueInSpectr = ULONG_MAX;
  m_spectralData.clear();
  m_summingSpectralData.clear();
  m_matrixData.clear();

  bool gettingSpectrResult;
  DWORD* data = m_sensor->getSpectralData(gettingSpectrResult);
  if (!gettingSpectrResult) {
    qDebug() << "getting Spectr error";
    emit gettingSpectrError();
    return;
  }
  m_horizontalPixels = m_sensor->m_pixelsInRow;
  m_verticalPixels = m_sensor->m_pixelsInColumn;
  //qDebug()<<"Количество горизонтальных пикселей: "<<m_horizontalPixels;
  //qDebug()<<"Количество вертикальных пикселей:"<<m_verticalPixels;

  if (data != nullptr) {

    m_summingSpectralData.append("Exposition:\t");
    m_summingSpectralData.append(QString::number(m_exposition));
    m_summingSpectralData.append("\n");


    for (int i = 0; i < m_horizontalPixels; i++) {
      for (int j = m_startSpectrLine; j < m_verticalPixels - (m_verticalPixels - m_endSpectrLine); j++) {


        if (m_isNeedMinusBlackFromSpectr && m_blackSpectralData.count() == m_horizontalPixels) {

          sum = sum + data[i + j * m_horizontalPixels] - m_blackSpectralData[i + j * m_horizontalPixels];

        } else {

          sum = (sum + data[i + j * m_horizontalPixels]);
        }

        if (m_isNeedSaveBlackSpectr) {

          m_blackSpectralData.append(data[i + j * m_horizontalPixels]);

        }

      }

      if (m_isNeedCalculateMean)
        sum = sum / m_verticalPixels;
      if (sum > m_maxValueInSpectr)
        m_maxValueInSpectr = sum;
      if (sum < m_minValueInSpectr)
        m_minValueInSpectr = sum;


      m_spectralData.append(sum);
      m_summingSpectralData.append(QString::number(i + 1));
      m_summingSpectralData.append("\t");
      m_summingSpectralData.append(QString::number(sum));
      m_summingSpectralData.append("\n");
      sum = 0;

    }

    if (m_verticalPixels > 1) {
      for (int k = 0; k < m_sensor->m_pixelsInColumn; k++) {

        for (int l = 0; l < m_sensor->m_pixelsInRow; l++) {

          m_matrixData.append(data[k * m_sensor->m_pixelsInRow + l]);

        }
      }
    }

    if (m_isNeedSave) {

      saveSpectr();
    }
    if (m_isNeedSaveBlackSpectr == true) {
      m_isNeedSaveBlackSpectr = false;
      emit darkSignalWasSaved();
      return;
    }
    //qDebug()<<m_spectralData.size()<<"**************************************************************";
    emit spectralDataRecieved(m_spectralData, m_maxValueInSpectr, m_minValueInSpectr);
  }

}

void OrminDevice::saveSpectr() {
  QString timeStamp = QDateTime::currentDateTime().toString("yyyyMMd_hhmmss.zzz_");
  QString fn;
  fn.append(m_pathToSaveDir);
  fn.append(timeStamp);

  fn.append(m_fileNameMetaData).append("_");
  fn.append(QString::number(m_exposition)).append("_");
  fn.append(QString::number(++m_counterSavedSpectrs));
  fn.append(".asc");
  qDebug() << fn;

  QFile file(fn);
  if (!file.open(QIODevice::WriteOnly))
    return;
  QTextStream out(&file);
  out << m_summingSpectralData;
  file.close();
  emit savedSpectrCounterChanged(m_counterSavedSpectrs);
}

void OrminDevice::finishWork() {

}

QString OrminDevice::getSerialNumber() const {
  return m_serialNumber;
}

double OrminDevice::getExposition() const {
  return m_exposition;
}

void OrminDevice::setExposition(double value) {
  m_isNeedChangeExposition = true;
  m_exposition = value;
}

void OrminDevice::setSaveFolderPath(QString path) {
  m_pathToSaveDir = path;
  qDebug() << "m_pathToSaveDir" << m_pathToSaveDir;
}

void OrminDevice::setIsNeedMinusBlackSpectr(bool value) {
  m_isNeedMinusBlackFromSpectr = value;
}

void OrminDevice::setSavedSpectrsCounter(int value) {
  m_counterSavedSpectrs = value;
}

void OrminDevice::changeSensorMode(sensorMode mode) {
  isNeedToChangeSensorMode = true;
  m_modeToChange = mode;
  switch (mode) {
    case MATRIX: qDebug() << "Matrix";
      break;
    case SPECTROSCOPE: qDebug() << "Spectroscope";
      break;
  }

}

void OrminDevice::saveSpectrTest() {
  saveSpectr();
}

void OrminDevice::setIsNeedSaveBlackSpectr(bool value) {
  m_isNeedSaveBlackSpectr = value;
  if (value)
    m_blackSpectralData.clear();
}

bool OrminDevice::getIsSensorConnected() const {
  return m_isConnected;
}

void OrminDevice::setFileNameMetaData(const QString& fileNameMetaData) {
  m_fileNameMetaData = fileNameMetaData;
}

int OrminDevice::getCounterSavedSpectrs() const {
  return m_counterSavedSpectrs;
}

void OrminDevice::setStartSpectrLine(int startSpectrLine) {
  m_startSpectrLine = startSpectrLine;
}

void OrminDevice::setEndSpectrLine(int endSpectrLine) {
  m_endSpectrLine = endSpectrLine;
}

void OrminDevice::setIsNeedCalculateMean(bool isNeedCalculateMean) {
  m_isNeedCalculateMean = isNeedCalculateMean;
}

double OrminDevice::getBlackExposition() const {
  return m_blackExposition;
}

void OrminDevice::setIsNeedMatrixShow(bool value) {
  m_isNeedMatrixShow = value;
}

void OrminDevice::setIsNeedChangeExposition(bool value) {
  m_isNeedChangeExposition = value;
}

int OrminDevice::getNumberOfHorizontalPixels() const {
  return m_horizontalPixels;
}

int OrminDevice::getNumberOfVerticalPixels() const {
  return m_verticalPixels;
}

void OrminDevice::setIsNeedSaveSpectr(bool value) {
  m_isNeedSave = value;
}

#include "SpectrDataSaver.h"
#include <QTextStream>
#include <QIODevice>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <qdebug.h>
#include <QtConcurrent/QtConcurrent>
using namespace QtConcurrent;

SpectrDataSaver::SpectrDataSaver(QObject* parent) : QObject(parent) {
  m_counter4Recording = 0;
  m_darkSpectrumsCounter = 0;
  m_darkSpectrumsCount = 0;
  m_isCalculatingReflectanceNow = false;
  m_isMilkSpectrumNow = false;
  m_isDarkSpectrumsNow = false;
  m_directoryAppendix = "";
}

void SpectrDataSaver::saveStringToFile(QString baseFileName, QString* strData) {
  QString absoluteFileName = getAbsoluteFileName(baseFileName);
//    qDebug()<<"saving..."<<absoluteFileName;
  createFileAndWrite(absoluteFileName, strData);

  if (m_spectrMode == SINGLE_MODE || m_counter4Recording == 0) {
    emit spectrSeriesWasFinished();
    emit spectrWasSaved(100);
  } else if (m_spectrMode == COMBO_MODE) {
    emit spectrWasSaved(m_counter4Recording * 100 / (2 * m_countInSeries));
  } else {
    emit spectrWasSaved(m_counter4Recording * 100 / m_countInSeries);
  }
}

void SpectrDataSaver::createFileAndWrite(QString absoluteFileName, QString* strData) {
  QFile file(absoluteFileName);
  file.open(QFile::WriteOnly);
  QTextStream out(&file);
  out.setCodec("cp1251");
  out << *strData;
  file.close();
}

QString SpectrDataSaver::getAbsoluteFileName(QString baseFileName) {
  checkDirAndCreateIfNeeded(m_savingDirectory);
//    m_currentSavingDirectory = m_savingDirectory + m_directoryAppendix;
  m_currentSavingDirectory = m_savingDirectory;
  checkDirAndCreateIfNeeded(m_currentSavingDirectory);

  ++m_counter4Recording;
  QString absoluteFileName;

  switch (m_spectrMode) {
    case SINGLE_MODE:
      absoluteFileName = getSingleSpectrumName(baseFileName);
      m_currentSavingDirectory.clear();
      break;
    case CONTINUOUS_MODE:
      absoluteFileName = getSingleSpectrumName(baseFileName);
      m_currentSavingDirectory.clear();
      break;
    case SERIES_MODE:
      absoluteFileName = getSeriesSpectrumName(baseFileName);
      break;
    case COMBO_MODE:
      absoluteFileName = getComboSpectrumName(baseFileName);
      break;
  }

  return absoluteFileName;
}

QString SpectrDataSaver::getSingleSpectrumName(QString baseFileName) {
  QString absoluteFileName;

  if (m_singleFileName.isEmpty()) {
    absoluteFileName = m_currentSavingDirectory;
    emit currentFolderChanged(m_currentSavingDirectory);
    absoluteFileName.append("/");
    absoluteFileName.append(baseFileName);
    absoluteFileName.append(".txt");
  } else {
    absoluteFileName = m_singleFileName;
    m_singleFileName.clear();
  }
  --m_counter4Recording;

  return absoluteFileName;
}

QString SpectrDataSaver::getSeriesSpectrumName(QString baseFileName) {
//    qDebug()<<m_currentSavingDirectory;
  if (m_currentSavingDirectory.isEmpty()) {
    m_currentSavingDirectory = m_savingDirectory + m_directoryAppendix;
  }
  checkDirAndCreateIfNeeded(m_currentSavingDirectory);
  QString appendix;
  if (m_isDarkSpectrumsNow)
    appendix = "/DarkSpectrums_";
  else if (m_isMilkSpectrumNow)
    appendix = "/MilkSpectrums_";
  else
    appendix = "/SeriesSpectrums_";

  QString absoluteFileName = getSpectrumNameWithAppendix(appendix, baseFileName);
  appendNameForSeries(absoluteFileName, baseFileName);
  if (m_spectrMode == SERIES_MODE)
    checkForDirectoryLevel();
//    qDebug()<<m_currentSavingDirectory;

  return absoluteFileName;
}

QString SpectrDataSaver::getComboSpectrumName(QString baseFileName) {
  if (m_currentSavingDirectory.isEmpty()) {
    m_currentSavingDirectory = m_savingDirectory + m_directoryAppendix;
  }
  checkDirAndCreateIfNeeded(m_currentSavingDirectory);

  QString appendix;
  if (m_isMilkSpectrumNow)
    appendix = "/MilkSpectrums_";
  else
    appendix = "/Spectrums_";
  QString absoluteFileName = getSpectrumNameWithAppendix(appendix, baseFileName);

  appendNameForComboSeries(absoluteFileName, baseFileName);
  checkForDirectoryLevel();

  return absoluteFileName;
}

QString SpectrDataSaver::getSpectrumNameWithAppendix(QString folderStartName, QString baseFileName) {
  QString absoluteFileName = m_currentSavingDirectory;

  if (m_directoryAppendix.isEmpty()) {
    m_directoryAppendix.append(folderStartName);
    m_directoryAppendix.append(baseFileName);
  }
  absoluteFileName.append(m_directoryAppendix);
  checkDirAndCreateIfNeeded(absoluteFileName);
  absoluteFileName.append("/");

  return absoluteFileName;
}

void SpectrDataSaver::appendNameForSeries(QString& currentAbsoluteName, QString baseFileName) {
  currentAbsoluteName.append(baseFileName);

  if (m_counter4Recording == 0) {
    emit currentFolderChanged(m_currentSavingDirectory);
    currentAbsoluteName.append("_mean");
  } else if (m_counter4Recording > 0) {
    currentAbsoluteName.append("_" + QString::number(m_counter4Recording));
  }
  currentAbsoluteName.append(".txt");
}

void SpectrDataSaver::appendNameForComboSeries(QString& currentAbsoluteName, QString baseFileName) {
  currentAbsoluteName.append(baseFileName);

  if (m_counter4Recording == 0) {
    emit currentFolderChanged(m_currentSavingDirectory);
    currentAbsoluteName.append("_mean");
  } else if (m_counter4Recording > 0) {
    if (m_isDarkSpectrumsNow)
      currentAbsoluteName.append("_d");
    currentAbsoluteName.append("_" + QString::number(ceil(static_cast<double>(m_counter4Recording) / 2)));
  }
  currentAbsoluteName.append(".txt");
}

void SpectrDataSaver::checkForDirectoryLevel() {
  m_lastSavedFolder = m_currentSavingDirectory;
  switch (m_spectrMode) {
    case COMBO_MODE:
    case SERIES_MODE:
      if (m_counter4Recording == 0) {
//            qDebug()<<"clearing...";
        emit currentFolderChanged(m_currentSavingDirectory + m_directoryAppendix);
        m_lastSavedFolder = m_currentSavingDirectory + m_directoryAppendix;
        m_directoryAppendix.clear();
        m_currentSavingDirectory.clear();
        m_isMilkSpectrumNow = false;
        m_isDarkSpectrumsNow = false;
      }
      break;
    default:
      break;
  }
}

void SpectrDataSaver::upCurrentDirectory() {
  QDir currentDir(m_currentSavingDirectory);
  currentDir.cdUp();
  m_currentSavingDirectory = currentDir.path();
}

void SpectrDataSaver::checkDirAndCreateIfNeeded(QString folderPath) {
  QDir checkDir(folderPath);
  if (!checkDir.exists()) {
    checkDir.mkdir(folderPath);
  }
}

QString SpectrDataSaver::savingDirectory() const {
  return m_savingDirectory;
}

void SpectrDataSaver::setSavingDirectory(SpectrometerSettingsType type, QVariant var) {
  if (type == SAVING_FILE_PATH)
    setSavingDirectory(var.value<QString>());
}

void SpectrDataSaver::setSavingDirectory(QString savingDir) {
  m_savingDirectory = savingDir;
  m_lastSavedFolder = m_savingDirectory;
}

QString SpectrDataSaver::getLastSavedFolder() const {
  return m_lastSavedFolder;
}

void SpectrDataSaver::setIsCalculatingReflectanceNow(bool isCalculatingReflectanceNow) {
  m_isCalculatingReflectanceNow = isCalculatingReflectanceNow;
}

void SpectrDataSaver::clearCurrentSavingDirectory() {
  m_currentSavingDirectory.clear();
}

void SpectrDataSaver::setIsMilkSpectrumNow(bool isMilkSpectrumNow) {
  m_isMilkSpectrumNow = isMilkSpectrumNow;
}

void SpectrDataSaver::setDarkSpectrumsCount(int darkSpectrumsCount) {
  m_darkSpectrumsCount = darkSpectrumsCount;
}

void SpectrDataSaver::setIsDarkSpectrumsNow(bool isDarkSpectrumsNow) {
  m_isDarkSpectrumsNow = isDarkSpectrumsNow;
}

void SpectrDataSaver::setSpectrMode(const SpectrMode& spectrMode) {
  m_spectrMode = spectrMode;
}

void SpectrDataSaver::setSingleFileName(const QString& singleFileName) {
  m_singleFileName = singleFileName;
}

int SpectrDataSaver::counter4Recording() const {
  return m_counter4Recording;
}

void SpectrDataSaver::saveStringToFileLater(QString baseFileName, QString strData) {
  QtConcurrent::run(this, &SpectrDataSaver::saveInSeparateThread, baseFileName, strData);
}

void SpectrDataSaver::saveInSeparateThread(QString baseFileName, QString strData) {
  QString absoluteFileName = getAbsoluteFileName(baseFileName);
//    qDebug()<<"saving spectrum in separate thread:"<<absoluteFileName;
  createFileAndWrite(absoluteFileName, &strData);
}

void SpectrDataSaver::setCountInSeries(int countInSeries) {
  m_countInSeries = countInSeries;
}

void SpectrDataSaver::setCounter4Recording(int value) {
  m_counter4Recording = value;
}

#include "CalibrationModule.h"
#include <QFile>
#include <QTextStream>
#include "qdebug.h"
#include <QDir>

CalibrationModule::CalibrationModule(QObject* parent) : QObject(parent) {

}

void CalibrationModule::loadWavesCalibrationFile(QString filepath) {
  QFile inputFile(filepath);
  if (inputFile.exists()) {
    if (inputFile.open(QIODevice::ReadOnly)) {
      QTextStream in(&inputFile);

      while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList xyInLine = line.split(QRegExp("\\s+"));
        int indexOfEmpty = xyInLine.indexOf(QString(""));
        xyInLine.removeAt(indexOfEmpty);
        if (xyInLine.count() < 2)
          continue;

        bool ok;
        double wavelenth = xyInLine.at(1).toDouble(&ok);
        if (ok && wavelenth > 0) {
          m_wavelength.append(wavelenth);
        } else {
          sendMessage("Ошибка при загрузке калибровочного файла на длине волны " + QString::number(wavelenth));
          break;
        }
      }
      inputFile.close();
    }
  }
}

void CalibrationModule::loadSolarExpositionCalibrationFile(QString filepath) {
  QFile inputFile(filepath);
  if (inputFile.exists()) {
    if (inputFile.open(QIODevice::ReadOnly)) {
      QTextStream in(&inputFile);

      bool isOk = true;
      QString line = in.readLine();
      QStringList sList = line.split(QRegExp("\t"));
      for (int i = 1; i < sList.count(); i++) {
        bool isOk;
        QPair<int, QVector<double>> expoCalibrationForPixel;
        expoCalibrationForPixel.first = sList.at(i).toInt(&isOk);
        QVector<double> emptyVector;
        expoCalibrationForPixel.second = emptyVector;
        m_expositionCalibrationsList.append(expoCalibrationForPixel);
      }

      while (!in.atEnd()) {
        line = in.readLine();
        sList = line.split(QRegExp("\t"));
        for (int i = 1; i < sList.count(); i++) {
          double value = sList.at(i).toDouble(&isOk);
          if (!isOk) {
            sendMessage("Ошибка при загрузке файла с темновыми спектрами");
            break;
          }
          m_expositionCalibrationsList[i - 1].second.append(value);
        }
      }
      inputFile.close();
    }
  }
//    qDebug()<<"Expo calibrations:"<<m_expositionCalibrationsList.count()<<m_expositionCalibrationsList.first().second.count();
}

QVector<double> CalibrationModule::wavelength() const {
  return m_wavelength;
}

QList<QVector<double> > CalibrationModule::calibrationsList() const {
  return m_calibrationsList;
}

QList<QPair<int, QVector<double> > > CalibrationModule::expositionCalibrationsList() const {
  return m_expositionCalibrationsList;
}

QVector<double> CalibrationModule::getDarkSpectrumValues(QString filepath) {
  QVector<double> resultValues;
  QFile inputFile(filepath);
  if (inputFile.exists()) {
    if (inputFile.open(QIODevice::ReadOnly)) {
      QTextStream in(&inputFile);

      while (!in.atEnd()) {
        QString line = in.readLine();
        if (!(line.contains("Spectrometer") || line.contains("Bands") ||
              line.contains("Exposition") || line.contains("Angle") ||
              line.contains("Filter") || line.contains("Shutter") ||
              line.isEmpty())) {
          QStringList xyInLine = line.split(QRegExp("\\s+"));
          int indexOfEmpty = xyInLine.indexOf(QString(""));
          xyInLine.removeAt(indexOfEmpty);
          if (xyInLine.count() < 2)
            continue;

          bool ok;
          double value = xyInLine.at(1).toDouble(&ok);
          if (ok) {
            resultValues.append(value);
          }
        }
      }
      inputFile.close();
    }
  }
  return resultValues;
}

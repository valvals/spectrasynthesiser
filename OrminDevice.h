#ifndef ORMINDEVICE_H
#define ORMINDEVICE_H
#include <QObject>
#include  "HamamatsuApi.h"
#include <QVector>

class OrminDevice : public QObject {
  Q_OBJECT

 public:
  OrminDevice(int id);
  ~OrminDevice();

  void setExposition(double value);
  void setSaveFolderPath(QString path);
  void setIsNeedSaveSpectr(bool value);
  void setIsNeedMatrixShow(bool value);
  void setIsNeedChangeExposition(bool value);
  void setIsNeedSaveBlackSpectr(bool value);
  void setIsNeedMinusBlackSpectr(bool value);
  void setSavedSpectrsCounter(int value);
  void setIsNeedCalculateMean(bool isNeedCalculateMean);
  void setStartSpectrLine(int startSpectrLine);
  void setEndSpectrLine(int endSpectrLine);
  void changeSensorMode(sensorMode mode);
  void saveSpectrTest();

  QString getSerialNumber() const;
  double  getExposition() const;
  double  getBlackExposition() const;
  int     getNumberOfHorizontalPixels() const;
  int     getNumberOfVerticalPixels() const;
  bool    getIsSensorConnected() const;
  void    setFileNameMetaData(const QString& fileNameMetaData);

  int getCounterSavedSpectrs() const;

 signals:
  void requestSpectr();
  void blackExpositionChanged(double*);
  void savedSpectrCounterChanged(int);
  void spectralDataRecieved(QVector<double> sumSpectr, double maxValue, double minValue);
  void gettingSpectrError();
  void darkSignalWasSaved();

 private:
  HamamatsuApi* m_sensor;
  QString m_serialNumber;
  double m_exposition;
  double m_blackExposition;
  double m_maxValueInSpectr;
  double m_minValueInSpectr;
  int  m_counterSavedSpectrs;
  int  m_horizontalPixels;
  int  m_verticalPixels;
  int  m_startSpectrLine;
  int  m_endSpectrLine;
  bool m_isConnected;
  bool m_isNeedSaveBlackSpectr;
  bool m_isNeedMinusBlackFromSpectr;
  bool m_isNeedSave;
  bool m_isNeedCalculateMean;
  bool m_isNeedChangeExposition;
  bool m_isNeedMatrixShow;
  bool isNeedToChangeSensorMode;
  sensorMode m_modeToChange;
  QVector <double> m_spectralData;
  QVector <double> m_matrixData;
  QVector <double> m_blackSpectralData;
  QString m_summingSpectralData;
  QString m_pathToSaveDir;
  QString m_fileNameMetaData;
  void saveSpectr();

 public slots:
  void finishWork();

 private slots:
  void shootSpectrum();


};

#endif // ORMINDEVICE_H

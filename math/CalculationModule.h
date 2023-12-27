#ifndef CALCULATIONMODULE_H
#define CALCULATIONMODULE_H
#include <QVector>
#include <QObject>
#include "../devices/stm_spectrometr/SpectrDataSaver.h"

class CalculationModule : public QObject {
  Q_OBJECT
 public:
  CalculationModule(SpectrDataSaver* spectrDataSaver);

  ~CalculationModule();

  void setWavelenth(const QVector<double>& wavelenth);

  void appendData4MeanCalculating(QVector<double>& spectrData);

  /**
   * @brief countInSeries Function to get count in series shooting
   * @return  Count in series shooting
   */
  int countInSeries() const;

  /**
   * @brief setSpectrMode Function to set spectrometer shooting mode
   * @param spectrMode    Needed spectrometer shooting mode
   */
  void setSpectrMode(const SpectrMode& spectrMode);

  QList<DarkSpectrum> darkSpectrumsList() const;

  void setDarkSpectrumsCount(int darkSpectrumsCount);

  void setIsDarkSpectrumsNow(bool isDarkSpectrumsNow);

  void appendDataToDarkSpectrumsList(DarkSpectrum spectrumData);

  void setCurrentMetaInfo(QString metaInfo);

  void clearDarkSpectrumsList();

  void setCurrentExposition(double currentExposition);

  static QString formStringFromSpectrum(Spectrum& spectrum);

 public slots:
  void setCountInSeries(SpectrometerSettingsType type, QVariant var);

  /**
   * @brief setCountInSeries  Function to set count in series shooting
   * @param count Count in series shooting
   */
  void setCountInSeries(int count);

 signals:
  void need2SaveCoefficient(QString coefName, QVector <double> spectrData);

  void coefficientCalculated(QString coefName, QVector <double> spectrData);

  void sendMessage(QString);

  void plotGraph(Spectrum spectrum);

 private:
  void calcMeanSpectrumAndSave();

  void calcMeanComboSpectrumAndSave();

  void calcMeanDistrSpectrumAndSave();

  void calcMeanDiffDistrSpectrumAndSave();

  void calcMeanDiffDistrWithRemovingAndSave();

  double calcPixelValue(QVector<double> signalVector, QVector<double> darkVector);

  double calcMean(QVector<double> vector);

  double calcStd(QVector<double> vector, double meanVal);

  void saveStringBySaver(QVector<double>& spectrData, QString baseFileName);

  QString formStringFromVector(QVector<double>& spectrData);

  QVector<double> divideFirstForSecond(QVector<double> spec, QVector<double> milkSpec, QString& specString);

  Spectrum divideFirstForSecond(Spectrum spec, Spectrum milkSpec);

  void filterData();

  void validateData();

  double valueAtWave(double wave);

  QVector<double> m_wavelength;

  QVector <double> m_spectrDataCoefficient;
  QList<Spectrum> m_reflectanceList;                  //!< Список КСЯ
  QList<DarkSpectrum> m_darkSpectrumsList;            //!< Список темновых спектров

  bool m_isDarkSpectrumsNow;
  int m_countInSeries;
  int m_darkSpectrumsCount;
  QList<QVector<double>> m_spectrDataList;
  SpectrMode m_spectrMode;
  SpectrDataSaver* m_spectrDataSaver;         //!< Spectrum Data Saver Object

  double m_currentExposition;
  QString m_currentMetaInfo;
};

#endif // CALCULATIONMODULE_H

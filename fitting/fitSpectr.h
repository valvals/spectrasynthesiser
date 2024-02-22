#ifndef FITSPECTR_H
#define FITSPECTR_H
#include <QRunnable>
#include <QObject>
#include <atomic>
#include "fitting/dataStructs.h"

// Функция подгонки спектра сферы к спектру эталона.
// Возвращает вектор с параметрами слайдеров ламп от 0 до 1
QVector<double> find_diod_spea_coefs(const QVector<double>& wavesEtalon,
                                     const QVector<double>& speyaEtalon,
                                     double waveStep,
                                     const QVector<lampInfo>& lampsAll,
                                     const FitSettings& settings);

QVector<double> emuleFullSpectr(double* params,
                                const QVector<lampInfo>& lamps,
                                const QVector<double>& wavesEtalon,
                                double wavesStep);

QVector<double> find_sliders_from_coefs(const QVector<double>& speyaCoefs,
                                        const QVector<lampInfo>& lamps);

QVector<double> fillCenterLines(const QVector<lampInfo>& lamps);

class fitterBySpectometer : public QObject, public QRunnable {
  Q_OBJECT
 public:
  static int objectCount;
  fitterBySpectometer(const QVector<double>& defaultSliders,
                      const QVector<double>& wavesEtalon,
                      const QVector<double>& speyaEtalon,
                      double waveStep,
                      const QVector<lampInfo>& lampsAll,
                      const FitSettings& settings,
                      QVector<double>* realSpectrPtr,
                      QVector<double>* optimalSlidersPtr,
                      std::atomic<bool>* needNewSpectrPtr,
                      std::atomic<bool>* needUpdateSlidersPtr,
                      double finite_derivative_step);
  void run() override ;

  static std::atomic<bool> isBlocked;

 signals:
  void workIsFinished();

 private:
  static int fitFunctRealSpectrometer(int m, int n, double* p, double* dy, double** /*dvec*/, void* vars);
  static std::atomic<bool>* m_needNewSpectrPtr;
  static std::atomic<bool>* m_needUpdateSlidersPtr;
  static QVector<double>* m_realSpectrPtr;
  static QVector<double>* m_optimalSlidersPtr;
  QVector<double> defaultSliders;
  QVector<double> wavesEtalon;
  QVector<double> speyaEtalon;
  double waveStep;
  double m_finite_derivative_step;
  QVector<lampInfo> lampsAll;
  FitSettings settings;


};


#endif // FITSPECTR_H

#ifndef FITSPECTR_H
#define FITSPECTR_H
#include "dataStructs.h"

// Функция подгонки спектра сферы к спектру эталона.
// Возвращает вектор с параметрами слайдеров ламп от 0 до 1
QVector<double> find_diod_spea_coefs(const QVector<double>& wavesEtalon, const QVector<double>& speyaEtalon, double waveStep,
                  const QVector<lampInfo>& lampsAll, const FitSettings& settings);

QVector<double> emuleFullSpectr(double *params, const QVector<lampInfo> &lamps, const QVector<double> &wavesEtalon, double wavesStep );

QVector<double> find_sliders_from_coefs(const QVector<double>& speyaCoefs, const QVector<lampInfo> &lamps);

QVector<double> fillCenterLines(const QVector<lampInfo> &lamps);
#endif // FITSPECTR_H

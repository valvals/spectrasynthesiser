#include <algorithm>
#include <QDebug>
#include "fitSpectr.h"
#include "mpfit.h"
#include <algorithm>

//! структура с данными для передачи в fitFunct
struct vars_struct {
  double* ey; //! погрешность измерений
  QVector<double> wavesEtalon;
  QVector<double> wavesOfCentersLampLines; // длины волн (центральные) каждого светодиода.
  QVector<double> wavesEtalonShort; // только для максимумов светодиодов
  QVector<double> speyaEtalonShort; // только для максимумов светодиодов
  QVector<double> indexesOfWavesEtalonShort; // индексы длин волн максимумов светодиодов в wavesEtalon
  double waveStep;
  QVector<double> speyaEtalon;
  QVector<lampInfo> lamps;
  FitSettings settings;
};

// Вычисляем длины волн (центральные) каждого светодиода
QVector<double> fillCenterLines(const QVector<lampInfo>& lamps) {
  QVector<double> centerWaves(lamps.size(), -1);
  for (int i = 0; i < lamps.size(); ++i) {
    auto maxIt = std::max_element(lamps.at(i).speya.constBegin(), lamps.at(i).speya.constEnd());
    int index = std::distance(lamps.at(i).speya.constBegin(), maxIt);
    double waveCenter = lamps.at(i).waves.at(index);
    centerWaves[i] = waveCenter;
  }
  return centerWaves;
}

//! Функция создания эталонного спектра с длинами волн только такими, как длины волн светодиодов
void createShortEtalon(const QVector<double>& wavesEtalon,
                       const QVector<double>& speyaEtalon,
                       const QVector<double>& lampCenterWaves,
                       QVector<double>& out_wavesEtalonShort,
                       QVector<double>& out_speyaEtalonShort,
                       QVector<double>& out_indexesOfWavesEtalonShort) {
  out_wavesEtalonShort.clear();
  out_speyaEtalonShort.clear();
  out_indexesOfWavesEtalonShort.clear();
  for (int i = 0; i < lampCenterWaves.size(); ++i) {
    auto it = std::find(wavesEtalon.begin(), wavesEtalon.end(), lampCenterWaves.at(i));
    int index = std::distance(wavesEtalon.begin(), it);
    if (it != wavesEtalon.end()) {
      out_wavesEtalonShort.append(wavesEtalon[index]);
      out_speyaEtalonShort.append(speyaEtalon[index]);
      out_indexesOfWavesEtalonShort.append(index);
    }
  }
}

//! Эта функция суммирует независимые спектры светодиодов в один спектр
QVector<double> emuleFullSpectr(double* params,
                                const QVector<lampInfo>& lamps,
                                const QVector<double>& wavesEtalon,
                                double wavesStep) {
  QVector<double> spectr(wavesEtalon.size(), 0);

  for (int i = 0; i < lamps.size(); ++i) {
    // вычисляем номер самого первого канала относительно wavesEtalon ,в котором появляется спектр светодиода.
    // round() - на всякий случай, для произвольных wavesStep. Когда wavesStep = 1 нм - все точно будет работать.
    int chanNumLampStart = round((lamps.at(i).waves.at(0) - wavesEtalon.at(0)) / wavesStep);   // это значение может оказаться и меньше 0. Так и надо
    if (chanNumLampStart > wavesEtalon.size() - 1)
      continue;
    // номер последнего канала, в котором есть спектр светодиода
    int chanNumFinish = chanNumLampStart + lamps.at(i).waves.size() - 1;
    if (chanNumFinish > wavesEtalon.size() - 1)
      chanNumFinish = wavesEtalon.size() - 1;
    int chanNumLampStartReal = chanNumLampStart;
    if (chanNumLampStart < 0)
      chanNumLampStartReal = 0;
    for (int j = chanNumLampStartReal; j <= chanNumFinish; ++j) {
      // в данном случае у нас lamps.at(j).speya - это СПЭЯ для положения коэффициента при СПЭЯ светодиода = 1, т.е. максимального СПЭЯ светодиода. Регулируем его с помощью params
      spectr[j] += lamps.at(i).speya.at(j - chanNumLampStart) * params[i];
    }
  }
  return spectr;
}

//! Функция подгонки моделированного спектра к эталонному
int fitFunct(int m, int /*n*/, double* p, double* dy, double** /*dvec*/, void* vars) {
  vars_struct* mydata = static_cast<vars_struct*>(vars);
  const QVector<double>& x = mydata->wavesEtalon;
  const QVector<double>& y = mydata->speyaEtalon;
  double* ey = mydata->ey;
  QVector<double> emulatedSpectr = emuleFullSpectr(p, mydata->lamps, x, mydata->waveStep);

  if (mydata->settings == FitSettings::FIT_ALL) {
    for (int i = 0; i < m; i++) {
      dy[i] = (y[i] - emulatedSpectr[i]) / ey[i];
    }
  } else if (mydata->settings == FitSettings::FIT_BY_MAXIMUMS) {
    Q_ASSERT(m == mydata->wavesEtalonShort.size());

    //нужен шорт emulatedSpectr. Для этого из emulatedSpectr надо выбрать только нужные значения
    QVector<double> emulatedSpectrShort(mydata->wavesEtalonShort.size(), -1);
    for (int i = 0; i < mydata->wavesEtalonShort.size(); ++i) {
      emulatedSpectrShort[i] = emulatedSpectr.at(mydata->indexesOfWavesEtalonShort.at(i));
    }

    for (int i = 0; i < m; i++) {
      dy[i] = (mydata->speyaEtalonShort[i] - emulatedSpectrShort[i]) / ey[i];
    }
  }

  return 0;
}


QVector<double> find_diod_spea_coefs(const QVector<double>& wavesEtalon, const QVector<double>& speyaEtalon,
                                     double waveStep, const QVector<lampInfo>& lampsAll, const FitSettings& settings) {
  QVector<lampInfo> lamps;
  QVector<bool> usedLampsAll; // использована лампа или нет
  if (settings == FitSettings::FIT_BY_MAXIMUMS) {
    //Здесь анализируем lampsAll и выкидываем те, которые выходят за границу длин волн эталона.
    //Иначе число переменных будет больше, чем число длин волн и mpfit выкинет с ошибкой
    QVector<double> allLampCenterWaves = fillCenterLines(lampsAll);

    for (int i = 0; i < allLampCenterWaves.size(); ++i) {
      // если центр лампы внутри диапазона длин волн эталона
      if (std::find(wavesEtalon.begin(), wavesEtalon.end(), allLampCenterWaves.at(i)) != wavesEtalon.end()) {
        lamps.append(lampsAll[i]);
        usedLampsAll.append(true);
      } else {
        usedLampsAll.append(false); // сохранить индексы выкинутых lampsAll, чтобы потом смочь создать выходной массив параметров слайдеров
      }
    }
  } else if (settings == FitSettings::FIT_ALL) {
    lamps = lampsAll;
  }

  int specChannels = wavesEtalon.size();
  int lampNums = lamps.size();
  Q_ASSERT(lampNums > 0);
  Q_ASSERT(specChannels == speyaEtalon.size());
  Q_ASSERT(specChannels > 0);
  Q_ASSERT(specChannels > lampNums); //требования метода оптимизации по МНК
  Q_ASSERT((wavesEtalon.last() - wavesEtalon.first()) / (specChannels - 1) == waveStep); //проверяем на константный шаг
  for (const auto& lamp : lamps) {
    Q_ASSERT(lamp.waves.size() > 0);
    Q_ASSERT(lamp.waves.size() == lamp.speya.size());
    Q_ASSERT((lamp.waves.last() - lamp.waves.first()) / (lamp.waves.size() - 1) == waveStep);
  }

  //-------------- Заполняем параметры для mpfit() -------

  //-params
  double* params = new double[lampNums];
  double defValSlider = 0.7;
  std::fill_n(params, lampNums, defValSlider); // default values

  //-pars
  mp_par* pars = new mp_par[lampNums]; // это коэффициенты при СПЭЯ светодиода.
  memset(pars, 0, lampNums * sizeof(mp_par));
  for (int i = 0; i < lampNums; ++i) {
    pars[i].limited[0] = 1;
    pars[i].limited[1] = 1;
    pars[i].limits[0] = 0; // нижняя граница значения коэффициента при СПЭЯ светодиода
    pars[i].limits[1] = 1; // верхняя граница значения коэффициента при СПЭЯ светодиода
  }

  //-mydata
  vars_struct mydata;
  double defValError = 0.1;

  // Реализуем здесь вариант с подгонкой спектров только по максимумам.
  if (settings == FitSettings::FIT_BY_MAXIMUMS) {
    mydata.wavesOfCentersLampLines = fillCenterLines(lamps);
    createShortEtalon(wavesEtalon, speyaEtalon, mydata.wavesOfCentersLampLines,
                      mydata.wavesEtalonShort, mydata.speyaEtalonShort, mydata.indexesOfWavesEtalonShort);
    specChannels = mydata.wavesEtalonShort.size();
  }

  double* ey = new double[specChannels];
  std::fill_n(ey, specChannels, defValError);
  mydata.ey = ey;
  mydata.lamps = lamps;
  mydata.settings = settings;
  mydata.speyaEtalon = speyaEtalon;
  mydata.wavesEtalon = wavesEtalon;
  mydata.waveStep = waveStep;

  //-result
  mp_result result;
  memset(&result, 0, sizeof(result));
  double* perror = new double[lampNums];
  result.xerror = perror;

  //-------------- Фитируем и находим параметры коэффициентов при СПЭЯ светодиодов -------
  int status = mpfit(fitFunct, specChannels, lampNums, params, pars,
                     0, (void*) &mydata, &result);
  qDebug() << "status code: " << status;

  QVector<double> diodSPEAcoefs(params, params + lampNums);

  if (settings == FitSettings::FIT_BY_MAXIMUMS) {
    diodSPEAcoefs.clear();
    for (int i = 0; i < lampsAll.size(); ++i) {
      if (usedLampsAll.at(i) == true)
        diodSPEAcoefs.append(params[i]);
      else
        diodSPEAcoefs.append(0);
    }
  }

  for (int  i = 0; i < usedLampsAll.size(); ++i) {
    qDebug() << "коэффициент при СПЭЯ светодиодов" << i << " = " << diodSPEAcoefs[i];
  }
  delete[] pars;
  delete[] params;
  delete[] ey;
  delete[] perror;
  return diodSPEAcoefs;
}

QVector<double> find_sliders_from_coefs(const QVector<double>& speyaCoefs, const QVector<lampInfo>& lamps) {
  Q_ASSERT(speyaCoefs.size() == lamps.size());
  QVector<double> sliderVals(speyaCoefs.size(), -1);
  for (int i = 0; i < lamps.size(); ++i) {
    if (speyaCoefs.at(i) == 0) {
      sliderVals[i] = 0;
      qDebug() << "светодиод " << i << " в подгонке не участвовал";
    } else {
      // считаем максимальное значение в СПЭЯ светодиода Smax
      QVector<double> spea = lamps.at(i).speya;
      double max = *std::max_element(spea.constBegin(), spea.constEnd());

      // находим корень квадратного уравнения. a b c - коэффициенты квадратного уравнения (не путать с коэфффициентами квадратичной зависимости у диода)
      double a = lamps.at(i).a;
      double b = lamps.at(i).b;
      double c = lamps.at(i).c - speyaCoefs.at(i) * max;

      double D = b * b - 4 * a * c;
      double root1 = (-b + sqrt(D)) / (2 * a);
      double root2 = (-b - sqrt(D)) / (2 * a);


      double sliderVal = 0;
      if (root1 >= 0 && root1 <= 1)
        sliderVal = root1;
      else if (root2 >= 0 && root2 <= 1)
        sliderVal = root2;
      qDebug() << "для светодиода " << i << " найдено 2 корня: " << root1 << " и " << root2 << ". Установили значение " << sliderVal;
      sliderVals[i] = sliderVal;
    }
  }
  for (int  i = 0; i < lamps.size(); ++i) {
    qDebug() << "слайдер для светодиода " << i << " = " << sliderVals.at(i);
  }
  return sliderVals;
}

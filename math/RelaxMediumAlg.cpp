#include "RelaxMediumAlg.h"

RelaxMediumAlg::RelaxMediumAlg() {

}

RelaxMediumAlg::~RelaxMediumAlg() {

}

QVector<double> RelaxMediumAlg::getFilteredSpectrum(QVector<double>& baseData, int smoothLevel) {
  double Weight = (double) smoothLevel / 100;

  double* spectrMas = new double [baseData.size()];
  int i = 0;
  foreach (double value, baseData) {
    spectrMas[i] = value;
    i++;
  }

  double* resultSpectr = new double[baseData.size()];
  relaxMedium(spectrMas, resultSpectr, baseData.size(), Weight);
  QVector<double> resSpectr;
  for (i = 0; i < baseData.size(); i++) {
    resSpectr.append(resultSpectr[i]);
  }
  delete [] spectrMas;
  delete [] resultSpectr;

  return resSpectr;
}

void RelaxMediumAlg::relaxFrontFilter(double* Source, double* Result, int Size, double Weight) {
  if (Size == 0)
    return;
  double* p_Result = Result;
  *Result++ = *Source++;
  for (int i = 1; i < Size; i++, Source++, p_Result++, Result++)
    *Result = *p_Result * Weight + *Source * (1 - Weight);
}

void RelaxMediumAlg::relaxBackFilter(double* Source, double* Result, int Size, double Weight) {
  if (Size == 0)
    return;
  Source += Size - 1;
  Result += Size - 1;
  double* p_Result = Result;
  *Result-- = *Source--;
  for (int i = 1; i < Size; i++, Source--, p_Result--, Result--)
    *Result = *p_Result * Weight + *Source * (1 - Weight);
}

void RelaxMediumAlg::relaxMedium(double* Source, double* Result, int Size, double Weight) {
  if (Size == 0)
    return;
  relaxFrontFilter(Source, Result, Size, Weight);
  double* res = new double[Size];
  relaxBackFilter(Source, res, Size, Weight);
  double* p_res = res;
  for (int i = 0; i < Size; i++, p_res++, Result++)
    *Result = (*Result + *p_res) / 2;
  delete[] res;
}

#ifndef RELAXFILTER_H
#define RELAXFILTER_H
#include <QVector>

//! релаксационный фильтр с проходом от начала к концу
void RelaxFrontFilter(double* Source, double* Result, int Size, double Weight);

//! релаксационный фильтр с проходом от конца к началу
void RelaxBackFilter(double* Source, double* Result, int Size, double Weight);

//! Усредненный медианный фильтр
void RelaxMedium(double* Source, double* Result, int Size, double Weight);

QVector<double> smoothRelax(const QVector<double>& sourceVec, double percent);



#endif // RELAXFILTER_H

#include "fitSpectr.h"
void RelaxFrontFilter(double *Source, double *Result, int Size, double Weight) {
    if (Size == 0)
        return;
    double* p_Result = Result;
    *Result++ = *Source++;
    for (int i = 1; i < Size; i++, Source++, p_Result++, Result++)
        *Result = *p_Result * Weight + *Source * (1 - Weight);
}

void RelaxBackFilter(double *Source, double *Result, int Size, double Weight) {
    if (Size == 0)
        return;
    Source += Size - 1;
    Result += Size - 1;
    double* p_Result = Result;
    *Result-- = *Source--;
    for (int i = 1; i < Size; i++, Source--, p_Result--, Result--)
        *Result = *p_Result * Weight + *Source * (1 - Weight);
}

void RelaxMedium(double *Source, double *Result, int Size, double Weight) {
    if (Size == 0)
        return;
    RelaxFrontFilter(Source, Result, Size, Weight);
    double* res = new double[Size];
    RelaxBackFilter(Source, res, Size, Weight);
    double* p_res = res;
    for (int i = 0; i < Size; i++, p_res++, Result++)
        *Result = (*Result + *p_res) / 2;
    delete[] res;
}

QVector<double> smoothRelax(const QVector<double> &sourceVec, double percent) {
    double* result = new double[sourceVec.size()];
    QVector<double> sourceVecForAlgoritm = sourceVec;

    RelaxMedium(sourceVecForAlgoritm.data(), result, sourceVec.size(), percent / 100.0);
    QVector<double> resVec(result, result + sourceVec.size());
    delete[] result;
    return resVec;
}

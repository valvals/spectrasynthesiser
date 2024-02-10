#ifndef DATASTRUCTS_H
#define DATASTRUCTS_H
#include <QVector>

// waves и speya - длины волн и СПЭЯ самого светодиода в окрестностях его гауусовой формы (например только в интревале 500-550 нм)
// waves с шагом в 1 нм
struct lampInfo{
    QVector<double> waves;
    QVector<double> speya;
    double a, b, c; // калибровочные коэффициенты квадратичной зависимости яркости (СПЭЯ) от значения слайдера (0-1)
};

//FIT_ALL - подгоняем по минимуму СКО для всег оспектра
//FIT_BY_MAXIMUMS - в идеале эталонный спектр должен быть огибающей кривой для гаусовских спектров светодиодов
enum class FitSettings {FIT_ALL, FIT_BY_MAXIMUMS};



#endif // DATASTRUCTS_H

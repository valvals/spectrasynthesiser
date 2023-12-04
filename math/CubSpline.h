#ifndef CUBSPLINE_H
#define CUBSPLINE_H
#include <Windows.h>

/*!
    \brief Класс является модулем для интерполяции

    Данный класс предназначен для выполнения кубической сплайн-интерполяции данных
*/
class CubSpline
{
public:
    //! Конструктор
    CubSpline(double *x, double *y, int count);

    //! Деструктор
    ~CubSpline();

    //! Функция получения значения спектра в точке
    //! @param x - координата по оси абсцисс
    //! @return - значение спектра в точке х
    double GetY(double x);

    double GetLSPoint(double *x, double *y, int n, double exp);

private:
    void solvetridiagonal(double *a, double *b, double *c, double *d, int n, double *x);

    double *c;         //!< Коэффициенты
};

#endif

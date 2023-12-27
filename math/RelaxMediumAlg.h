#ifndef RELAXMEDIUMALG
#define RELAXMEDIUMALG

#include <QElapsedTimer>
#include <QApplication>
#include <QVector>

/**
 * @brief The RelaxMediumAlg class
 * Class needed to apply filtration algorythm to spectrum data
 */
class RelaxMediumAlg : public QObject {
  Q_OBJECT

 public:
  /**
   * @brief RelaxMediumAlg    Constructor
   */
  RelaxMediumAlg();
  virtual ~RelaxMediumAlg();

 public:
  /**
   * @brief getFilteredSpectrum   Function to get filtered spectrum from data vector
   * @param baseData  Base data vector
   * @param smoothLevel   Smooth level
   * @return  Filtered spectrum
   */
  static QVector<double> getFilteredSpectrum(QVector<double>& baseData, int smoothLevel);

 private:
  /**
   * @brief relaxFrontFilter  Relax filter with moving from the start to the end
   * @param Source    Source data
   * @param Result    Result data
   * @param Size  Data size
   * @param Weight    Smooth value
   */
  static void relaxFrontFilter(double* Source, double* Result, int Size, double Weight);

  /**
   * @brief relaxBackFilter   Relax filter with moving from the end to the start
   * @param Source    Source data
   * @param Result    Result data
   * @param Size  Data size
   * @param Weight    Smooth value
   */
  static void relaxBackFilter(double* Source, double* Result, int Size, double Weight);

  //! Усредненный медианный фильтр
  /**
   * @brief relaxMedium   Relax filter with averaging
   * @param Source    Source data
   * @param Result    Result data
   * @param Size  Data size
   * @param Weight    Smooth value
   */
  static void relaxMedium(double* Source, double* Result, int Size, double Weight);

};

#endif // RELAXMEDIUMALG


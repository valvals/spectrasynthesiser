#ifndef DATASTRUCTS_H
#define DATASTRUCTS_H
#include <QVector>

// waves и speya - длины волн и СПЭЯ самого светодиода в окрестностях его гауусовой формы (например только в интревале 500-550 нм)
// waves с шагом в 1 нм
struct lampInfo {
  QVector<double> waves;
  QVector<double> speya;
  double a, b, c; // калибровочные коэффициенты квадратичной зависимости яркости (СПЭЯ) от значения слайдера (0-1)
  int max_slider_value;
};

//FIT_ALL - подгоняем по минимуму СКО для всег оспектра
//FIT_BY_MAXIMUMS - в идеале эталонный спектр должен быть огибающей кривой для гаусовских спектров светодиодов
enum class FitSettings {FIT_ALL, FIT_BY_MAXIMUMS};

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

class WorkerThread : public QThread {
  Q_OBJECT

 public:
  WorkerThread() : m_mutex(), m_cond(), m_ready(false) {}

  void run() override {
    // Do some work...
    // ...

    // Signal that the work is done
    m_mutex.lock();
    m_ready = true;
    m_cond.wakeOne();
    m_mutex.unlock();
  }

  void waitForReady() {
    m_mutex.lock();
    while (!m_ready)
      m_cond.wait(&m_mutex);
    m_mutex.unlock();
  }

 private:
  QMutex m_mutex;
  QWaitCondition m_cond;
  bool m_ready;
};


#endif // DATASTRUCTS_H

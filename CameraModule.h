#ifndef CAMERAMODULE_H
#define CAMERAMODULE_H
#include <QCamera>
#include <QLabel>
#include <QCameraImageCapture>
#include <QCameraViewfinder>
#include <QMediaRecorder>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QObject>
#include <QImage>
#include <Windows.h>
#include <QThread>
#include <QCloseEvent>

class CameraViewfinder:public QCameraViewfinder{
   Q_OBJECT
protected:
    void closeEvent(QCloseEvent *event) override{
    emit camera_window_closed();
    event->accept();
    };
signals:
    void camera_window_closed();
};


class CameraModule: public QObject {
  Q_OBJECT
 public:
  CameraModule();
  ~CameraModule();

  void setIsSave(bool value);
  void setRecordingFolder(QString value);
  void setSleepDuration(int value);
  void setRecordFormat(const QString& value);
  void changeGpsStamp(QString gpsCoordinate);

 private slots:
  void setCamera(const QCameraInfo& cameraInfo);
  void startCamera();
  void stopCamera();
  void mayBeShowCamera(bool is_show);
  void imageWasCaptured(int id, const QImage &preview);

signals:
  void cameraWindowClosed();

 private:
  QCamera* camera;
  CameraViewfinder* m_view_finder;
  QCameraImageCapture* m_image_capture;

};

#endif // CAMERAMODULE_H

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
#include "QSharedViewfinder.h"



class CameraModule: public QObject {
  Q_OBJECT
 public:
  CameraModule(QLabel* obzorVideo, QString recordFolder, QString format, FieldOfViewJustCoord fov);
  ~CameraModule();

  void setIsSave(bool value);
  void setRecordingFolder(QString value);
  void setSleepDuration(int value);
  void setRecordFormat(const QString& value);
  void setFieldOfView(FieldOfViewJustCoord fov);
  void changeGpsStamp(QString gpsCoordinate);

 public slots:
  void setCamera(const QCameraInfo& cameraInfo);
  void startCamera();
  void stopCamera();
  void captureJustNecessaryImage(SpectrPosition);

 private slots:
  void showVedeoFrame(QImage vframe);


 signals:
  void send_image(const QImage&);
  void request_next_frame_from_gui();
  void necessarySaveFrame(SpectrPosition);

 private:
  QFile* file;
  QDir default_dir;
  QCamera* camera;
  QLabel* m_labelVideo;
  QThread m_viewFinderThread;
  QSharedViewFinder* m_sharedViewFinder;                    //!< Объект для обработки изображения с камеры
  QString path_for_saving_camera_data;
  int frameSleep;

};

#endif // CAMERAMODULE_H

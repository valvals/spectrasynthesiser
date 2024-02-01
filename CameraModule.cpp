#include "CameraModule.h"
#include <QCamera>
#include <QCameraImageCapture>
#include <QMediaRecorder>
#include <QImage>
#include <QPainter>
#include <QCameraInfo>
#include <QDebug>
#include <QTimer>
#include <QBuffer>

CameraModule::CameraModule(QLabel* label_observationVideo, QString recordFolder, QString format, FieldOfViewJustCoord fov): camera(nullptr) {
  m_labelVideo = label_observationVideo;
  m_sharedViewFinder = new QSharedViewFinder(640, 480, 28, recordFolder, format, fov);
  m_sharedViewFinder->moveToThread(&m_viewFinderThread);
  m_viewFinderThread.start();
  setCamera(QCameraInfo::defaultCamera());
  startCamera();
  qRegisterMetaType<SpectrPosition>();
  connect(this, SIGNAL(necessarySaveFrame(SpectrPosition)), m_sharedViewFinder, SLOT(saveJustNecessaryImage(SpectrPosition)), Qt::DirectConnection);
}

CameraModule::~CameraModule() {


  delete camera;
}

void CameraModule::setCamera(const QCameraInfo& cameraInfo) {
  delete camera;
  camera = new QCamera(cameraInfo);
  QCameraViewfinderSettings settings = camera->viewfinderSettings();
  settings.setResolution(QSize(1280, 720));
  camera->setViewfinderSettings(settings);
  camera->setCaptureMode(QCamera::CaptureStillImage);
  camera->setViewfinder(m_sharedViewFinder);
  QObject::connect(m_sharedViewFinder, SIGNAL(frameReady(QImage)), this, SLOT(showVedeoFrame(QImage)));

}

void CameraModule::startCamera() {
  camera->start();
}

void CameraModule::stopCamera() {
  camera->stop();
  disconnect(m_sharedViewFinder, SIGNAL(frameReady(QImage)), this, SLOT(showVedeoFrame(QImage)));
}

void CameraModule::captureJustNecessaryImage(SpectrPosition specPos) {
  m_sharedViewFinder->setIsSaveNecessaryImage(true);
  //emit necessarySaveFrame(specPos);
}


void CameraModule::showVedeoFrame(QImage vframe) {
  m_labelVideo->setPixmap(QPixmap::fromImage(vframe));
}

void CameraModule::changeGpsStamp(QString gpsCoordinate) {
  m_sharedViewFinder->setGpsCoordinate(gpsCoordinate);
}

void CameraModule::setRecordFormat(const QString& value) {
  m_sharedViewFinder->setSaveFormat(value);
}

void CameraModule::setFieldOfView(FieldOfViewJustCoord fov) {
  m_sharedViewFinder->setFieldOfView(fov);
}

void CameraModule::setSleepDuration(int value) {
  frameSleep = value;
}

void CameraModule::setRecordingFolder(QString value) {
  QString cameraFolder = value;
  cameraFolder.append("/camera");
  QDir dir(value);
  dir.mkdir(cameraFolder);
  m_sharedViewFinder->setRecordingFolder(cameraFolder);
}

void CameraModule::setIsSave(bool value) {
  if (m_sharedViewFinder->getSaveFormat().contains("AVI") && value)
    m_sharedViewFinder->startMakingAviFile();
  if (m_sharedViewFinder->getSaveFormat().contains("AVI") && !value)
    m_sharedViewFinder->stopMakingAviFile();
  m_sharedViewFinder->setIsSave(value);
}

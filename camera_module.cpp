#include "camera_module.h"
#include <QCamera>
#include <QCameraImageCapture>
#include <QMediaRecorder>
#include <QImage>
#include <QPainter>
#include <QCameraInfo>
#include <QDebug>
#include <QTimer>
#include <QBuffer>
#include <QAction>
#include "QClipboard"
#include "QGuiApplication"

CameraModule::CameraModule(): camera(nullptr) {
  m_view_finder = new CameraViewfinder;
  m_view_finder->setWindowFlags(Qt::WindowStaysOnTopHint);
  m_view_finder->setMinimumSize(QSize(200, 200));
  m_view_finder->setContextMenuPolicy(Qt::ActionsContextMenu);
  QAction* action_capture = new QAction;
  action_capture->setText("копировать снимок в буфер обмена");
  m_view_finder->addAction(action_capture);
  /*const QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
  for (const QCameraInfo &cameraInfo : cameras) {
      if (cameraInfo.deviceName() == "mycamera"){
          camera = new QCamera(cameraInfo);
      }
         qDebug()<<cameraInfo.deviceName();
  }*/

  setCamera(QCameraInfo::defaultCamera());
  startCamera();
  connect(m_view_finder, SIGNAL(camera_window_closed()), SIGNAL(cameraWindowClosed()));
  connect(action_capture, SIGNAL(triggered()), m_image_capture, SLOT(capture()));
  connect(m_image_capture, SIGNAL(imageCaptured(int, const QImage&)), SLOT(imageWasCaptured(int, const QImage&)));
}

CameraModule::~CameraModule() {
  delete camera;
}

void CameraModule::setCamera(const QCameraInfo& cameraInfo) {
  if (camera)
    delete camera;
  camera = new QCamera(cameraInfo);
  camera->setViewfinder(m_view_finder);
  m_image_capture = new QCameraImageCapture(camera);
  QCameraViewfinderSettings settings = camera->viewfinderSettings();
  camera->setViewfinderSettings(settings);
  camera->setCaptureMode(QCamera::CaptureStillImage);

}

void CameraModule::startCamera() {
  camera->start();
}

void CameraModule::stopCamera() {
  camera->stop();
}

void CameraModule::mayBeShowCamera(bool is_show) {
  if (is_show) {
    m_view_finder->show();
  } else {
    m_view_finder->hide();
  }
}

void CameraModule::imageWasCaptured(int id, const QImage& preview) {
  //qDebug() << "image size:" << preview.size();
  Q_UNUSED(id)
  auto clip = QGuiApplication::clipboard();
  clip->setImage(preview);
}




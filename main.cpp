#include "SpectraSynthesizer.h"

#include "QCommandLineParser"
#include "QrcFilesRestorer.h"
#include <QApplication>

void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
  Q_UNUSED(context);
  QFile file(QCoreApplication::applicationDirPath() + "//spectrasyn.log");
  if (file.exists())
    file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
  else
    file.open(QIODevice::WriteOnly | QIODevice::Text);
  QString time = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");
  QTextCodec::setCodecForLocale(QTextCodec::codecForName("CP 1251"));
  QString OutMessage = "";
  QTextStream out(&file);

  switch (type) {
    case QtInfoMsg:
      OutMessage = msg;
      break;
    case QtDebugMsg:
      OutMessage = QString("Debug[%1]: %2\n").arg(time, msg);
      break;
    case QtWarningMsg:
      OutMessage = QString("Warning[%1]: %2\n").arg(time, msg);
      break;
    case QtCriticalMsg:
      OutMessage = QString("Critical[%1]: %2\n").arg(time, msg);
      break;
    case QtFatalMsg:
      OutMessage = QString("Fatal[%1]: %2\n").arg(time, msg);
      abort();
  }
  out << OutMessage;
  file.close();

}

int main(int argc, char* argv[]) {

  QrcFilesRestorer::restoreFilesFromQrc(":/jsons");
  QApplication a(argc, argv);

  QSystemSemaphore semaphore("<SPECTRASYN>", 1);
  semaphore.acquire();

  QSharedMemory sharedMemory("<SPECTRASYN 2>");
  bool is_running;
  if (sharedMemory.attach()) {
    is_running = true;
  } else {
    sharedMemory.create(1);
    is_running = false;
  }
  semaphore.release();

  if (is_running) {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("Приложение уже запущено.\nВы можете запустить только один экземпляр приложения.");
    msgBox.exec();
    return 0;
  }


  QCommandLineParser cli;
  QCommandLineOption debugOption(QStringList() << "d" << "debug", "Debug mode");
  cli.addOption(debugOption);
  cli.process(a);
  bool isDebug = cli.isSet(debugOption);
  if (isDebug) {
    qInstallMessageHandler(myMessageOutput);
    qInfo() << "\n\n\n******************** SPECTRASYNTHESIZER **********************\n";
  }
  SpectraSynthesizer w;
  w.show();
  return a.exec();
}

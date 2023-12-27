#include "SpectraSynthesizer.h"

#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  SpectraSynthesizer w;
  w.show();
  return a.exec();
}

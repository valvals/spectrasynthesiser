#include "debug_console.h"
#include "ui_debug_console.h"
#include "QDebug"

DebugConsole::DebugConsole(QWidget* parent) :
  QWidget(parent),
  ui(new Ui::DebugConsole) {
  ui->setupUi(this);
}

DebugConsole::~DebugConsole() {
  delete ui;
}

void DebugConsole::add_message(const QString& msg, dbg::object obj) {
  QTextCursor cursor(ui->textBrowser_debug_console->textCursor());
  QTextCharFormat format;
  format.setFontWeight(QFont::DemiBold);
  QString color;
  switch (obj) {
    case dbg::SOFT:
      color = "red";
      break;
    case dbg::DIODS_CONTROLLER:
      color = "yellow";
      break;
    case dbg::STM_CONTROLLER:
      color = "green";
      break;
  }
  format.setForeground(QBrush(QColor(color)));
  cursor.setCharFormat(format);
  cursor.insertText(msg);
}

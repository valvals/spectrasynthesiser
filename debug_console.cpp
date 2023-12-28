#include "debug_console.h"
#include "ui_debug_console.h"
#include "QDebug"
#include "QAction"
#include "QScrollBar"

DebugConsole::DebugConsole(QWidget* parent) :
  QWidget(parent),
  ui(new Ui::DebugConsole) {
  ui->setupUi(this);
  QAction* clear_console_action = new QAction;
  clear_console_action->setText("очистить консоль");
  ui->textBrowser_debug_console->setContextMenuPolicy(Qt::ContextMenuPolicy::ActionsContextMenu);
  ui->textBrowser_debug_console->addAction(clear_console_action);
  connect(clear_console_action, SIGNAL(triggered()), SLOT(clearConsole()));
}

DebugConsole::~DebugConsole() {
  delete ui;
}

void DebugConsole::add_message(const QString& msg, dbg::object obj) {
  QTextCursor cursor(ui->textBrowser_debug_console->textCursor());
  QTextCharFormat format;
  format.setFontWeight(QFont::Cursive);
  format.setFontPointSize(16);
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
  auto sb = ui->textBrowser_debug_console->verticalScrollBar();
  sb->setValue(sb->maximum());
}

void DebugConsole::clearConsole() {
  ui->textBrowser_debug_console->clear();
}

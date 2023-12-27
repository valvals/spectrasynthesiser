#ifndef DEBUG_CONSOLE_H
#define DEBUG_CONSOLE_H

#include <QWidget>

namespace Ui {
class DebugConsole;
}

namespace dbg{
enum object{
    SOFT,
    DIODS_CONTROLLER,
    STM_CONTROLLER
};
}

class DebugConsole : public QWidget {
  Q_OBJECT

 public:
  explicit DebugConsole(QWidget* parent = nullptr);
  ~DebugConsole();
  void add_message(const QString& msg, dbg::object obj);

 private:
  Ui::DebugConsole* ui;
};

#endif // DEBUG_CONSOLE_H

#ifndef QRC_FILES_RESTORER_H
#define QRC_FILES_RESTORER_H


class QString;

class QrcFilesRestorer {
 public:
  QrcFilesRestorer(const QString& path2Qrc);
  static void restoreFilesFromQrc(const QString& path2Qrc);

};

#endif // QRC_FILES_RESTORER_H

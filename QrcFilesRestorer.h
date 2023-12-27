#ifndef QRCFILESRESTORER_H
#define QRCFILESRESTORER_H
#include "qstring.h"

/*!
    \brief Класс является объектом, необходимым для восстановления файлов в директории с СПО из ресурсов в случае их отсутствия.

    Данный класс предназначен для восстановления файлов в директории с СПО из ресурсов в случае их отсутствия.
*/
class QrcFilesRestorer
{
public:
    QrcFilesRestorer(const QString& path2Qrc);
    static void restoreFilesFromQrc(const QString& path2Qrc);

};

#endif // QRCFILESRESTORER_H

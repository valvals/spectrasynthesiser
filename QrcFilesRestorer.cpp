#include "QrcFilesRestorer.h"
#include <QFile>
#include <QDir>
#include "qdebug.h"

QrcFilesRestorer::QrcFilesRestorer(QString path2Qrc)
{
    QDir dir(path2Qrc);
    QStringList files = dir.entryList();

}

void QrcFilesRestorer::restoreFilesFromQrc(QString path2Qrc)
{
    QDir dir(path2Qrc);
    QStringList files = dir.entryList();

    for(int i=0;i<files.count();++i){
        QString filenameNew = QDir::currentPath()+"/"+ files.at(i);
        QFile file(filenameNew);

        if(!file.exists()){
            QFile::copy(path2Qrc +"/"+files.at(i), filenameNew);
            file.setPermissions(QFileDevice::WriteUser|QFileDevice::ReadUser);
        }
    }
}



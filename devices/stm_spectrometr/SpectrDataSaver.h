#ifndef SPECTRDATASAVER_H
#define SPECTRDATASAVER_H

#include <QObject>
#include <QVector>
#include <QVariant>
#include "Include/CommonTypes.h"

class SpectrDataSaver : public QObject
{
    Q_OBJECT
public:
    explicit SpectrDataSaver(QObject *parent = nullptr);

    void setCounter4Recording(int value);

    QString savingDirectory() const;

    /**
     * @brief setCountInSeries  Function to set count in series shooting
     * @param count Count in series shooting
     */
    void setCountInSeries(int countInSeries);

    int counter4Recording() const;

    void saveInSeparateThread(QString baseFileName, QString strData);

    void saveStringToFile(QString baseFileName, QString *strData);

    static void createFileAndWrite(QString absoluteFileName, QString *strData);

    QString getAbsoluteFileName(QString baseFileName);

    QString getSingleSpectrumName(QString baseFileName);

    QString getSeriesSpectrumName(QString baseFileName);

    QString getComboSpectrumName(QString baseFileName);

    QString getSpectrumNameWithAppendix(QString folderStartName, QString baseFileName);

    void appendNameForSeries(QString &currentAbsoluteName, QString baseFileName);

    void appendNameForComboSeries(QString &currentAbsoluteName, QString baseFileName);

    void checkForDirectoryLevel();

    void upCurrentDirectory();

    void checkDirAndCreateIfNeeded(QString folderPath);

    void setSingleFileName(const QString &singleFileName);

    /**
     * @brief setSpectrMode Function to set spectrometer shooting mode
     * @param spectrMode    Current spectrometer shooting mode
     */
    void setSpectrMode(const SpectrMode &spectrMode);

    void setMilkSpectrumsCount(int milkSpectrumsCount);

    void setIsDarkSpectrumsNow(bool isDarkSpectrumsNow);

    void setDarkSpectrumsCount(int darkSpectrumsCount);

    void setIsMilkSpectrumNow(bool isMilkSpectrumNow);

    void setIsCalculatingReflectanceNow(bool isCalculatingReflectanceNow);

    void clearCurrentSavingDirectory();

    QString getLastSavedFolder() const;

public slots:
    void saveStringToFileLater(QString baseFileName, QString strData);

signals:
    void spectrWasSaved(int);

    void spectrSeriesWasFinished();

    void currentFolderChanged(QString folder);

public slots:
    void setSavingDirectory(SpectrometerSettingsType type, QVariant var);

    void setSavingDirectory(QString savingDir);

private:
    int m_counter4Recording;            //!< Счетчик для одной серии
    int m_countInSeries;                //!< Количество спектров в серии
    int m_darkSpectrumsCounter;         //!< Счетчик для темновых спектров
    int m_darkSpectrumsCount;           //!< Общее количество темновых спектров
    QString m_savingDirectory;          //!< Общая директория для записи
    QString m_currentSavingDirectory;   //!< Текущая директория для записи
    QString m_singleFileName;           //!< Имя файла в случае сохранения не по пути по умолчанию
    QString m_directoryAppendix;        //!< Добавка к пути для названия папки для хранения спектров
    QString m_lastSavedFolder;          //!< Папка для сохранения, использованная в последний раз

    QVector <double> m_bands;           //!< Каналы
    SpectrMode m_spectrMode;            //!< Режим записи спектра
    bool m_isDarkSpectrumsNow;          //!< Производится ли сейчас съемка темновых пикселей
    bool m_isMilkSpectrumNow;           //!< Производится ли сейчас съемка молочного стекла
    bool m_isCalculatingReflectanceNow; //!< находится ли сейчас ПО в режиме вычисления КСЯ
};

#endif // SPECTRDATASAVER_H

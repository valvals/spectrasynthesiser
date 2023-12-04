#ifndef CALIBRATIONMODULE_H
#define CALIBRATIONMODULE_H
#include <QVector>
#include <QObject>
#include <QXmlStreamReader>
#include <QXmlStreamAttribute>

class CalibrationModule : public QObject
{
    Q_OBJECT
public:
    explicit CalibrationModule(QObject *parent = nullptr);

    void loadWavesCalibrationFile(QString filepath);

    void loadSolarExpositionCalibrationFile(QString filepath);

    QVector<double> wavelength() const;

    QList<QVector<double>> calibrationsList() const;

    QList<QPair<int, QVector<double> > > expositionCalibrationsList() const;

    static QVector<double> getDarkSpectrumValues(QString filepath);

signals:
    void sendMessage(QString message);

private:
    QVector<double> m_wavelength;
    QList<QVector<double>> m_calibrationsList;
    QList<QPair<int, QVector<double>>> m_expositionCalibrationsList;
};

#endif // CALIBRATIONMODULE_H

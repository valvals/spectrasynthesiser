#include "CalculationModule.h"
#include "../Math/RelaxMediumAlg.h"
#include "math.h"
#include <QDateTime>
#include "qdebug.h"

CalculationModule::CalculationModule(SpectrDataSaver *spectrDataSaver) : QObject()
{
    m_spectrDataSaver = spectrDataSaver;
    m_countInSeries = 1;
}

CalculationModule::~CalculationModule()
{
}

void CalculationModule::setWavelenth(const QVector<double> &wavelenth)
{
    m_wavelength = wavelenth;
}

void CalculationModule::calcMeanSpectrumAndSave()
{
    QVector<double> spectrData;
    for (int i=0; i < m_wavelength.count(); i++)
    {
        double meanVal = 0;
        for (int j = 0; j < m_spectrDataList.count(); j++){
            meanVal += m_spectrDataList.at(j).at(i);
        }
        meanVal /= m_spectrDataList.count();
        if(meanVal < 0) meanVal = 1;
        spectrData.append(meanVal);
    }
    m_spectrDataList.clear();

    QString baseFileName = QDateTime::currentDateTime().toString("yyyyMMdd_hh_mm_ss_z");
    saveStringBySaver(spectrData, baseFileName);
}

void CalculationModule::calcMeanComboSpectrumAndSave()
{
    QVector<double> spectrData;
    for (int i = 0; i < m_wavelength.count(); i++){
        double meanVal = 0;
        for (int j = 0; j < m_spectrDataList.count(); j++){
            meanVal += m_spectrDataList.at(j).at(i) - m_darkSpectrumsList.at(j).dataSpectrum.at(i);
        }
        meanVal /= m_spectrDataList.count();
        if(meanVal < 0) meanVal = 1;
        spectrData.append(meanVal);
    }
    m_spectrDataList.clear();

    QString baseFileName = QDateTime::currentDateTime().toString("yyyyMMdd_hh_mm_ss_z");
    saveStringBySaver(spectrData, baseFileName);
}

void CalculationModule::calcMeanDistrSpectrumAndSave()
{
    QVector<double> spectrData;

    for(int i = 0; i < m_wavelength.count(); i++){
        QVector<double> pixelValues;
        QVector<double> pixelDarkValues;
        for (int j = 0; j < m_spectrDataList.count(); j++){
            pixelValues.append(m_spectrDataList.at(j).at(i));
            pixelDarkValues.append(m_darkSpectrumsList.at(j).dataSpectrum.at(i));
        }
        qSort(pixelValues);
        qSort(pixelDarkValues);
        for (int k = 0; k < 5; k++){
            pixelValues.removeLast();
            pixelValues.removeFirst();
            pixelDarkValues.removeLast();
            pixelDarkValues.removeFirst();
        }
        double meanVal = 0;
        for (int l = 0; l < pixelValues.count(); l++){
            meanVal += pixelValues.at(l) - pixelDarkValues.at(l);
        }
        meanVal /= pixelValues.count();
        spectrData.append(meanVal);
    }

    QString baseFileName = QDateTime::currentDateTime().toString("yyyyMMdd_hh_mm_ss_z");
    saveStringBySaver(spectrData, baseFileName);
}

void CalculationModule::calcMeanDiffDistrSpectrumAndSave()
{
    QVector<double> spectrData;

    for(int i = 0; i < m_wavelength.count(); i++){
        QVector<double> pixelValues;
        QVector<double> pixelDarkValues;
        for (int j = 0; j < m_spectrDataList.count(); j++){
            pixelValues.append(m_spectrDataList.at(j).at(i));
            pixelDarkValues.append(m_darkSpectrumsList.at(j).dataSpectrum.at(i));
        }
        if( i == 100){
            qDebug()<< "\n\n-----------------------------------\n";
            qDebug()<< "pixel 100:";
            qDebug()<< "signal"<<"dark";
            for(int i = 0; i < pixelValues.count(); i++){
                qDebug()<<pixelValues.at(i)<<pixelDarkValues.at(i);
            }
        }
        spectrData.append(calcPixelValue(pixelValues, pixelDarkValues));
    }

    QString baseFileName = QDateTime::currentDateTime().toString("yyyyMMdd_hh_mm_ss_z");
    saveStringBySaver(spectrData, baseFileName);
}

void CalculationModule::calcMeanDiffDistrWithRemovingAndSave()
{
    QVector<double> spectrData;

    for(int i = 0; i < m_wavelength.count(); i++){
        QVector<double> pixelValues;
        QVector<double> pixelDarkValues;
        for (int j = 0; j < m_spectrDataList.count(); j++){
            pixelValues.append(m_spectrDataList.at(j).at(i));
            pixelDarkValues.append(m_darkSpectrumsList.at(j).dataSpectrum.at(i));
        }
        if( i == 200){
            qDebug()<< "\n\n-----------------------------------\n";
            qDebug()<< "pixel 200:";
            qDebug()<< "signal"<<"dark";
            for(int i = 0; i < pixelValues.count(); i++){
                qDebug()<<pixelValues.at(i)<<pixelDarkValues.at(i);
            }
        }
        qSort(pixelValues);
        qSort(pixelDarkValues);
        for (int k = 0; k < 5; k++){
            pixelValues.removeLast();
            pixelValues.removeFirst();
            pixelDarkValues.removeLast();
            pixelDarkValues.removeFirst();
        }
        spectrData.append(calcPixelValue(pixelValues, pixelDarkValues));
    }

    QString baseFileName = QDateTime::currentDateTime().toString("yyyyMMdd_hh_mm_ss_z");
    saveStringBySaver(spectrData, baseFileName);
}

double CalculationModule::calcPixelValue(QVector<double> signalVector, QVector<double> darkVector)
{
    double result = 0;
    double meanSignal = calcMean(signalVector);
    double stdSignal = calcStd(signalVector, meanSignal);
    double meanDark = calcMean(darkVector);
    double stdDark = calcStd(darkVector, meanDark);

    for (int i = 0; i < signalVector.count(); i++){
        double sigmasInCurrentSignal = fabs(meanSignal - signalVector.at(i))/stdSignal;
        if(darkVector.at(i) < meanDark)
            result += meanDark - stdDark*sigmasInCurrentSignal;
        else
            result += meanDark + stdDark*sigmasInCurrentSignal;
    }
    result /= signalVector.count();

    return result;
}

double CalculationModule::calcMean(QVector<double> vector)
{
    double result = 0;
    for(int i = 0; i < vector.count(); i++)
        result += vector.at(i);
    result /= vector.count();

    return result;
}

double CalculationModule::calcStd(QVector<double> vector, double meanVal)
{
    double result = 0;
    for(int i = 0; i < vector.count(); i++)
        result += pow((vector.at(i) - meanVal), 2);
    result /= vector.count();

    return sqrt(result);
}

void CalculationModule::saveStringBySaver(QVector<double> &spectrData, QString baseFileName)
{
    QString specString;
    specString.append(m_currentMetaInfo + "\n");
    specString.append(formStringFromVector(spectrData));
    m_spectrDataSaver->setCounter4Recording(-1);
    m_spectrDataSaver->saveStringToFile(baseFileName, &specString);
}

QString CalculationModule::formStringFromVector(QVector<double> &spectrData)
{
    QString specString;
    for (int i = 0; i < spectrData.count(); i++){
        if(m_wavelength.isEmpty() || m_wavelength.count() != spectrData.count())
            specString.append(QString::number(i)+"\t"+QString::number(spectrData[i])+"\n");
        else
            specString.append(QString::number(m_wavelength.at(i))+"\t"+QString::number(spectrData[i])+"\n");
    }
    return specString;
}

QString CalculationModule::formStringFromSpectrum(Spectrum &spectrum)
{
    QString specString;
    specString.append("Name:\t" + spectrum.name + "\n");
    if(spectrum.bandsValues == WAVELENGTH)
        specString.append("Bands:\tWavelength, Nm\n");
    else
        specString.append("Bands:\tband numbers\n");

    if(spectrum.graphValues == RFL_VALUES)
        specString.append("Values:\tReflectance\n\n");
    else
        specString.append("Values:\tADC\n\n");

    for (int i = 0; i < spectrum.dataSpectrum.count(); i++){
        specString.append(QString::number(spectrum.wavesSpectrum.at(i)) + "\t" +
                          QString::number(spectrum.dataSpectrum.at(i)) + "\n");
    }
    return specString;
}

QVector<double> CalculationModule::divideFirstForSecond(QVector<double> spec, QVector<double> milkSpec, QString &specString)
{
    QVector<double> result;
    for (int i = 0; i < spec.count(); i++){
        double coef = spec.at(i)/milkSpec.at(i);
        result.append(coef);
        if(m_wavelength.isEmpty())
            specString.append(QString::number(i+1)+"\t"+QString::number(coef)+"\n");
        else
            specString.append(QString::number(m_wavelength.at(i))+"\t"+QString::number(coef)+"\n");
    }
    return result;
}

Spectrum CalculationModule::divideFirstForSecond(Spectrum spec, Spectrum milkSpec)
{
    Spectrum result;
    result.name = spec.name;
    result.exposition = spec.exposition;
    result.wavesSpectrum = spec.wavesSpectrum;
    QVector<double> resVector;
    for (int i = 0; i < spec.dataSpectrum.count(); i++){
        double coef;
        if(fabs(milkSpec.dataSpectrum.at(i)) < 0.001)
            coef = spec.dataSpectrum.at(i);
        else
            coef = spec.dataSpectrum.at(i)/milkSpec.dataSpectrum.at(i);

        resVector.append(coef);
    }
    result.dataSpectrum = resVector;
    return result;
}

void CalculationModule::appendData4MeanCalculating(QVector<double> &spectrData)
{
    m_spectrDataList.append(spectrData);
    if(m_spectrDataList.count() == m_countInSeries){
        if(m_spectrMode != COMBO_MODE)
            calcMeanSpectrumAndSave();
        else
//            calcMeanDiffDistrWithRemovingAndSave();
//            calcMeanDiffDistrSpectrumAndSave();
//            calcMeanDistrSpectrumAndSave();
            calcMeanComboSpectrumAndSave();
    }
}

void CalculationModule::filterData()
{
    RelaxMediumAlg filtrationModule;

    m_spectrDataCoefficient = filtrationModule.getFilteredSpectrum(m_spectrDataCoefficient, 60);
}

void CalculationModule::validateData()
{
    if(m_wavelength.isEmpty()){
        emit sendMessage("Невозможно валидировать данные: калибровочный файл не задан!");
    }else{
        for(int i = 0; i < m_wavelength.count(); i++){
            if(m_wavelength.at(i) >= 400 && m_wavelength.at(i) <= 900){
                if(m_spectrDataCoefficient.at(i) > 0.9){
                    emit sendMessage("Ошибка: значения полученного коэффициента не корректны!");
                    return;
                }
            }
        }
        emit sendMessage("Данные валидны!");
    }
}

double CalculationModule::valueAtWave(double wave)
{
    double minDifference = 5000;
    for(int i = 1; i < m_wavelength.count(); i++){
        double currDifference = fabs(m_wavelength.at(i) - wave);
        if(currDifference < minDifference)
            minDifference = currDifference;
        else
            return m_spectrDataCoefficient.at(i-1);
    }
    return -1;
}

void CalculationModule::setCurrentExposition(double currentExposition)
{
    m_currentExposition = currentExposition;
}

void CalculationModule::setIsDarkSpectrumsNow(bool isDarkSpectrumsNow)
{
    m_isDarkSpectrumsNow = isDarkSpectrumsNow;
    m_spectrDataSaver->setIsDarkSpectrumsNow(m_isDarkSpectrumsNow);
}

void CalculationModule::appendDataToDarkSpectrumsList(DarkSpectrum spectrumData)
{
    m_darkSpectrumsList.append(spectrumData);
}

void CalculationModule::setCurrentMetaInfo(QString metaInfo)
{
    m_currentMetaInfo = metaInfo;
}

void CalculationModule::clearDarkSpectrumsList()
{
    m_darkSpectrumsList.clear();
}

void CalculationModule::setDarkSpectrumsCount(int darkSpectrumsCount)
{
    m_darkSpectrumsCount = darkSpectrumsCount;
    m_spectrDataSaver->setDarkSpectrumsCount(m_darkSpectrumsCount);
}

QList<DarkSpectrum> CalculationModule::darkSpectrumsList() const
{
    return m_darkSpectrumsList;
}

void CalculationModule::setSpectrMode(const SpectrMode &spectrMode)
{
    m_spectrMode = spectrMode;
}

void CalculationModule::setCountInSeries(SpectrometerSettingsType type, QVariant var)
{
    if(type == COUNT_IN_SERIES){
        setCountInSeries(var.value<int>());
    }
}

void CalculationModule::setCountInSeries(int count)
{
    m_countInSeries = count;
    m_spectrDataSaver->setCountInSeries(m_countInSeries);
}

int CalculationModule::countInSeries() const
{
    return m_countInSeries;
}

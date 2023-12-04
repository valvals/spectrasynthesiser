#include "BaseSpectroDevice.h"
#include "qdebug.h"
#include "qdatetime.h"

BaseSpectroDevice::BaseSpectroDevice():
    BaseSpectrometer()
{
    m_bandsCount = 0;
    m_isDarkSpectrumsNow = false;
    m_spectrDataSaver = nullptr;
    m_calculationModule = nullptr;
    m_isNeedToSaveMean = true;
}

void BaseSpectroDevice::deviceErrorrRecived(QString recivedError)
{
    emit showMessage(recivedError);
}

void BaseSpectroDevice::saveSpectrum()
{
//    qDebug()<<"save spectrum:"<<m_spectrMode<<m_calculationModule->countInSeries()<<m_spectrDataSaver->counter4Recording();
    QString baseFileName;
    baseFileName.append(QDateTime::currentDateTime().toString("yyyyMMdd_hh_mm_ss_z"));
    if(m_isDarkSpectrumsNow && m_spectrMode == COMBO_MODE){
        DarkSpectrum spectrum;
        spectrum.expositionIndex = m_expositionIndex;
        spectrum.dataSpectrum = m_spectrData;
        m_calculationModule->appendDataToDarkSpectrumsList(spectrum);
    }

    if(m_spectrMode == CONTINUOUS_MODE){
        emit sendSpectrumToSaver(baseFileName, m_specString);
    }else
        m_spectrDataSaver->saveStringToFile(baseFileName, &m_specString);

    if(m_spectrDataSaver->counter4Recording() == m_calculationModule->countInSeries() ||
            m_spectrMode == SINGLE_MODE || m_spectrMode == COMBO_MODE){
//        qDebug()<<"don't need to save";
        m_isNeedToSaveNow = false;
    }
    if(m_isNeedToSaveMean && m_spectrMode == SERIES_MODE){
        m_calculationModule->appendData4MeanCalculating(m_spectrData);
    }
    if(m_isNeedToSaveMean && m_spectrMode == COMBO_MODE && !m_isDarkSpectrumsNow){
        m_calculationModule->appendData4MeanCalculating(m_spectrData);
    }
}

void BaseSpectroDevice::addSpectrometerStatesToString()
{
    m_specString.append("Spectrometer:\tFlyS\n");
    m_specString.append("Bands values:\t");
    switch(m_bandsValues){
    case BAND_NUMBERS:
        m_specString.append("Numbers\n");
        break;
    case WAVELENGTH:
        m_specString.append("Wavelength, nm\n");
        break;
    }

    m_specString.append("Exposition:\t");
    m_specString.append(QString::number(m_expositionValue/1000, 'f', 3) + "\n");

    m_calculationModule->setCurrentMetaInfo(m_specString);
    m_calculationModule->setCurrentExposition(m_expositionValue);

    m_specString.append("\n");
}

void BaseSpectroDevice::setCalculationModule(CalculationModule *calculationModule)
{
    m_calculationModule = calculationModule;
    connect(m_calculationModule, SIGNAL(sendMessage(QString)), SIGNAL(showMessage(QString)));
}

void BaseSpectroDevice::setSpectrDataSaver(SpectrDataSaver *spectrDataSaver)
{
    m_spectrDataSaver = spectrDataSaver;
    connect(this, SIGNAL(sendSpectrumToSaver(QString, QString)), m_spectrDataSaver, SLOT(saveStringToFileLater(QString, QString)), Qt::QueuedConnection);
}

void BaseSpectroDevice::captureSpectr()
{

}

void BaseSpectroDevice::stopAll()
{
    if(m_spectrDataSaver != nullptr)
        delete m_spectrDataSaver;
    if(m_calculationModule != nullptr)
        delete m_calculationModule;
}

void BaseSpectroDevice::setBands(const QVector<double> &bands, BandsValues bandsValues)
{
    m_bands = bands;
    m_bandsValues = bandsValues;
    if(m_calculationModule != nullptr)
        m_calculationModule->setWavelenth(m_bands);
}

void BaseSpectroDevice::setIsNeedToSaveNow(bool isNeed2SaveNow)
{
    m_isNeedToSaveNow = isNeed2SaveNow;
}

void BaseSpectroDevice::setSpectrMode(const SpectrMode &spectrMode)
{
    m_spectrMode = spectrMode;
    m_spectrDataSaver->setSpectrMode(m_spectrMode);
    m_calculationModule->setSpectrMode(m_spectrMode);
}

void BaseSpectroDevice::setExpositionValue(double value)
{
    m_expositionValue = value;
    emit expositionWasChanged(value);
}

void BaseSpectroDevice::changeSettings(SpectrometerSettingsType settingsType, QVariant var)
{
    switch (settingsType){
    case EXPOSITION:
    {
        setExpositionValue(var.value<int>());
        break;
    }
    case IS_NEED2SAVE_MEAN:
    {
        setIsNeedToSaveMean(var.value<bool>());
        break;
    }
    default:
        break;
    }
}

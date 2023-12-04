#include "BaseSpectrometer.h"

BaseSpectrometer::BaseSpectrometer() : QObject()
{
    m_isScannerMode = false;
    m_isConfigurationMode = false;
    m_shutterState = true;
    m_filterState = false;
    m_spectrMode = SINGLE_MODE;
}

void BaseSpectrometer::changeSettings(SpectrometerSettingsType settingsType, QVariant var)
{
    Q_UNUSED(settingsType)
    Q_UNUSED(var)
}

void BaseSpectrometer::stopAll()
{

}

void BaseSpectrometer::captureSpectr()
{

}

void BaseSpectrometer::setSpectrometerElevationAngle(int elevationAngle)
{
    m_spectrometerElevationAngle = elevationAngle;
}

void BaseSpectrometer::setSpectrometerShootingSurface(bool shootingSurface)
{
    m_spectrometerShootingSurface = shootingSurface;
}

int BaseSpectrometer::getExpositionIndex() const
{
    return m_expositionIndex;
}

SpectrMode BaseSpectrometer::getSpectrMode() const
{
    return m_spectrMode;
}

bool BaseSpectrometer::getIsNeedToSaveMean() const
{
    return m_isNeedToSaveMean;
}

void BaseSpectrometer::setIsScannerMode(bool isScannerMode)
{
    m_isScannerMode = isScannerMode;
}

void BaseSpectrometer::setSpectrMode(const SpectrMode &spectrMode)
{
    m_spectrMode = spectrMode;
}

void BaseSpectrometer::setIsConfigurationMode(bool isConfigurationMode)
{
    m_isConfigurationMode = isConfigurationMode;
}

int BaseSpectrometer::getBandsCount() const
{
    return m_bandsCount;
}

double BaseSpectrometer::getExpositionValue() const
{
    return m_expositionValue;
}

void BaseSpectrometer::setExpositionValue(double expositionValue)
{
    m_expositionValue = expositionValue;
    emit expositionWasChanged(expositionValue);
}

void BaseSpectrometer::changeShutterState(bool isOpen)
{
    m_shutterState = isOpen;
}

void BaseSpectrometer::changeFilterState(bool doesExist)
{
    m_filterState = doesExist;
}

void BaseSpectrometer::setIsDarkSpectrumsNow(bool isDarkSpectrumsNow)
{
    m_isDarkSpectrumsNow = isDarkSpectrumsNow;
}

void BaseSpectrometer::setBands(const QVector<double> &bands,  BandsValues bandsValues)
{
    m_bands = bands;
    m_bandsValues = bandsValues;
}

QVector<double> BaseSpectrometer::getBands() const
{
    return m_bands;
}

BandsValues BaseSpectrometer::getBandsValues() const
{
    return m_bandsValues;
}

void BaseSpectrometer::setIsNeedToSaveNow(bool isNeed2SaveNow)
{
    m_isNeedToSaveNow = isNeed2SaveNow;
}

void BaseSpectrometer::setIsNeedToSaveMean(bool isNeed2SaveMean)
{
    m_isNeedToSaveMean = isNeed2SaveMean;
}

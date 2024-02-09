#include "HamamatsuApi.h"
#include <qdebug.h>

HamamatsuApi::HamamatsuApi()
  : m_isAttached(false)

{
  m_hCCDdll = nullptr;
  m_fCCD_Init = nullptr;
  m_fCCD_HitTest = nullptr;
  m_fCCD_CameraReset = nullptr;
  m_fCCD_SetExtendParameters = nullptr;
  m_fCCD_GetExtendParameters = nullptr;
  m_fCCD_SetParameter = nullptr;
  m_fCCD_GetParameter = nullptr;
  m_fCCD_InitMeasuring = nullptr;
  m_fCCD_InitMeasuringData = nullptr;
  m_fCCD_StartWaitMeasuring = nullptr;
  m_fCCD_StartMeasuring = nullptr;
  m_fCCD_GetMeasureStatus = nullptr;
  m_fCCD_GetData = nullptr;
  m_fCCD_GetSerialNum = nullptr;
  m_fCCD_GetID = nullptr;
  m_ID = 0;
  m_pData = nullptr;
  m_pixelsInColumn = 0;
  m_pixelsInRow = 0;
  m_pixelsInMatrix = 0;
  m_sensorMode = MATRIX;
  params = new TCCDUSBExtendParams;


}

HamamatsuApi::~HamamatsuApi() {
  if (m_pData != nullptr)
    delete[] m_pData;
  if (params != nullptr)
    delete params;
  FreeLibrary(m_hCCDdll);
}

void HamamatsuApi::prepareMemory() {
  m_pixelsInMatrix = m_pixelsInRow * m_pixelsInColumn;
  int bufferSize = m_pixelsInMatrix;
  m_pData = new DWORD[static_cast<uint>(bufferSize)];
  qDebug() << "Memory size is allocated:" << bufferSize;
}

void HamamatsuApi::setSensorMode(sensorMode sensorMode) {
  m_sensorMode = sensorMode;
  m_fCCD_SetParameter(m_ID, PRM_DEVICEMODE, m_sensorMode);
  qDebug() << "Sensor mode:" << m_sensorMode;
}

BOOL HamamatsuApi::InitInstance() {

  m_hCCDdll =::LoadLibrary(L"CCDUSBDCOM01.dll");
  if (m_hCCDdll == nullptr)
  {
    DWORD errorMessage = GetLastError();
    qDebug()<<"Load library error: " << errorMessage;
    return FALSE;
  } else {

    m_fCCD_Init = (PCCD_Init)::GetProcAddress(m_hCCDdll, "CCD_Init");
    m_fCCD_HitTest = (PCCD_HitTest)::GetProcAddress(m_hCCDdll, "CCD_HitTest");
    m_fCCD_CameraReset = (PCCD_CameraReset)::GetProcAddress(m_hCCDdll, "CCD_CameraReset");
    m_fCCD_SetExtendParameters = (PCCD_SetExtendParameters)::GetProcAddress(m_hCCDdll, "CCD_SetExtendParameters");
    m_fCCD_GetExtendParameters = (PCCD_GetExtendParameters)::GetProcAddress(m_hCCDdll, "CCD_GetExtendParameters");
    m_fCCD_SetParameter = (PCCD_SetParameter)::GetProcAddress(m_hCCDdll, "CCD_SetParameter");
    m_fCCD_GetParameter = (PCCD_GetParameter)::GetProcAddress(m_hCCDdll, "CCD_GetParameter");
    m_fCCD_InitMeasuring = (PCCD_InitMeasuring)::GetProcAddress(m_hCCDdll, "CCD_InitMeasuring");
    m_fCCD_InitMeasuringData = (PCCD_InitMeasuringData)::GetProcAddress(m_hCCDdll, "CCD_InitMeasuringData");
    m_fCCD_StartWaitMeasuring = (PCCD_StartWaitMeasuring)::GetProcAddress(m_hCCDdll, "CCD_StartWaitMeasuring");
    m_fCCD_StartMeasuring = (PCCD_StartMeasuring)::GetProcAddress(m_hCCDdll, "CCD_StartMeasuring");
    m_fCCD_GetMeasureStatus = (PCCD_GetMeasureStatus)::GetProcAddress(m_hCCDdll, "CCD_GetMeasureStatus");
    m_fCCD_GetData = (PCCD_GetData)::GetProcAddress(m_hCCDdll, "CCD_GetData");
    m_fCCD_GetSerialNum = (PCCD_GetSerialNum)::GetProcAddress(m_hCCDdll, "CCD_GetSerialNum");
    m_fCCD_GetID = (PCCD_GetID)::GetProcAddress(m_hCCDdll, "CCD_GetID");
    m_fCCD_AddStrip = (PCCD_AddStrip)::GetProcAddress(m_hCCDdll, "CCD_AddStrip");

    integer ID = 0;
    return m_fCCD_Init(nullptr, nullptr, &ID);
  }
}

bool HamamatsuApi::attach() {
  m_isAttached = m_fCCD_HitTest(0);
  if (m_isAttached)
    m_ID = 0;
  return m_isAttached;
}

bool HamamatsuApi::attach(int id) {
  m_isAttached = m_fCCD_HitTest(id);
  if (m_isAttached)
    m_ID = id;
  return m_isAttached;
}

int HamamatsuApi::getPixelsInMatrix() const {
  return m_pixelsInMatrix;
}

sensorMode HamamatsuApi::getSensorMode() const {
  return m_sensorMode;
}

DWORD* HamamatsuApi::getSpectralData(bool& result) {

  if (!m_isAttached)
    result = false;

  if (m_fCCD_InitMeasuringData(m_ID, m_pData)) {
    if (m_fCCD_StartWaitMeasuring(m_ID)) {
      result = true;
      //Sleep(50);
      return m_pData;
    }
  }
  result = false;
  return m_pData;
}

bool HamamatsuApi::resetDevice() {
  if (!m_isAttached)
    return false;
  return m_fCCD_CameraReset(m_ID);
}

bool HamamatsuApi::getSerialNumber(QString& serialNumber, const int& id) {
  PCHAR pstr;
  bool result =  m_fCCD_GetSerialNum(id, &pstr);
  serialNumber = QString::fromLocal8Bit(pstr);
  return result;
}

bool HamamatsuApi::getID(const QString& serialNumber, int& id) {
  QByteArray ba = serialNumber.toLocal8Bit();
  char* c_str = ba.data();
  return m_fCCD_GetID(c_str, &id);
}

bool HamamatsuApi::getParameters() {
  if (!m_isAttached)
    return false;

  if (m_fCCD_GetExtendParameters(m_ID, params)) {
    m_dwDigitCapacity = params->dwDigitCapacity;
    m_pixelRate =       params->nPixelRate;
    m_pixelsInRow =     params->nNumPixelsH;
    m_pixelsInColumn =  params->nNumPixelsV;
    m_nNumReadOuts =    params->nNumReadOuts;
    m_sPreBurning =     params->sPreBurning;
    m_exposureTime =    params->sExposureTime;
    m_dwSynchr =        params->dwSynchr;
    m_bSummingMode =    params->bSummingMode;
    m_dwDeviceMode =    params->dwDeviceMode;
    m_nStripCount =     params->nStripCount;
    m_dwSensitivity =   params->dwSensitivity;
    m_dwProperty =      params->dwProperty ;
    m_sShuterTime =     params->sShuterTime;
    return true;
  }
  return false;
}

bool HamamatsuApi::setParameters() {
  if (!m_isAttached)
    return false;

  params->dwDigitCapacity = m_dwDigitCapacity;
  params->nPixelRate =      m_pixelRate;
  params->nNumPixelsH =     m_pixelsInRow;
  params->nNumPixelsV =     m_pixelsInColumn;
  params->nNumReadOuts =    m_nNumReadOuts;
  params->sPreBurning =     m_sPreBurning;
  params->sExposureTime =   m_exposureTime;
  params->dwSynchr =        m_dwSynchr;
  params->bSummingMode =    m_bSummingMode;
  params->dwDeviceMode =    m_sensorMode;
  params->dwSensitivity =   m_dwSensitivity;
  params->dwProperty =      m_dwProperty ;
  params->sShuterTime =     m_sShuterTime;

  return m_fCCD_SetExtendParameters(m_ID, params);
}

bool HamamatsuApi::setReadOuts(int nNumReadOuts) {
  if (!m_isAttached)
    return FALSE;
  if (m_fCCD_SetParameter(m_ID, PRM_READOUTS, nNumReadOuts)) {
    if (m_fCCD_GetParameter(m_ID, PRM_READOUTS, (float*)&nNumReadOuts)) {
      m_nNumReadOuts = nNumReadOuts;
      return TRUE;
    }
  }
  return FALSE;
}

bool HamamatsuApi::setExposition(float exposureTime) {
  if (!m_isAttached)
    return FALSE;

  if (m_fCCD_SetParameter(m_ID, PRM_EXPTIME, exposureTime)) {
    if (m_fCCD_GetParameter(m_ID, PRM_EXPTIME, &exposureTime)) {
      m_exposureTime = exposureTime;
      return TRUE;
    }
  }

  return FALSE;
}

bool HamamatsuApi::setSynchro(DWORD dwSynchr) {
  if (!m_isAttached)
    return FALSE;
  if (m_fCCD_SetParameter(m_ID, PRM_SYNCHR, dwSynchr)) {
    if (m_fCCD_GetParameter(m_ID, PRM_SYNCHR, (float*)&dwSynchr)) {
      m_dwSynchr = dwSynchr;
      return TRUE;
    }
  }
  return FALSE;
}

bool HamamatsuApi::setSensivity(sensivityMode mode) {
  if (!m_isAttached)
    return FALSE;
  DWORD dwSens = mode;
  if (m_fCCD_SetParameter(m_ID, PRM_SENSIT, dwSens)) {
    if (m_fCCD_GetParameter(m_ID, PRM_SENSIT, (float*)&dwSens)) {
      return TRUE;
    }
  }
  return FALSE;
}

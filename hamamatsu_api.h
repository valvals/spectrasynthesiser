#ifndef HAMAMATSU_API_H
#define HAMAMATSU_API_H

#include <Windows.h>

#pragma once

#define BYTEx8_ALIGN __declspec(align(8))

#define  PRM_DIGIT           1
#define  PRM_PIXELRATE       2
#define  PRM_NUMPIXELS       3
#define PRM_READOUTS         4
#define PRM_EXPTIME          5
#define PRM_SYNCHR           6
#define PRM_NUMPIXELSH       7
#define PRM_NUMPIXELSV       8
#define PRM_SUMMING          9
#define PRM_DEVICEMODE      10
#define PRM_STRIPCOUNT      11
#define PRM_SENSIT          14
#define PRM_DEVICEPROPERTY  15
#define PRM_PREBURNING      16
#define PRM_SHUTERTIME      17;

#define SYNCHR_NONE      0x01
#define SYNCHR_CONTR     0x20
#define SYNCHR_CONTR_FRS 0x04
#define SYNCHR_CONTR_NEG 0x08
#define SYNCHR_EXT       0x10
#define SYNCHR_EXT_FRS   0x02

#define STATUS_WAIT_DATA   1
#define STATUS_WAIT_TRIG   2
#define STATUS_DATA_READY  3
#define MAXSTRIPS          8

#define DEVICEMODEA1  0x0002
#define DEVICEMODEA2  0x0000
#define DEVICEMODES   0x0003

#define DP_SYNCHR_CONTR         0x00000001
#define DP_SYNCHR_CONTR_FRS     0x00000002
#define DP_SYNCHR_CONTR_NEG     0x00000004
#define DP_SYNCHR_EXT           0x00000008
#define DP_SYNCHR_EXT_FRS       0x00000010
#define DP_SENSIT               0x00000020
#define DP_MODEA2               0x00000040
#define DP_MODES1               0x00000080
#define DP_MODES2               0x00000100
#define DP_PREBURNING           0x00000200
#define DP_SHUTER               0x00000400
#define DP_CLOCKCONTROL         0x00000800

#define NCAMMAX  3


typedef DWORD*  PDWORDArr;
typedef WORD*   PWORDArr;
typedef RECT TRect;

#define integer int

typedef struct {
  DWORD   dwDigitCapacity; //The digit capacity of CCD-camera
  integer nPixelRate;      //The pixel rate kHz
  integer nNumPixels;      //The number of pixels
  integer nNumReadOuts;    //The number of readouts
  integer nExposureTime;   //The exposure time
  DWORD dwSynchr;          //The synchronization mode

} TCCDUSBParams;


#define boolean bool

typedef struct BYTEx8_ALIGN {
  DWORD dwDigitCapacity;    //The digit capacity of CCD-camera
  integer nPixelRate;       //The pixel rate kHz
  integer nNumPixelsH;      // The number of pixels on a horizontal (columns number of CCD-array)
  integer nNumPixelsV;      // The number of pixels on a vertical (rows number of CCD-array)
  DWORD Reserve1;           // not used
  DWORD Reserve2;           // not used

  integer nNumReadOuts;     // The number of readouts
  float sPreBurning;        // The Time preliminary burning in seconds.
  float sExposureTime;      // The exposure time
  float sTime2;             // not used
  DWORD dwSynchr;           // The synchronization mode.
  boolean bSummingMode;     // Turn on(off) summing mode. Not used.

  DWORD dwDeviceMode;       // Turn on(off) spectral mode of CCD-array.
  integer nStripCount;      // The number of strips for a spectral mode
  RECT rcStrips[MAXSTRIPS]; // The strips for a spectral mode.
  integer Reserve11;

  DWORD dwSensitivity;      // Turn on (off) a mode of the raised sensitivity of a CCD-sensor control. Actually if dwProperty & DP_SENSIT <> 0.
  DWORD dwProperty ;        // The device property.
  float sShuterTime;        // Shuter time (ms). Active in minimal exposure time.

  DWORD Reserve6;           // not used
  DWORD Reserve7;           // not used
  DWORD Reserve8;           // not used
  DWORD Reserve9;           // not used
  DWORD Reserve10;          // not used

} TCCDUSBExtendParams;

#define PChar PCHAR

typedef boolean(__stdcall* PCCD_Init)(HWND ahAppWnd, PChar Prm, integer* ID);
typedef boolean(__stdcall* PCCD_HitTest)(integer ID);
typedef boolean(__stdcall* PCCD_CameraReset)(integer ID);
typedef boolean(__stdcall* PCCD_SetParameters)(integer ID, TCCDUSBParams* Prms);
typedef boolean(__stdcall* PCCD_SetExtendParameters)(integer ID, TCCDUSBExtendParams* Prms);
typedef boolean(__stdcall* PCCD_GetParameters)(integer ID, TCCDUSBExtendParams* Prms);
typedef boolean(__stdcall* PCCD_GetExtendParameters)(integer ID, TCCDUSBExtendParams* Prms);
typedef boolean(__stdcall* PCCD_SetParameter)(integer ID, DWORD dwPrmID, float Prm);
typedef boolean(__stdcall* PCCD_GetParameter)(integer ID, DWORD dwPrmID, float* Prm);
typedef boolean(__stdcall* PCCD_InitMeasuring)(integer ID);
typedef boolean(__stdcall* PCCD_InitMeasuringData)(integer ID, PDWORDArr apData);
typedef boolean(__stdcall* PCCD_StartWaitMeasuring)(integer ID);
typedef boolean(__stdcall* PCCD_StartMeasuring)(integer aID);
typedef boolean(__stdcall* PCCD_GetMeasureStatus)(integer ID, DWORD* adwStatus);
typedef boolean(__stdcall* PCCD_GetData)(integer ID, PDWORDArr pData);
typedef boolean(__stdcall* PCCD_GetSerialNum)(integer ID, PCHAR* sernum);
typedef boolean(__stdcall* PCCD_GetID)(PCHAR sernum, integer* ID);
typedef boolean(__stdcall* PCCD_ClearStrips)(integer ID);
typedef boolean(__stdcall* PCCD_AddStrip)(integer ID, TRect arcStrip);
typedef boolean(__stdcall* PCCD_DeleteStrip)(integer ID, integer Index);
typedef boolean(__stdcall* PCCDDCOM_DisconectDCOM)(integer ID);
typedef boolean(__stdcall* PCCDDCOM_SetDCOMRemoteName)(integer ID, PCHAR RemoteName);

enum sensivityMode {
  LOW_SENSIVITY, HIGH_SENSIVITY
};
enum sensorMode {
  MATRIX = 0, SPECTROSCOPE = 3
};
class QString;
class HamamatsuApi {
 public:
  HamamatsuApi();

  virtual BOOL InitInstance();
  ~HamamatsuApi();
  TCCDUSBExtendParams* params;

 private:
  PCCD_Init m_fCCD_Init;
  PCCD_HitTest m_fCCD_HitTest;
  PCCD_CameraReset m_fCCD_CameraReset;
  PCCD_SetExtendParameters m_fCCD_SetExtendParameters;
  PCCD_GetExtendParameters m_fCCD_GetExtendParameters;
  PCCD_SetParameter m_fCCD_SetParameter;
  PCCD_GetParameter m_fCCD_GetParameter;
  PCCD_InitMeasuring m_fCCD_InitMeasuring;
  PCCD_InitMeasuringData m_fCCD_InitMeasuringData;
  PCCD_StartWaitMeasuring m_fCCD_StartWaitMeasuring;
  PCCD_StartMeasuring m_fCCD_StartMeasuring;
  PCCD_GetMeasureStatus m_fCCD_GetMeasureStatus;
  PCCD_GetData m_fCCD_GetData;
  PCCD_GetSerialNum m_fCCD_GetSerialNum;
  PCCD_GetID m_fCCD_GetID;
  PCCD_AddStrip m_fCCD_AddStrip;
  HMODULE m_hCCDdll;
  PDWORDArr m_pData;
  RECT m_rcStrip;
  bool m_isAttached;
  int m_ID;
  int m_pixelsInMatrix;
  sensorMode m_sensorMode;


 public:
  DWORD m_dwDigitCapacity;
  DWORD m_dwSynchr;
  DWORD m_dwDeviceMode;
  DWORD m_dwSensitivity;
  DWORD m_dwProperty ;
  int m_pixelRate;
  int m_pixelsInRow;
  int m_pixelsInColumn;
  int m_nNumReadOuts;
  int m_nStripCount;
  float m_sPreBurning;
  float m_exposureTime;
  float m_sShuterTime;
  boolean m_bSummingMode;

  DWORD* getSpectralData(bool& result);
  bool attach(int id);
  bool attach();
  bool resetDevice();
  bool getSerialNumber(QString& serialNumber, const int& id);
  bool getID(const QString& serialNumber, int& id);
  bool getParameters();
  bool setParameters();
  bool setReadOuts(int nNumReadOuts);
  bool setExposition(float exposureTime);
  bool setSynchro(DWORD dwSynchr);
  bool setSensivity(sensivityMode mode);
  int  getPixelsInMatrix() const;
  sensorMode getSensorMode() const;
  void prepareMemory();
  void setSensorMode(sensorMode sensorMode);
};

#endif // HAMAMATSU_API_H

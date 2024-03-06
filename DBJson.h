#ifndef DBJSON_H
#define DBJSON_H
#include "QString"
#include "QVector"
#include "QJsonDocument"

class QJsonArray;
class QJsonObject;


namespace jsn {

constexpr int UNDEFINED = -999;

struct CLASSIFICATION {
  QString general_type;
  QString class_name;
  QString object_name;
};

struct LOCATION {
  double latitude = UNDEFINED;
  double longitude = UNDEFINED;
  double altitude = UNDEFINED;
  QString local_name;
  QString place_name;
  QString place_type;
  QString region_name;
};

struct IMAGE_OBJECT {
  QString name;
  QString type;
  QString description;
};

struct FRACTION {
  QString name;
  double from = UNDEFINED;
  double to = UNDEFINED;
  QString unit;
};

struct AIR_CONDITIONS {
  double temperature;
  double humidity;
};

struct WHEATHER_CONDITIONS {
  int clouds_level = UNDEFINED;
  int wind = UNDEFINED;
  QString direction;
};

struct META_DATA {
  QString date_time;
  QString owner;
  double sun_elevation_angle = UNDEFINED;
  double capture_angle = UNDEFINED;
  QString experiment_name;
  QString capture_type;
  CLASSIFICATION classification;
  LOCATION location;
  QVector<IMAGE_OBJECT> images;
  QString relief_type;
  FRACTION fraction;
  AIR_CONDITIONS air_conditions;
  WHEATHER_CONDITIONS wheather_conditions;
};

struct SPECTRAL_ATRIBUTES {
  QString instrument;
  QString type;
  QString description;
};

struct SPECTRAL_DATA {
  QVector<SPECTRAL_ATRIBUTES> atributes;
  QVector<QVector<double>> waves;
  QVector<QVector<double>> values;
};

struct SPECTRAL_STRUCT {
  META_DATA md;
  SPECTRAL_DATA sd;
};

bool getJsonObjectFromFile(const QString& path,
                           QJsonObject& object);

bool getJsonArrayFromFile(const QString& path,
                          QJsonArray& object);

void getStructFromJsonObject(SPECTRAL_STRUCT& spectral_struct,
                             const QJsonObject& json_object);

bool saveJsonObjectToFile(const QString& path,
                          const QJsonObject& json_object,
                          QJsonDocument::JsonFormat format);
bool saveJsonArrayToFile(const QString& path,
                         const QJsonArray& json_object,
                         QJsonDocument::JsonFormat format);

bool saveStructToJsonFile(const QString& path,
                          const SPECTRAL_STRUCT& spectral_struct,
                          QJsonDocument::JsonFormat format);

void getJsonObjectFromStruct(const SPECTRAL_STRUCT& spectral_struct,
                             QJsonObject& json_object);

bool copyStructLikeJsonToClipboard(const SPECTRAL_STRUCT& spectral_struct);

bool isAllNodesExist(const QJsonObject& object);

void makeStructCleared(SPECTRAL_STRUCT& spectral_struct);

} // end namespace db_json

#endif // DBJSON_H

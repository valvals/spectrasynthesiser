#include "DBJson.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QFile>
#include <QDebug>
#include <QClipboard>
#include <QGuiApplication>
#include <QDir>


namespace {

bool checkNodes(const QJsonObject& obj,
                const QStringList& nodes) {
  for (int i = 0; i < nodes.size(); ++i) {
    if (obj.find(nodes[i]) == obj.end())
      return false;
  }
  return true;
};

} // end namespace

namespace db_json {

bool getJsonObjectFromFile(const QString& path,
                           QJsonObject& object) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << "File can't be opened!" << path;
    return false;
  };
  QByteArray data = file.readAll();
  QJsonParseError errorPtr;
  object = QJsonDocument::fromJson(data, &errorPtr).object();
  if (object.isEmpty()) {
    qDebug() << "JSON IS EMPTY: " << errorPtr.errorString();
    return false;
  }
  file.close();

  // ADD BRIGHT DEPS from file (I will use this code like a draft for creating update module)
  /*file.setFileName("bright_deps.txt");
  file.open(QIODevice::ReadOnly|QIODevice::Text);
  QTextStream qts(&file);
  QString line;
  QStringList list;
  while(qts.readLineInto(&line)){
     list.append(line);
  }
  qDebug()<<"List size: "<<list.size();
  QStringList waves = list[0].split('\t');
  QStringList c_coeff = list[1].split('\t');
  QStringList b_coeff = list[2].split('\t');
  QStringList a_coeff = list[3].split('\t');
  auto obj = object["pins_array"].toArray();
  for(int i=0;i<obj.size();++i){
      auto element = obj[i].toObject();
      auto coeffs = obj[i].toObject()["bright_deps"].toObject();
      element["wave"] = waves[i].toDouble();
      coeffs["c"] = c_coeff[i].toDouble();
      coeffs["b"] = b_coeff[i].toDouble();
      coeffs["a"] = a_coeff[i].toDouble();
      element["bright_deps"] = coeffs;
      obj[i] = element;
  }
  object["pins_array"] = obj;
  db_json::saveJsonObjectToFile("test.json",object,QJsonDocument::Indented);*/

  // ADD DIOD MODELS from files (I will use this code like a draft for creating update module)
  /*QDir dir("diod_models");
  QStringList models;
  models = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
  qDebug()<<"models_size: "<<models.size();
  auto obj = object["pins_array"].toArray();
  for(int i=0;i<obj.size();++i){
      auto element = obj[i].toObject();
      QFile file("diod_models/"+models[i]);
      file.open(QIODevice::ReadOnly|QIODevice::Text);
      QTextStream qts(&file);
      QString line;
      QStringList values;
      QJsonObject model;
      QJsonArray bright_values;
      QJsonArray model_waves;
      while(qts.readLineInto(&line)){
         values = line.split('\t');
         model_waves.push_back(values[0].toDouble());
         bright_values.push_back(values[1].toDouble());
      }
      model["waves"] = model_waves;
      model["values"]= bright_values;
      element["model"] = model;
      obj[i] = element;
  }
  object["pins_array"] = obj;
  db_json::saveJsonObjectToFile("test.json",object,QJsonDocument::Indented);*/


  return true;
}

bool getJsonArrayFromFile(const QString& path,
                          QJsonArray& object) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << "File can't be opened!" << path;
    return false;
  };
  QByteArray data = file.readAll();
  QJsonParseError errorPtr;
  object = QJsonDocument::fromJson(data, &errorPtr).array();
  if (object.isEmpty()) {
    qDebug() << "JSON IS EMPTY: " << errorPtr.errorString();
    return false;
  }
  file.close();
  return true;
}

void getStructFromJsonObject(SPECTRAL_STRUCT& spectral_struct,
                             const QJsonObject& json_object) {

  auto md = json_object.find("meta_data").value().toObject();
  spectral_struct.md.date_time = md.find("date_time").value().toString();
  spectral_struct.md.owner = md.find("owner").value().toString();
  spectral_struct.md.sun_elevation_angle = md.find("sun_elevation_angle").value().toDouble();
  spectral_struct.md.capture_angle = md.find("capture_angle").value().toDouble();
  spectral_struct.md.experiment_name = md.find("experiment_name").value().toString();
  spectral_struct.md.capture_type = md.find("capture_type").value().toString();
  auto classification = md.find("classification")->toObject();
  spectral_struct.md.classification.general_type = classification.find("general_type")->toString();
  spectral_struct.md.classification.class_name = classification.find("class_name")->toString();
  spectral_struct.md.classification.object_name = classification.find("object_name")->toString();
  auto location = md.find("location")->toObject();
  spectral_struct.md.location.latitude = location.find("latitude").value().toDouble();
  spectral_struct.md.location.longitude = location.find("longitude").value().toDouble();
  spectral_struct.md.location.altitude = location.find("altitude").value().toDouble();
  spectral_struct.md.location.local_name = location.find("local_name").value().toString();
  spectral_struct.md.location.place_name = location.find("place_name").value().toString();
  spectral_struct.md.location.place_type = location.find("place_type").value().toString();
  spectral_struct.md.location.region_name = location.find("region_name").value().toString();
  auto images = md.find("images")->toArray();
  for (auto &&it : images) {
    auto image_object = it.toObject();
    IMAGE_OBJECT img_obj;
    img_obj.name = image_object.find("name")->toString();
    img_obj.type = image_object.find("type")->toString();
    img_obj.description = image_object.find("description")->toString();
    spectral_struct.md.images.append(img_obj);
  };
  spectral_struct.md.relief_type = md.find("relief_type").value().toString();
  auto fraction = md.find("fraction").value().toObject();
  spectral_struct.md.fraction.name = fraction.find("name")->toString();
  spectral_struct.md.fraction.from = fraction.find("from")->toDouble();
  spectral_struct.md.fraction.to = fraction.find("to")->toDouble();
  spectral_struct.md.fraction.unit = fraction.find("unit")->toString();
  auto air_conditions = md.find("air_conditions")->toObject();
  spectral_struct.md.air_conditions.temperature = air_conditions.find("temperature").value().toDouble();
  spectral_struct.md.air_conditions.humidity = air_conditions.find("humidity").value().toDouble();
  auto wheather_conditions = md.find("wheather_conditions").value().toObject();
  spectral_struct.md.wheather_conditions.clouds_level = wheather_conditions.find("clouds_level")->toDouble();
  spectral_struct.md.wheather_conditions.wind = wheather_conditions.find("wind")->toDouble();
  spectral_struct.md.wheather_conditions.direction = wheather_conditions.find("direction")->toString();
  auto sd = json_object.find("spectral_data").value().toObject();
  auto atributes = sd.find("atributes")->toArray();
  for (int i = 0; i < atributes.size(); ++i) {
    db_json::SPECTRAL_ATRIBUTES sa;
    auto atr = atributes[i].toObject();
    sa.instrument = atr.find("instrument")->toString();
    sa.type = atr.find("type")->toString();
    sa.description = atr.find("description")->toString();
    spectral_struct.sd.atributes.append(sa);
  }
  auto waves = sd.find("waves").value().toArray();
  for (auto &&it : waves) {
    auto wave_line = it.toArray();
    QVector<double> wave;
    for (auto &&it : wave_line) {
      wave.push_back(it.toDouble());
    }
    spectral_struct.sd.waves.append(wave);
  };
  auto values = sd.find("values").value().toArray();
  for (auto &&it : values) {
    auto value_line = it.toArray();
    QVector<double> value;
    for (auto &&it : value_line) {
      value.push_back(it.toDouble());
    }
    spectral_struct.sd.values.append(value);
  };
}

bool saveJsonObjectToFile(const QString& path,
                          const QJsonObject& json_object,
                          QJsonDocument::JsonFormat format) {
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly))
    return false;
  auto json_doc = QJsonDocument(json_object).toJson(format);
  auto result = file.write(json_doc);
  file.close();
  if (result == -1)
    return false;
  else
    return true;
}


bool saveStructToJsonFile(const QString& path,
                          const SPECTRAL_STRUCT& spectral_struct,
                          QJsonDocument::JsonFormat format) {
  QJsonObject root;
  getJsonObjectFromStruct(spectral_struct, root);
  return saveJsonObjectToFile(path, root, format);
}

bool isAllNodesExist(const QJsonObject& object) {
  QStringList nodes = {
    "meta_data",
    "spectral_data"
  };
  if (!checkNodes(object, nodes))
    return false;
  nodes.clear();
  nodes << "date_time" << "owner" << "sun_elevation_angle"
        << "capture_angle" << "experiment_name" << "capture_type"
        << "classification" << "location" << "images"
        << "relief_type" << "fraction" << "air_conditions"
        << "wheather_conditions";
  QJsonObject md = object.find("meta_data")->toObject();
  if (!checkNodes(md, nodes))
    return false;
  return true;
}

bool copyStructLikeJsonToClipboard(const SPECTRAL_STRUCT& spectral_struct) {
  QJsonObject jo;
  getJsonObjectFromStruct(spectral_struct, jo);
  QJsonDocument doc(jo);
  QString strJson(doc.toJson(QJsonDocument::Indented));
  QClipboard* clipboard = QGuiApplication::clipboard();
  clipboard->setText(strJson);
  return true;
}

void getJsonObjectFromStruct(const SPECTRAL_STRUCT& spectral_struct, QJsonObject& root) {
  QJsonObject meta_data_object;
  meta_data_object["date_time"] = spectral_struct.md.date_time;
  meta_data_object["owner"] = spectral_struct.md.owner;
  meta_data_object["sun_elevation_angle"] = spectral_struct.md.sun_elevation_angle;
  meta_data_object["capture_angle"] = spectral_struct.md.capture_angle;
  meta_data_object["experiment_name"] = spectral_struct.md.experiment_name;
  meta_data_object["capture_type"] = spectral_struct.md.capture_type;

  QJsonObject classification;
  classification["general_type"] = spectral_struct.md.classification.general_type;
  classification["class_name"] = spectral_struct.md.classification.class_name;
  classification["object_name"] = spectral_struct.md.classification.object_name;
  meta_data_object["classification"] = classification;

  QJsonObject location;
  location["latitude"] = spectral_struct.md.location.latitude;
  location["longitude"] = spectral_struct.md.location.longitude;
  location["altitude"] = spectral_struct.md.location.altitude;
  location["local_name"] = spectral_struct.md.location.local_name;
  location["place_name"] = spectral_struct.md.location.place_name;
  location["place_type"] = spectral_struct.md.location.place_type;
  location["region_name"] = spectral_struct.md.location.region_name;
  meta_data_object["location"] = location;

  QJsonArray images_array;
  for (int i = 0; i < spectral_struct.md.images.size(); ++i) {
    images_array.append(QJsonValue({
      {"name", spectral_struct.md.images[i].name},
      {"type", spectral_struct.md.images[i].type},
      {"description", spectral_struct.md.images[i].description}
    }));
  }
  meta_data_object["images"] = images_array;
  meta_data_object["relief_type"] = spectral_struct.md.relief_type;

  QJsonObject fraction;
  fraction["name"] = spectral_struct.md.fraction.name;
  fraction["from"] = spectral_struct.md.fraction.from;
  fraction["to"] = spectral_struct.md.fraction.to;
  fraction["unit"] = spectral_struct.md.fraction.unit;
  meta_data_object["fraction"] = fraction;

  QJsonObject air_conditions;
  air_conditions["temperature"] = spectral_struct.md.air_conditions.temperature;
  air_conditions["humidity"] = spectral_struct.md.air_conditions.humidity;
  meta_data_object["air_conditions"] = air_conditions;

  QJsonObject weather_conditions;
  weather_conditions["clouds_level"] = spectral_struct.md.wheather_conditions.clouds_level;
  weather_conditions["wind"] = spectral_struct.md.wheather_conditions.wind;
  weather_conditions["direction"] = spectral_struct.md.wheather_conditions.direction;
  meta_data_object["wheather_conditions"] = weather_conditions;

  QJsonObject spectral_data;

  QJsonArray atributes;
  for (int i = 0; i < spectral_struct.sd.atributes.size(); ++i) {
    atributes.append(QJsonValue({
      {"instrument", spectral_struct.sd.atributes[i].instrument},
      {"type", spectral_struct.sd.atributes[i].type},
      {"description", spectral_struct.sd.atributes[i].description}
    }));
  }
  spectral_data["atributes"] = atributes;
  QJsonArray waves;
  for (int i = 0; i < spectral_struct.sd.waves.size(); ++i) {
    QJsonArray waves_line;
    for (int j = 0; j < spectral_struct.sd.waves[i].size(); ++j) {
      waves_line.append(spectral_struct.sd.waves[i][j]);
    }
    waves.append(waves_line);
  }
  spectral_data["waves"] = waves;

  QJsonArray values;
  for (int i = 0; i < spectral_struct.sd.values.size(); ++i) {
    QJsonArray values_line;
    for (int j = 0; j < spectral_struct.sd.values[i].size(); ++j) {
      values_line.append(spectral_struct.sd.values[i][j]);
    }
    values.append(values_line);
  }
  spectral_data["values"] = values;

  root["meta_data"] = meta_data_object;
  root["spectral_data"] = spectral_data;

}

void makeStructCleared(SPECTRAL_STRUCT& spectral_struct) {
  spectral_struct.md.date_time = "";
  spectral_struct.md.owner = "";
  spectral_struct.md.experiment_name = "";
  spectral_struct.md.capture_type = "";
  spectral_struct.md.sun_elevation_angle = UNDEFINED;
  spectral_struct.md.capture_angle = UNDEFINED;

  spectral_struct.md.location.local_name = "";
  spectral_struct.md.location.place_name = "";
  spectral_struct.md.location.region_name = "";
  spectral_struct.md.location.place_type = "";
  spectral_struct.md.location.latitude = UNDEFINED;
  spectral_struct.md.location.longitude = UNDEFINED;
  spectral_struct.md.location.altitude = UNDEFINED;

  spectral_struct.md.classification.class_name = "";
  spectral_struct.md.classification.general_type = "";
  spectral_struct.md.classification.object_name = "";

  spectral_struct.md.air_conditions.humidity = UNDEFINED;
  spectral_struct.md.air_conditions.temperature = UNDEFINED;

  spectral_struct.md.wheather_conditions.clouds_level = UNDEFINED;
  spectral_struct.md.wheather_conditions.wind = UNDEFINED;
  spectral_struct.md.wheather_conditions.direction = "";

  spectral_struct.md.fraction.from = UNDEFINED;
  spectral_struct.md.fraction.to = UNDEFINED;
  spectral_struct.md.fraction.name = "";
  spectral_struct.md.fraction.unit = "";

  spectral_struct.md.images.clear();
  spectral_struct.sd.atributes.clear();
  spectral_struct.sd.waves.clear();
  spectral_struct.sd.values.clear();

}

bool saveJsonArrayToFile(const QString& path,
                         const QJsonArray& json_object,
                         QJsonDocument::JsonFormat format) {
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly))
    return false;
  auto json_doc = QJsonDocument(json_object).toJson(format);
  auto result = file.write(json_doc);
  file.close();
  if (result == -1)
    return false;
  else
    return true;
}

} // end db_json namespace

#include "json_utils.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QFile>
#include <QDebug>
#include <QGuiApplication>
#include <QDir>


namespace jsn {

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

} // end jsn namespace

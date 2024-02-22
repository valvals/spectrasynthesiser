#ifndef VOICE_INFORMATOR_H
#define VOICE_INFORMATOR_H

#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <memory>
#include <QObject>


class VoiceInformator: public QObject {
  Q_OBJECT
 public:
  VoiceInformator(const QString& pathToSounds);
  void playSound(const QString& sampleName);
  void muteSoundNotifications(bool isOn);
  void getSoundsList(QStringList& list);
  bool isNotificationsMuted() const;

 private:
  bool isNotificationsMuted_;
  const QString m_pathToSounds_;
  void createPlayer();
  int lastSampleIndex_;
  std::unique_ptr <QMediaPlayer>   m_player_;
  std::unique_ptr <QMediaPlaylist> m_playlist_;
  QStringList mySounds_;

 private slots:
  void playLastSound();
};

#endif // VOICE_INFORMATOR_H

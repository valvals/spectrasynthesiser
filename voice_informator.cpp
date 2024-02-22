#include "voice_informator.h"
#include "QDir"
#include "QDebug"

VoiceInformator::VoiceInformator(const QString& pathToSounds)
    : isNotificationsMuted_(false),
  m_pathToSounds_(pathToSounds){
  createPlayer();
}

void VoiceInformator::playSound(const QString& sampleName) {
  if (isNotificationsMuted_)
    return;
  uint16_t index = 0;
  auto it = std::find(mySounds_.begin(), mySounds_.end(), sampleName);
  if (it== std::end(mySounds_)){
      return;
  }
  index = std::distance(mySounds_.begin(),it);
  m_playlist_->setCurrentIndex(index);
  m_player_->play();
  lastSampleIndex_ = index;
}

void VoiceInformator::playLastSound() {
  if (isNotificationsMuted_)
    return;
  m_playlist_->setCurrentIndex(lastSampleIndex_);
  m_player_->play();
}

void VoiceInformator::muteSoundNotifications(bool isOn) {
  isNotificationsMuted_ = isOn;
}

void VoiceInformator::getSoundsList(QStringList& list) {
  list = mySounds_;
}

bool VoiceInformator::isNotificationsMuted() const {
  return isNotificationsMuted_;
}

void VoiceInformator::createPlayer() {
  isNotificationsMuted_  = false;
  m_player_ = std::make_unique<QMediaPlayer>();
  m_playlist_ = std::make_unique<QMediaPlaylist>(m_player_.get());
  m_player_->setPlaylist(m_playlist_.get());
  m_player_->setVolume(70);
  m_playlist_->setPlaybackMode(QMediaPlaylist::CurrentItemOnce);

  QDir dir(m_pathToSounds_);
  QString urlPrefix = QString(m_pathToSounds_);
  urlPrefix.prepend("qrc");
  urlPrefix.append("/");
  qDebug() << urlPrefix;

  mySounds_ = dir.entryList();
  for (int i = 0; i < mySounds_.count(); ++i) {
    QString url = urlPrefix + mySounds_.at(i);
    qDebug() << url;
    m_playlist_->addMedia(QUrl(url));
  }
}

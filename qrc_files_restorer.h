#ifndef QRC_FILES_RESTORER_H
#define QRC_FILES_RESTORER_H


class QString;

/*!
    \brief РљР»Р°СЃСЃ СЏРІР»СЏРµС‚СЃСЏ РѕР±СЉРµРєС‚РѕРј, РЅРµРѕР±С…РѕРґРёРјС‹Рј РґР»СЏ РІРѕСЃСЃС‚Р°РЅРѕРІР»РµРЅРёСЏ С„Р°Р№Р»РѕРІ РІ РґРёСЂРµРєС‚РѕСЂРёРё СЃ РЎРџРћ РёР· СЂРµСЃСѓСЂСЃРѕРІ РІ СЃР»СѓС‡Р°Рµ РёС… РѕС‚СЃСѓС‚СЃС‚РІРёСЏ.

    Р”Р°РЅРЅС‹Р№ РєР»Р°СЃСЃ РїСЂРµРґРЅР°Р·РЅР°С‡РµРЅ РґР»СЏ РІРѕСЃСЃС‚Р°РЅРѕРІР»РµРЅРёСЏ С„Р°Р№Р»РѕРІ РІ РґРёСЂРµРєС‚РѕСЂРёРё СЃ РЎРџРћ РёР· СЂРµСЃСѓСЂСЃРѕРІ РІ СЃР»СѓС‡Р°Рµ РёС… РѕС‚СЃСѓС‚СЃС‚РІРёСЏ.
*/
class QrcFilesRestorer {
 public:
  QrcFilesRestorer(const QString& path2Qrc);
  static void restoreFilesFromQrc(const QString& path2Qrc);

};

#endif // QRC_FILES_RESTORER_H

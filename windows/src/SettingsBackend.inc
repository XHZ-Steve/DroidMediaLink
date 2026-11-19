// Included once by Bridge.cpp; keeps the validated preference surface separate
// from media transport and the asynchronous lyric engine.
QVariantList Bridge::settingsCatalog()const{return SettingsSchema::descriptors();}
QVariantList Bridge::appearanceDescriptors()const{return AppearanceConfig::descriptors();}
bool Bridge::appearanceInherited(QString scope)const{return scope!="global"&&m_settings.value("appearance/"+scope+"/inherit",true).toBool();}
QVariantMap Bridge::appearanceConfig(QString scope)const{
 auto config=AppearanceConfig::defaults();
 const auto preset=m_settings.value("appearance/preset","balanced").toString();
 if(preset=="clear"){config["glassOpacity"]=.06;config["glassBlur"]=.7;config["glassRefraction"]=.7;config["backgroundSaturation"]=1.;}
 if(preset=="vivid"){config["glassOpacity"]=.09;config["glassBlur"]=1.5;config["backgroundVibrance"]=.45;config["backgroundSaturation"]=1.25;}
 if(preset=="optical"){
  config["glassOpacity"]=.045;config["glassTintStrength"]=.12;config["glassBlur"]=.6;
  config["glassRefraction"]=1.45;config["glassDistortion"]=1.25;config["glassDispersion"]=.23;
  config["glassEdgeGlow"]=.85;config["glassEdgeWidth"]=2.2;config["glassShadow"]=.08;
  config["backgroundSoftness"]=.65;config["backgroundFill"]=true;config["backgroundFillStrength"]=.45;
  config["backgroundSaturation"]=1.15;config["backgroundVibrance"]=.3;
 }
 for(auto it=config.begin();it!=config.end();++it){auto stored=m_settings.value("appearance/global/"+it.key());QVariant normalized;if(stored.isValid()&&AppearanceConfig::validate(it.key(),stored,&normalized))it.value()=normalized;}
 if(scope!="global"&&!appearanceInherited(scope))for(auto it=config.begin();it!=config.end();++it){auto stored=m_settings.value("appearance/"+scope+"/"+it.key());QVariant normalized;if(stored.isValid()&&AppearanceConfig::validate(it.key(),stored,&normalized))it.value()=normalized;}
 return config;
}
void Bridge::setAppearance(QString scope,QString key,QVariant value){
 if(!QStringList{"global","background","dock","popup","floating","desktop"}.contains(scope))return;
 if(value.metaType()==QMetaType::fromType<QJSValue>())value=value.value<QJSValue>().toVariant();QVariant normalized;
 if(!AppearanceConfig::validate(key,value,&normalized))return;
 m_settings.setValue("appearance/"+scope+"/"+key,normalized);emit settingsChanged();
}
void Bridge::setAppearanceInherited(QString scope,bool inherited){if(!QStringList{"background","dock","popup","floating","desktop"}.contains(scope))return;m_settings.setValue("appearance/"+scope+"/inherit",inherited);emit settingsChanged();}
void Bridge::resetAppearance(QString scope){if(!QStringList{"global","background","dock","popup","floating","desktop"}.contains(scope))return;m_settings.remove("appearance/"+scope);if(scope=="global")m_settings.setValue("appearance/preset","balanced");emit settingsChanged();}
void Bridge::resetSettingsGroup(int group,int section){for(const auto &item:SettingsSchema::descriptors()){auto row=item.toMap();if(row["group"].toInt()!=group||row["section"].toInt()!=section)continue;const auto key=row["key"].toString();if(row["type"]=="action"||row["type"]=="readonly")continue;setSetting(key,row["defaultValue"]);}emit settingsChanged();}
void Bridge::exportDiagnostics(QUrl url){QSaveFile file(url.toLocalFile());if(!file.open(QIODevice::WriteOnly))return;const QJsonObject data{{"applicationVersion",QCoreApplication::applicationVersion()},{"qtVersion",qVersion()},{"connected",online()},{"player",appName()},{"lyricsLoaded",m_lyrics.size()},{"lyricSource",lyricSource()},{"smtcStatus",nativeStatus()},{"appearancePreset",m_settings.value("appearance/preset","balanced").toString()}};file.write(QJsonDocument(data).toJson());file.commit();}
QVariant Bridge::setting(QString key,QVariant fallback)const{
 if(key=="cache/directory")return m_settings.value("preferences/cache/directory",QString()).toString();
 if(key=="info/player")return appName().isEmpty()?tr("No media"):appName();
 if(key=="info/volume")return volumeKnown()?QString::number(qRound(volume()*100))+"%":tr("Unavailable");
 auto value=m_settings.value("preferences/"+key,fallback);if(!value.isValid())return value;
 QVariant normalized;if(SettingsSchema::normalize(key,value,&normalized))return normalized;
 const auto descriptor=SettingsSchema::descriptor(key);if(!descriptor.isEmpty()&&descriptor["type"]!="readonly"&&descriptor["type"]!="action")return fallback;
 if(booleanSettings.contains(key))return value.toBool();if(key=="glass")return value.toDouble();
 if(QStringList{"lyricSize","lyrics/offset","floatX","floatY","lyricsX","lyricsY"}.contains(key))return value.toInt();
 return value;
}
void Bridge::setSetting(QString key,QVariant value){
 if(key=="cache/directory"){setCacheDirectory(value.toString().isEmpty()?QUrl():QUrl::fromLocalFile(value.toString()));return;}
 if(value.metaType()==QMetaType::fromType<QJSValue>())value=value.value<QJSValue>().toVariant();QVariant normalized;
 if(SettingsSchema::normalize(key,value,&normalized))value=normalized;
 else if(key=="glass"){bool ok=false;double number=value.toDouble(&ok);if(!ok||!std::isfinite(number))return;value=qBound(0.,number,.5);}
 else if(key=="lyricSize"||key=="floatX"||key=="floatY"||key=="lyricsX"||key=="lyricsY"){bool ok=false;int number=value.toInt(&ok);if(!ok)return;value=key=="lyricSize"?qBound(18,number,72):qBound(-32000,number,32000);}
 else if(key=="notifications"){if(value.metaType().id()!=QMetaType::Bool)return;}
 else return;
 m_settings.setValue("preferences/"+key,value);
   // Choosing a lyric layout is an explicit request to show home lyrics. Older
   // versions used the Dock button to persist showLyrics=false independently.
   if(key=="home/layout"&&value.toInt()!=1)m_settings.setValue("preferences/showLyrics",true);
 if(key=="general/autostart"){QSettings run("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",QSettings::NativeFormat);if(value.toBool())run.setValue("DroidMediaLink","\""+QDir::toNativeSeparators(QCoreApplication::applicationFilePath())+"\" --hidden");else run.remove("DroidMediaLink");}
 if(key=="lyrics/phoneSource"||key=="lyrics/online"||key=="lyrics/autoMatch"||key=="lyrics/providerPriority"||key=="lyrics/preferredProvider"||key=="lyrics/preferCurrentApp"||key=="lyrics/matchStrategy"||key=="lyrics/crossSourceTranslation"||key.startsWith("lyrics/providers/")){cancelLyricRequest();m_lyricKey.clear();fetchLyrics();}
 if(key=="connection/autoReconnect"&&!value.toBool())m_retry.stop();
 if(key=="theme/source"||key=="theme/brand"){applyTheme();emit changed();}
 if(key.startsWith("cover/")){m_coverRequestKey.clear();enhanceCover();}
 if(key=="media/smtc")updateNative();if(key.startsWith("lyrics/providers/"))emit lyricProvidersChanged();emit settingsChanged();
}
void Bridge::resetSettings(){setCacheDirectory(QUrl());const auto cacheDirectory=m_settings.value("preferences/cache/directory");m_settings.remove("preferences");if(!cacheDirectory.toString().isEmpty())m_settings.setValue("preferences/cache/directory",cacheDirectory);m_settings.remove("appearance");for(auto defaults=factoryDefaults();auto key:defaults.keys())m_settings.setValue(key,defaults.value(key).toVariant());m_settings.setValue("preferences/lyrics/autoMatchMigration",true);setSetting("general/autostart",false);cancelLyricRequest();m_lyricKey.clear();fetchLyrics();emit settingsChanged();}
namespace {
QString lyricPresetKey(const QString &name){return "lyricPresets/custom/"+QString::fromLatin1(name.toUtf8().toBase64(QByteArray::Base64UrlEncoding|QByteArray::OmitTrailingEquals));}
bool validLyricPresetName(const QString &name){return !name.isEmpty()&&name==name.trimmed()&&name.size()<=64&&std::none_of(name.begin(),name.end(),[](QChar ch){return ch.unicode()<32;});}
bool builtinLyricPreset(const QString &name){return QStringList{"soft","clear","contrast"}.contains(name);}
QVariantMap defaultLyricStyle(){QVariantMap result;for(const auto &value:SettingsSchema::descriptors()){const auto row=value.toMap();if(row.value("lyricStyle").toBool())result[row["key"].toString()]=row["defaultValue"];}return result;}
}
QVariantList Bridge::lyricPresets()const{
 QVariantList rows{QVariantMap{{"name","soft"},{"label",QCoreApplication::translate("Settings","Soft")},{"builtIn",true}},QVariantMap{{"name","clear"},{"label",QCoreApplication::translate("Settings","Clear")},{"builtIn",true}},QVariantMap{{"name","contrast"},{"label",QCoreApplication::translate("Settings","High contrast")},{"builtIn",true}}};
 for(const auto &key:m_settings.allKeys())if(key.startsWith("lyricPresets/custom/")){const auto preset=m_settings.value(key).toMap();if(validLyricPresetName(preset["name"].toString()))rows<<QVariantMap{{"name",preset["name"]},{"label",preset["name"]},{"builtIn",false}};}
 return rows;
}
void Bridge::saveLyricPreset(QString name){
 if(!validLyricPresetName(name)||builtinLyricPreset(name))return;
 if(lyricPresets().size()>=53&&!m_settings.contains(lyricPresetKey(name)))return;
 auto values=defaultLyricStyle();for(auto it=values.begin();it!=values.end();++it)it.value()=setting(it.key(),it.value());
 m_settings.setValue(lyricPresetKey(name),QVariantMap{{"name",name},{"values",values}});emit settingsChanged();
}
void Bridge::applyLyricPreset(QString name){
 QVariantMap values;if(builtinLyricPreset(name)){
  values=defaultLyricStyle();for(const auto &scope:QStringList{"lyrics/","desktopLyrics/"}){
   if(name=="clear"){values[scope+"activeScale"]=1.;values[scope+"fontWeight"]=450;values[scope+"activeWeight"]=500;values[scope+"outlineWidth"]=0.;}
   if(name=="contrast"){values[scope+"activeScale"]=1.14;values[scope+"activeWeight"]=650;values[scope+"pastOpacity"]=.56;values[scope+"futureOpacity"]=.48;values[scope+"outlineWidth"]=.75;}
  }
 }else values=m_settings.value(lyricPresetKey(name)).toMap().value("values").toMap();
 if(values.isEmpty())return;
 for(auto it=values.cbegin();it!=values.cend();++it){if(!SettingsSchema::descriptor(it.key()).value("lyricStyle").toBool())continue;QVariant normalized;if(SettingsSchema::normalize(it.key(),it.value(),&normalized))m_settings.setValue("preferences/"+it.key(),normalized);}
 emit settingsChanged();
}
void Bridge::renameLyricPreset(QString oldName,QString newName){
 if(!validLyricPresetName(newName)||builtinLyricPreset(oldName)||builtinLyricPreset(newName)||oldName==newName||m_settings.contains(lyricPresetKey(newName)))return;
 auto preset=m_settings.value(lyricPresetKey(oldName)).toMap();if(preset.isEmpty())return;preset["name"]=newName;m_settings.setValue(lyricPresetKey(newName),preset);m_settings.remove(lyricPresetKey(oldName));emit settingsChanged();
}
void Bridge::deleteLyricPreset(QString name){if(builtinLyricPreset(name))return;m_settings.remove(lyricPresetKey(name));emit settingsChanged();}
void Bridge::resetLyricSettings(QString scope){
 if(!QStringList{"main","desktop","all"}.contains(scope))return;
 for(const auto &value:SettingsSchema::descriptors()){const auto row=value.toMap();const auto key=row["key"].toString();if(row["lyricStyle"].toBool()&&(scope=="all"||key.startsWith(scope=="main"?"lyrics/":"desktopLyrics/")))m_settings.remove("preferences/"+key);}
 emit settingsChanged();
}

QVariantMap Bridge::lyricStyleConfig(QString prefix)const{QVariantMap result;if(prefix!="lyrics"&&prefix!="desktopLyrics")return result;for(const auto &item:SettingsSchema::descriptors()){const auto row=item.toMap();const auto key=row.value("key").toString();if(row.value("lyricStyle").toBool()&&key.startsWith(prefix+"/"))result[key.mid(prefix.size()+1)]=setting(key,row.value("defaultValue"));}return result;}
void Bridge::resetSetting(QString key){const auto row=SettingsSchema::descriptor(key);if(row.isEmpty()||row["type"]=="readonly"||row["type"]=="action")return;const auto defaults=factoryDefaults();setSetting(key,defaults.contains("preferences/"+key)?defaults.value("preferences/"+key).toVariant():row["defaultValue"]);}
void Bridge::resetAppearanceValue(QString scope,QString key){if(!QStringList{"global","background","dock","popup","floating","desktop"}.contains(scope)||!AppearanceConfig::defaultValue(key).isValid())return;const auto full="appearance/"+scope+"/"+key;const auto defaults=factoryDefaults();if(scope=="global"&&defaults.contains(full))m_settings.setValue(full,defaults.value(full).toVariant());else m_settings.remove(full);emit settingsChanged();}

namespace {
QString appearancePresetKey(const QString &name){return "appearancePresets/custom/"+QString::fromLatin1(name.toUtf8().toBase64(QByteArray::Base64UrlEncoding|QByteArray::OmitTrailingEquals));}
bool builtinAppearancePreset(const QString &name){return QStringList{"clear","balanced","vivid","optical"}.contains(name);}
QVariantMap validatedAppearanceStyle(const QVariantMap &input){
 QVariantMap materials,preferences;const auto source=input.value("materials").toMap();
 for(auto it=source.cbegin();it!=source.cend();++it){const auto parts=it.key().split('/');if(parts.size()!=2||!QStringList{"global","background","dock","popup","floating","desktop"}.contains(parts[0]))continue;QVariant normalized;if(parts[1]=="inherit"&&parts[0]!="global"&&it.value().metaType()==QMetaType::fromType<bool>())materials[it.key()]=it.value();else if(AppearanceConfig::validate(parts[1],it.value(),&normalized))materials[it.key()]=normalized;}
 const auto prefs=input.value("preferences").toMap();for(auto it=prefs.cbegin();it!=prefs.cend();++it){const auto row=SettingsSchema::descriptor(it.key());QVariant normalized;if(!row.isEmpty()&&row["group"].toInt()==4&&row["type"]!="action"&&row["type"]!="readonly"&&SettingsSchema::normalize(it.key(),it.value(),&normalized))preferences[it.key()]=normalized;}
 if(materials.isEmpty())return {};return {{"materials",materials},{"preferences",preferences}};
}
}
QVariantList Bridge::appearancePresets()const{
 QVariantList rows;for(const auto &name:QStringList{"clear","balanced","vivid","optical"}){const auto label=name=="clear"?"Clear":name=="balanced"?"Balanced":name=="vivid"?"Vivid":"Optical";rows<<QVariantMap{{"name",name},{"label",QCoreApplication::translate("SettingsPage",label)},{"builtIn",true}};}
 for(const auto &key:m_settings.allKeys())if(key.startsWith("appearancePresets/custom/")){const auto preset=m_settings.value(key).toMap();if(validLyricPresetName(preset["name"].toString()))rows<<QVariantMap{{"name",preset["name"]},{"label",preset["name"]},{"builtIn",false}};}
 return rows;
}
bool Bridge::saveAppearancePreset(QString name){
 if(!validLyricPresetName(name)||builtinAppearancePreset(name))return false;
 if(appearancePresets().size()>=54&&!m_settings.contains(appearancePresetKey(name))){emit notification(tr("Preset limit reached"));return false;}
 QVariantMap materials,preferences;for(const auto &scope:QStringList{"global","background","dock","popup","floating","desktop"}){const auto config=appearanceConfig(scope);for(auto it=config.cbegin();it!=config.cend();++it)materials[scope+"/"+it.key()]=it.value();if(scope!="global")materials[scope+"/inherit"]=appearanceInherited(scope);}
 for(const auto &entry:SettingsSchema::descriptors()){const auto row=entry.toMap();if(row["group"].toInt()==4&&row["type"]!="action"&&row["type"]!="readonly")preferences[row["key"].toString()]=setting(row["key"].toString(),row["defaultValue"]);}
 m_settings.setValue(appearancePresetKey(name),QVariantMap{{"name",name},{"materials",materials},{"preferences",preferences}});emit settingsChanged();return true;
}
void Bridge::applyAppearancePreset(QString name){
 if(builtinAppearancePreset(name)){m_settings.remove("appearance/global");m_settings.setValue("appearance/preset",name);emit settingsChanged();return;}
 const auto style=validatedAppearanceStyle(m_settings.value(appearancePresetKey(name)).toMap());if(style.isEmpty())return;
 m_settings.remove("appearance");m_settings.setValue("appearance/preset","balanced");const auto materials=style["materials"].toMap(),preferences=style["preferences"].toMap();
 for(auto it=materials.cbegin();it!=materials.cend();++it)m_settings.setValue("appearance/"+it.key(),it.value());for(auto it=preferences.cbegin();it!=preferences.cend();++it)m_settings.setValue("preferences/"+it.key(),it.value());applyTheme();emit settingsChanged();
}
void Bridge::renameAppearancePreset(QString oldName,QString newName){
 if(!validLyricPresetName(newName)||builtinAppearancePreset(oldName)||builtinAppearancePreset(newName)||oldName==newName||m_settings.contains(appearancePresetKey(newName)))return;
 auto preset=m_settings.value(appearancePresetKey(oldName)).toMap();if(preset.isEmpty())return;preset["name"]=newName;m_settings.setValue(appearancePresetKey(newName),preset);m_settings.remove(appearancePresetKey(oldName));emit settingsChanged();
}
void Bridge::deleteAppearancePreset(QString name){if(builtinAppearancePreset(name))return;m_settings.remove(appearancePresetKey(name));emit settingsChanged();}

#pragma once
#include <QCoreApplication>
#include <QJsonDocument>
#include <QVariant>

namespace AppearancePresets {
struct Entry { const char *id; const char *label; };
inline constexpr Entry entries[]{
 {"dark-balanced", QT_TRANSLATE_NOOP("SettingsPage","Balanced dark")},
 {"dark-bright", QT_TRANSLATE_NOOP("SettingsPage","Bright dark")},
 {"dark-immersive", QT_TRANSLATE_NOOP("SettingsPage","Immersive dark")},
 {"dark-cinema", QT_TRANSLATE_NOOP("SettingsPage","Dark cinema")},
 {"light-soft", QT_TRANSLATE_NOOP("SettingsPage","Soft light")},
};

// Appearance-only snapshots. All five original snapshots inherit the global material.
inline QVariantMap configuration(const QString &name){
 static const auto configs=QJsonDocument::fromJson(R"presets({
  "dark-balanced": {
   "materials": {
    "global/backgroundBrightness": 1.3, "global/backgroundDiffusion": 1.3, "global/backgroundDither": 0.75,
    "global/backgroundDynamics": true, "global/backgroundFill": true, "global/backgroundFillReach": 1.35,
    "global/backgroundFillStrength": 0.45, "global/backgroundFlow": 0.15000000000000002, "global/backgroundOpacity": 1,
    "global/backgroundSaturation": 1.75, "global/backgroundShadowProtection": 0.25, "global/backgroundSoftness": 0.2,
    "global/backgroundStrength": 1.1, "global/backgroundVibrance": 1, "global/glassBlur": 0,
    "global/glassDispersion": 0.13, "global/glassDistortion": 0.9500000000000001, "global/glassEdgeGlow": 0.35000000000000003,
    "global/glassEdgeWidth": 3, "global/glassMotion": true, "global/glassMotionIntensity": 0.45,
    "global/glassOpacity": 0, "global/glassRadius": 20, "global/glassRefraction": 0.5,
    "global/glassShadow": 0.03, "global/glassTint": "auto", "global/glassTintStrength": 0.15000000000000002,
    "global/quality": 1
   },
   "preferences": {
    "appearance/style": "liquid", "dock/playStyle": "water", "dock/shape": "capsule",
    "home/artworkScale": 1, "home/hideTitleSuffix": false, "home/layout": 2,
    "home/layouts/0/alignment": "center", "home/layouts/0/coverX": 0, "home/layouts/0/coverY": 0,
    "home/layouts/0/lyricsX": 0, "home/layouts/0/lyricsY": 0, "home/layouts/0/titleX": 0,
    "home/layouts/0/titleY": 0, "home/layouts/1/alignment": "left", "home/layouts/1/coverX": 0,
    "home/layouts/1/coverY": 0, "home/layouts/1/lyricsX": 0, "home/layouts/1/lyricsY": 0,
    "home/layouts/1/titleX": 0, "home/layouts/1/titleY": 0, "home/layouts/2/alignment": "left",
    "home/layouts/2/coverX": 0, "home/layouts/2/coverY": 0, "home/layouts/2/lyricsX": 0,
    "home/layouts/2/lyricsY": 0, "home/layouts/2/titleX": 0, "home/layouts/2/titleY": 0,
    "home/proportionalScaling": false, "home/windowBackgroundOpacity": 1, "light": false,
    "motion": true, "theme/brand": "spotify", "theme/source": "auto"
   }
  },
  "dark-bright": {
   "materials": {
    "global/backgroundBrightness": 1.6, "global/backgroundDiffusion": 1.3, "global/backgroundDither": 0.75,
    "global/backgroundDynamics": true, "global/backgroundFill": true, "global/backgroundFillReach": 1.35,
    "global/backgroundFillStrength": 0.45, "global/backgroundFlow": 0.15000000000000002, "global/backgroundOpacity": 1,
    "global/backgroundSaturation": 1.8, "global/backgroundShadowProtection": 0, "global/backgroundSoftness": 0.45,
    "global/backgroundStrength": 1, "global/backgroundVibrance": 0.55, "global/glassBlur": 0,
    "global/glassDispersion": 0.13, "global/glassDistortion": 0.9500000000000001, "global/glassEdgeGlow": 0.35000000000000003,
    "global/glassEdgeWidth": 3, "global/glassMotion": true, "global/glassMotionIntensity": 0.45,
    "global/glassOpacity": 0, "global/glassRadius": 20, "global/glassRefraction": 0.5,
    "global/glassShadow": 0.03, "global/glassTint": "auto", "global/glassTintStrength": 0.15000000000000002,
    "global/quality": 1
   },
   "preferences": {
    "appearance/style": "liquid", "dock/playStyle": "water", "dock/shape": "capsule",
    "home/artworkScale": 1, "home/hideTitleSuffix": false, "home/layout": 2,
    "home/layouts/0/alignment": "center", "home/layouts/0/coverX": 0, "home/layouts/0/coverY": 0,
    "home/layouts/0/lyricsX": 0, "home/layouts/0/lyricsY": 0, "home/layouts/0/titleX": 0,
    "home/layouts/0/titleY": 0, "home/layouts/1/alignment": "left", "home/layouts/1/coverX": 0,
    "home/layouts/1/coverY": 0, "home/layouts/1/lyricsX": 0, "home/layouts/1/lyricsY": 0,
    "home/layouts/1/titleX": 0, "home/layouts/1/titleY": 0, "home/layouts/2/alignment": "left",
    "home/layouts/2/coverX": 0, "home/layouts/2/coverY": 0, "home/layouts/2/lyricsX": 0,
    "home/layouts/2/lyricsY": 0, "home/layouts/2/titleX": 0, "home/layouts/2/titleY": 0,
    "home/proportionalScaling": false, "home/windowBackgroundOpacity": 1, "light": false,
    "motion": true, "theme/brand": "spotify", "theme/source": "auto"
   }
  },
  "dark-immersive": {
   "materials": {
    "global/backgroundBrightness": 0.55, "global/backgroundDiffusion": 0.75, "global/backgroundDither": 1,
    "global/backgroundDynamics": true, "global/backgroundFill": true, "global/backgroundFillReach": 1.35,
    "global/backgroundFillStrength": 0.7000000000000001, "global/backgroundFlow": 0.1, "global/backgroundOpacity": 0.65,
    "global/backgroundSaturation": 2, "global/backgroundShadowProtection": 0, "global/backgroundSoftness": 0.4,
    "global/backgroundStrength": 1.3, "global/backgroundVibrance": 0.7000000000000002, "global/glassBlur": 2.4,
    "global/glassDispersion": 0.07, "global/glassDistortion": 1, "global/glassEdgeGlow": 0.45,
    "global/glassEdgeWidth": 1.33, "global/glassMotion": true, "global/glassMotionIntensity": 0.35,
    "global/glassOpacity": 0.12, "global/glassRadius": 24, "global/glassRefraction": 1,
    "global/glassShadow": 0.06, "global/glassTint": "auto", "global/glassTintStrength": 0.4,
    "global/quality": 1
   },
   "preferences": {
    "appearance/style": "liquid", "dock/playStyle": "water", "dock/shape": "capsule",
    "home/artworkScale": 1, "home/hideTitleSuffix": false, "home/layout": 2,
    "home/layouts/0/alignment": "center", "home/layouts/0/coverX": 0, "home/layouts/0/coverY": 0,
    "home/layouts/0/lyricsX": 0, "home/layouts/0/lyricsY": 0, "home/layouts/0/titleX": 0,
    "home/layouts/0/titleY": 0, "home/layouts/1/alignment": "left", "home/layouts/1/coverX": 0,
    "home/layouts/1/coverY": 0, "home/layouts/1/lyricsX": 0, "home/layouts/1/lyricsY": 0,
    "home/layouts/1/titleX": 0, "home/layouts/1/titleY": 0, "home/layouts/2/alignment": "left",
    "home/layouts/2/coverX": 0, "home/layouts/2/coverY": 0, "home/layouts/2/lyricsX": 0,
    "home/layouts/2/lyricsY": 0, "home/layouts/2/titleX": 0, "home/layouts/2/titleY": 0,
    "home/proportionalScaling": false, "home/windowBackgroundOpacity": 1, "light": false,
    "motion": true, "theme/brand": "spotify", "theme/source": "auto"
   }
  },
  "dark-cinema": {
   "materials": {
    "global/backgroundBrightness": 0.55, "global/backgroundDiffusion": 0.75, "global/backgroundDither": 1,
    "global/backgroundDynamics": true, "global/backgroundFill": true, "global/backgroundFillReach": 1.35,
    "global/backgroundFillStrength": 0.7000000000000001, "global/backgroundFlow": 0.1, "global/backgroundOpacity": 0.5,
    "global/backgroundSaturation": 2, "global/backgroundShadowProtection": 0, "global/backgroundSoftness": 0.4,
    "global/backgroundStrength": 0.1, "global/backgroundVibrance": 0.8, "global/glassBlur": 2.4,
    "global/glassDispersion": 0.07, "global/glassDistortion": 1, "global/glassEdgeGlow": 0.45,
    "global/glassEdgeWidth": 1.33, "global/glassMotion": true, "global/glassMotionIntensity": 0.35,
    "global/glassOpacity": 0.12, "global/glassRadius": 24, "global/glassRefraction": 1,
    "global/glassShadow": 0.06, "global/glassTint": "auto", "global/glassTintStrength": 0.4,
    "global/quality": 1
   },
   "preferences": {
    "appearance/style": "liquid", "dock/playStyle": "water", "dock/shape": "capsule",
    "home/artworkScale": 1, "home/hideTitleSuffix": false, "home/layout": 2,
    "home/layouts/0/alignment": "center", "home/layouts/0/coverX": 0, "home/layouts/0/coverY": 0,
    "home/layouts/0/lyricsX": 0, "home/layouts/0/lyricsY": 0, "home/layouts/0/titleX": 0,
    "home/layouts/0/titleY": 0, "home/layouts/1/alignment": "left", "home/layouts/1/coverX": 0,
    "home/layouts/1/coverY": 0, "home/layouts/1/lyricsX": 0, "home/layouts/1/lyricsY": 0,
    "home/layouts/1/titleX": 0, "home/layouts/1/titleY": 0, "home/layouts/2/alignment": "left",
    "home/layouts/2/coverX": 0, "home/layouts/2/coverY": 0, "home/layouts/2/lyricsX": 0,
    "home/layouts/2/lyricsY": 0, "home/layouts/2/titleX": 0, "home/layouts/2/titleY": 0,
    "home/proportionalScaling": false, "home/windowBackgroundOpacity": 1, "light": false,
    "motion": true, "theme/brand": "spotify", "theme/source": "auto"
   }
  },
  "light-soft": {
   "materials": {
    "global/backgroundBrightness": 1, "global/backgroundDiffusion": 1.1, "global/backgroundDither": 0.55,
    "global/backgroundDynamics": true, "global/backgroundFill": true, "global/backgroundFillReach": 1.4500000000000002,
    "global/backgroundFillStrength": 0.8500000000000001, "global/backgroundFlow": 0.15000000000000002, "global/backgroundOpacity": 1,
    "global/backgroundSaturation": 2, "global/backgroundShadowProtection": 0.35, "global/backgroundSoftness": 0.4,
    "global/backgroundStrength": 1.6, "global/backgroundVibrance": 1, "global/glassBlur": 0.7,
    "global/glassDispersion": 0.07, "global/glassDistortion": 1, "global/glassEdgeGlow": 0.45,
    "global/glassEdgeWidth": 1.33, "global/glassMotion": true, "global/glassMotionIntensity": 0.35,
    "global/glassOpacity": 0.06, "global/glassRadius": 24, "global/glassRefraction": 0.7,
    "global/glassShadow": 0.06, "global/glassTint": "auto", "global/glassTintStrength": 0.4,
    "global/quality": 1
   },
   "preferences": {
    "appearance/style": "liquid", "dock/playStyle": "water", "dock/shape": "capsule",
    "home/artworkScale": 1, "home/hideTitleSuffix": false, "home/layout": 2,
    "home/layouts/0/alignment": "center", "home/layouts/0/coverX": 0, "home/layouts/0/coverY": 0,
    "home/layouts/0/lyricsX": 0, "home/layouts/0/lyricsY": 0, "home/layouts/0/titleX": 0,
    "home/layouts/0/titleY": 0, "home/layouts/1/alignment": "left", "home/layouts/1/coverX": 0,
    "home/layouts/1/coverY": 0, "home/layouts/1/lyricsX": 0, "home/layouts/1/lyricsY": 0,
    "home/layouts/1/titleX": 0, "home/layouts/1/titleY": 0, "home/layouts/2/alignment": "left",
    "home/layouts/2/coverX": 0, "home/layouts/2/coverY": 0, "home/layouts/2/lyricsX": 0,
    "home/layouts/2/lyricsY": 0, "home/layouts/2/titleX": 0, "home/layouts/2/titleY": 0,
    "home/proportionalScaling": false, "home/windowBackgroundOpacity": 1, "light": true,
    "motion": true, "theme/brand": "spotify", "theme/source": "auto"
   }
  }
 })presets").toVariant().toMap();
 return configs.value(name).toMap();
}
}

import QtQuick
import QtQuick.Controls
import "Motion.js" as Motion

ToolTip {
    id: hint
    property Item materialSource: null
    property bool light: false
    property bool liquid: true
    property bool motion: true
    property real motionRate:1
    property color foreground: light ? "#202431" : "#f5f5fa"
    property color surface: light ? "#f5f6fb" : "#222936"
    popupType: Popup.Item
    delay: 480
    margins: 8
    padding: 8
    leftPadding: 12
    rightPadding: 12
    x: parent ? (parent.width-width)/2 : 0
    y: -height-8
    width: Math.min(320,implicitWidth)
    font.pixelSize: 13
    contentItem: Label {
        text:hint.text;textFormat:Text.PlainText
        font:hint.font;color:hint.foreground;wrapMode:Text.Wrap
    }
    background: GlassMaterial {
        sourceItem:hint.visible?hint.materialSource:null
        glassEnabled:hint.liquid&&GraphicsInfo.api!==GraphicsInfo.Software
        dark:!hint.light;motion:false;active:hint.visible
        backdropColor:hint.surface;tintColor:"white";fallbackColor:hint.surface
        configuration:({glassRadius:12,glassOpacity:hint.light?.16:.045,glassTintStrength:1,
            glassBlur:3.8,glassRefraction:1.25,glassDistortion:1.05,glassDispersion:.19,
            glassEdgeGlow:.8,glassEdgeWidth:1.4,glassShadow:0,glassMotion:false,quality:0})
    }
    enter:Transition{NumberAnimation{property:"opacity";from:0;to:1;duration:hint.motion?Motion.duration(130,hint.motionRate):0}}
    exit:Transition{NumberAnimation{property:"opacity";to:0;duration:hint.motion?Motion.duration(95,hint.motionRate):0}}
}

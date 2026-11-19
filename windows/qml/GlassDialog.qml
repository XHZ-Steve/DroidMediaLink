import QtQuick
import QtQuick.Controls

// Standard Dialog semantics, shared optical surface and keyboard-only focus ring.
Dialog {
    id: control
    property Item materialSource: null
    property var configuration: ({})
    property bool light: false
    property bool motion: true
    property bool glassEnabled: true
    property bool highContrast: false
    property color foreground: light ? "#202431" : "#f5f5fa"
    property color surface: light ? "#edf0f6" : "#202532"
    property color accent: "#b5c9ee"
    readonly property real panelRadius: Math.max(0,configuration.glassRadius ?? 24)
    readonly property real footerInset: Math.max(20,panelRadius/2)
    readonly property real footerBottomInset: Math.max(14,Math.ceil(panelRadius*.3))
    padding: 24
    spacing: 20
    modal: true
    popupType: Popup.Item
    header: Label {
        text: control.title; visible: text.length > 0
        leftPadding: 24; rightPadding: 24; topPadding: 24; bottomPadding: 4
        color: control.foreground; font.pixelSize: 20; font.weight: Font.DemiBold
        wrapMode: Text.Wrap
    }
    footer: DialogButtonBox {
        standardButtons: control.standardButtons
        topPadding: 2; leftPadding: control.footerInset; rightPadding: control.footerInset
        bottomPadding: control.footerBottomInset; spacing: 8; alignment: Qt.AlignRight
        delegate: Button {
            id: action
            implicitWidth: Math.max(88,implicitContentWidth+32); implicitHeight: 40
            hoverEnabled: true
            contentItem: Label { text: action.text; color: control.foreground; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            background: Rectangle {
                radius: 12
                color: Qt.rgba(control.accent.r,control.accent.g,control.accent.b,action.down?.24:action.hovered?.16:.08)
                border.width: action.visualFocus?2:0; border.color: control.accent
            }
        }
        background: Item {}
        onAccepted: control.accept()
        onRejected: control.reject()
    }
    background: GlassMaterial {
        sourceItem: control.materialSource; configuration: control.configuration
        active: control.visible; motion: control.motion
        glassEnabled: control.glassEnabled; dark: !control.light
        backdropColor: control.surface; tintColor: control.surface; fallbackColor: control.surface
        increasedContrast: control.highContrast?1:0; cornerRadius: 24
    }
    Overlay.modal: Rectangle { color: control.light?"#25202c48":"#65060812" }
    enter: Transition { NumberAnimation { property: "opacity"; from: 0; to: 1; duration: control.motion?150:0 } }
    exit: Transition { NumberAnimation { property: "opacity"; from: 1; to: 0; duration: control.motion?120:0 } }
}

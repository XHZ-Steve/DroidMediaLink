pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Effects

Item {
    id: cover
    signal activated()
    property string actionText:qsTr("Switch layout")
    property color ambientColor:"#527a9c"
    property real cornerRadius:0
    property real rasterScale:1
    property url source
    property bool motion: true
    property int transitionDuration:340
    property int front: 0
    property int retryAttempts:0
    Timer{id:reloadRetry;interval:Math.min(4000,300*Math.pow(2,cover.retryAttempts));onTriggered:{const next=cover.front===0?b:a;next.source="";cover.load()}}
    function retryFailed(){if(retryAttempts>=3)return;++retryAttempts;reloadRetry.restart()}
    property url pending
    property real blend:1
    readonly property bool ready:a.status===Image.Ready||b.status===Image.Ready
    function present(index) {
        if(front===index)return
        fade.stop();front=index;blend=motion?0:1
        if(motion)fade.start()
    }
    function load() {
        if(!source.toString().length) {a.source="";b.source="";return}
        var current=front===0?a:b
        if(current.source.toString()===source.toString())return
        pending=source
        var next=front===0?b:a
        if(next.source===source && next.status===Image.Ready)present(front===0?1:0)
        else next.source=source
    }
    onSourceChanged:{retryAttempts=0;reloadRetry.stop();load()}
    // Keep the outgoing image opaque underneath; opposing alpha animations
    // would expose the background and darken the cover halfway through a fade.
    RectangularShadow {anchors.fill:parent;visible:cover.ready;offset:Qt.vector2d(0,12);blur:32;radius:Math.max(4,Math.min(cover.cornerRadius,width/2,height/2));color:Qt.rgba(cover.ambientColor.r*.32,cover.ambientColor.g*.32,cover.ambientColor.b*.32,.34)}
    component ArtworkImage: Image {
        id:picture
        anchors.fill:parent
        asynchronous:true;smooth:true;mipmap:true;retainWhileLoading:true
        fillMode:Image.PreserveAspectFit
        layer.enabled:cover.cornerRadius>0&&visible&&GraphicsInfo.api!==GraphicsInfo.Software
        layer.smooth:true
        layer.textureSize:Qt.size(Math.ceil(width*Screen.devicePixelRatio*cover.rasterScale),Math.ceil(height*Screen.devicePixelRatio*cover.rasterScale))
        layer.effect:ShaderEffect {
            property var source
            property vector2d extent:Qt.vector2d(picture.width,picture.height)
            property vector2d artworkExtent:Qt.vector2d(picture.paintedWidth,picture.paintedHeight)
            property real radius:Math.min(cover.cornerRadius,picture.paintedWidth/2,picture.paintedHeight/2)
            fragmentShader:"qrc:/qt/qml/DroidMediaLink/shaders/covermask.frag.qsb"
        }
    }
    Item{id:artworkContent;anchors.fill:parent
    ArtworkImage {id:a;visible:cover.front===0||cover.blend<1;z:cover.front===0?1:0;opacity:cover.front===0?cover.blend:1
        onStatusChanged:if(status===Image.Ready && source===cover.pending){cover.retryAttempts=0;reloadRetry.stop();cover.present(0)}else if(status===Image.Error&&source===cover.pending)cover.retryFailed()
    }
    ArtworkImage {id:b;visible:cover.front===1||cover.blend<1;z:cover.front===1?1:0;opacity:cover.front===1?cover.blend:1
        onStatusChanged:if(status===Image.Ready && source===cover.pending){cover.retryAttempts=0;reloadRetry.stop();cover.present(1)}else if(status===Image.Error&&source===cover.pending)cover.retryFailed()
    }
    }
    Text{anchors.centerIn:parent;visible:!cover.ready;text:"♪";font.family:"Segoe UI";font.pixelSize:Math.max(40,Math.min(80,cover.width*.2));color:cover.ambientColor;opacity:.5}
    Button{objectName:"coverLayoutSwitch";anchors.fill:parent;text:cover.actionText;Accessible.name:text;hoverEnabled:true;background:Rectangle{color:"transparent";radius:Math.max(4,Math.min(cover.cornerRadius,width/2,height/2));border.width:parent.visualFocus?2:0;border.color:cover.ambientColor}contentItem:Item{}
 onClicked:cover.activated();ToolTip.visible:hovered;ToolTip.delay:900;ToolTip.text:text}
    NumberAnimation{id:fade;target:cover;property:"blend";from:0;to:1;duration:cover.transitionDuration;easing.type:Easing.InOutCubic}
    Component.onCompleted:load()
}

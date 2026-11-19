pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import "Motion.js" as Motion

Item {
 id:view
 required property var controller
 property int settingsRevision:0
 property real rasterScale:1
 property bool light:false
 readonly property real pixelDensity:Screen.devicePixelRatio*rasterScale
 property string settingsPrefix:compact?"desktopLyrics":"lyrics"
 readonly property bool desktop:settingsPrefix==="desktopLyrics"
 property var configuration:({})
 property var sampledOptions:({})
 function sampleSettings(){if(controller&&typeof controller.lyricStyleConfig==="function"){const next=controller.lyricStyleConfig(settingsPrefix);if(JSON.stringify(next)!==JSON.stringify(sampledOptions))sampledOptions=next}}
 Timer{id:styleSample;interval:100;onTriggered:view.sampleSettings()}
 onSettingsRevisionChanged:if(active)styleSample.restart()
 function option(key,fallback){if(configuration&&configuration[key]!==undefined)return configuration[key];if(sampledOptions[key]!==undefined)return sampledOptions[key];return controller&&typeof controller.setting==="function"?controller.setting(settingsPrefix+"/"+key,fallback):fallback}
 function colorOption(key,fallback){var value=option(key,"auto");return value==="auto"?fallback:value}
 signal seekRequested(real position)
 property color textColor:"#fff8f5"
 property color inactiveColor:"#bdc5d5"
 property string fontFamily:option("fontFamily",type.displayFamily)
 property real fontSize:option("fontSize",desktop?36:28)
 readonly property real scrollRate:Motion.rate(controller,settingsRevision,"lyricsScroll")
 readonly property real highlightRate:Motion.rate(controller,settingsRevision,"lyricsHighlight")
 function scrollTime(ms){return Motion.duration(ms,scrollRate)}
 function highlightTime(ms){return Motion.duration(ms,highlightRate)}
 property int scrollDuration:scrollTime(option("scrollDuration",420))
 property string easingCurve:option("easing","outCubic")
 property real focusPosition:option("focus",desktop?.5:.45)
 property real lineSpacing:option("lineSpacing",desktop?.22:.65)
 property real activeScale:option("activeScale",1.10)
 property real inactiveOpacity:option("futureOpacity",desktop?.42:.5)
 property string highlightMode:option("highlight","word")
 property bool motion:true
 property bool clockEnabled:true
 property bool interactive:true
 property real originalFocus:0
 readonly property rect primaryRect:{view.width;view.height;timeline.contentY;timeline.count;const row=timeline.itemAtIndex(line);return row?row.primaryGeometry():Qt.rect(0,height*.45,width,fontSize*1.4)}
 property bool centerCurrentText:false
 property bool flushAlignment:false
 property bool alignmentAnimation:true
 property real alignmentPosition:alignment==="right"?1:alignment==="center"?.5:0
 Behavior on alignmentPosition{enabled:view.alignmentAnimation&&view.active&&view.motion;NumberAnimation{duration:view.highlightTime(220);easing.type:Easing.OutCubic}}
 property bool compact:false
 property int compactLines:2
 property bool showTranslation:option("translation",true)
 property bool showRomanization:option("romanization",false)
 property real outlineWidth:option("outlineWidth",0)
 property color outlineColor:option("outlineColor","#80000000")
 property bool shadow:option("shadow",false)
 property bool centered:compact
 readonly property string alignment:option("alignment",centered?"center":"left")
 readonly property string longLine:option("longLine",desktop?"shrink":"wrap")
 readonly property real reserveScale:Math.max(1,activeScale,option("inactiveScale",1))
 readonly property int fontWeight:option("fontWeight",desktop?450:500)
 readonly property int activeWeight:option("activeWeight",desktop?500:600)
 property real translationGap:desktop?6+option("groupSpacing",10):option("translationSpacing",6)
 Behavior on translationGap{enabled:view.motion&&view.active;NumberAnimation{duration:view.highlightTime(180);easing.type:Easing.InOutCubic}}
 Behavior on fontSize{enabled:view.motion&&view.active;NumberAnimation{duration:view.highlightTime(180);easing.type:Easing.InOutCubic}}
 readonly property color automaticGlowColor:light?"black":"white"
 readonly property real wordGlowRadius:option("wordGlowRadius",10)
 readonly property real wordGlowOpacity:option("wordGlowOpacity",.75)
 readonly property color wordGlowColor:colorOption("wordGlowColor",automaticGlowColor)
 readonly property real inkPadding:Math.ceil(Math.max(5,outlineWidth+2,
  shadow?option("shadowBlur",6)+Math.abs(option("shadowOffset",2)):0,
  option("glow",false)?option("glowRadius",8):0,
  option("wordGlow",false)?wordGlowRadius:0))
 readonly property int rendering:option("rendering","curve")==="native"?Text.NativeRendering:option("rendering","curve")==="qt"?Text.QtRendering:Text.CurveRendering
 readonly property var rows:controller?controller.lyrics:[]
 readonly property int line:controller&&rows.length?Math.max(0,controller.currentLine):-1
 readonly property bool untimed:rows.length>0&&rows[0].untimed===true
 readonly property bool active:visible&&Window.window&&Window.window.visible&&Window.window.visibility!==Window.Minimized
 readonly property real inkOverhang:option("glow",false)||option("wordGlow",false)||outlineWidth>0||shadow?Math.ceil((inkPadding+2)*reserveScale):0
 readonly property bool edgeLayer:active&&(inkOverhang>0||!compact&&(originalFocus>0||(option("fadeStrength",1)>0&&(option("fadeTop",70)>0||option("fadeBottom",70)>0))))&&GraphicsInfo.api!==GraphicsInfo.Software
 readonly property real topFade:Math.min(option("fadeTop",70),height*.28)
 readonly property real bottomFade:Math.min(option("fadeBottom",70),height*.28)
 readonly property var timelinePosition:active&&controller?controller.lyricPosition:0
 readonly property var position:clockEnabled?timelinePosition:0
 property string clockSurface:"lyrics-"+Math.random().toString(36).slice(2)
 readonly property font layoutFace:Qt.font({family:fontFamily,pixelSize:fontSize,weight:Math.max(fontWeight,activeWeight),letterSpacing:option("letterSpacing",0),contextFontMerging:true,preferTypoLineMetrics:true,hintingPreference:Font.PreferNoHinting})
 TypographyTokens{id:type}
 function align(value){return value==="right"?Text.AlignRight:value==="center"?Text.AlignHCenter:Text.AlignLeft}
 function plainChunks(row){const text=row.text||"";const parts=text.match(/[\u2e80-\u9fff\u3040-\u30ff]|[^\s\u2e80-\u9fff\u3040-\u30ff]+\s*|\s+/g)||[];return parts.map(function(part){return {text:part,time:row.time||0,end:row.end||0}})}
 function primaryTokens(){const row=timeline.itemAtIndex(line);return row?row.originalTokens():[]}
 function syncClock(){if(controller&&typeof controller.setLyricsVisible==="function")controller.setLyricsVisible(clockSurface,active&&clockEnabled)}
 // Fit one timing curve per line, never retarget it at word boundaries.
 function scrollControls(row){
  const words=row.words||[];if(words.length<2)return Qt.point(1/3,2/3)
  const start=words[0].time,end=words[words.length-1].end,total=words.reduce(function(sum,w){return sum+Array.from(w.text||"").length},0)
  if(!(end>start)||!total)return Qt.point(1/3,2/3)
  let aa=.08,ab=0,bb=.08,ay=.08/3,by=.08*2/3,advance=0
  for(let i=0;i<words.length;i++){
   const word=words[i],length=Array.from(word.text||"").length;if(!length)continue
   const distance=(advance+length*.5)/total;advance+=length
   let lo=0,hi=1
   for(let n=0;n<16;n++){const u=(lo+hi)/2;if(u*u*(3-2*u)<distance)lo=u;else hi=u}
   const u=(lo+hi)/2,a=3*(1-u)*(1-u)*u,b=3*(1-u)*u*u,t=Math.max(0,Math.min(1,((word.time+word.end)/2-start)/(end-start)))-u*u*u
   aa+=a*a;ab+=a*b;bb+=b*b;ay+=a*t;by+=b*t
  }
  const determinant=aa*bb-ab*ab;if(determinant<=0)return Qt.point(1/3,2/3)
  let x1=Math.max(.08,Math.min(.8,(ay*bb-by*ab)/determinant)),x2=Math.max(.2,Math.min(.92,(by*aa-ay*ab)/determinant))
  if(x1>x2){const mid=(x1+x2)/2;x1=mid;x2=mid}
  return Qt.point(x1,x2)
 }
 function scrollCurve(fraction,controls){
  if(fraction<=0)return 0;if(fraction>=1)return 1
  if(Math.abs(controls.x-1/3)<.000001&&Math.abs(controls.y-2/3)<.000001)return fraction*fraction*(3-2*fraction)
  let lo=0,hi=1
  for(let i=0;i<22;i++){const u=(lo+hi)/2,x=3*(1-u)*(1-u)*u*controls.x+3*(1-u)*u*u*controls.y+u*u*u;if(x<fraction)lo=u;else hi=u}
  const u=(lo+hi)/2;return u*u*(3-2*u)
 }
 function recover(){timeline.userScrolling=false;timeline.cancelFlick();timeline.follow()}
 function canSeekAt(point){
  if(point.x<0||point.x>=width||point.y<0||point.y>=height)return false
  if(option("fadeStrength",1)>0&&(point.y<topFade||point.y>height-bottomFade))return false
  return originalFocus<=0||(point.y>=primaryRect.y&&point.y<primaryRect.y+primaryRect.height)
 }
 onClockEnabledChanged:syncClock()
 onActiveChanged:{if(active)sampleSettings();syncClock();if(!active){scrollMotion.stop();timeline.positioned=false}else Qt.callLater(timeline.follow)}
 onLineChanged:Qt.callLater(timeline.follow)
 onWidthChanged:Qt.callLater(timeline.follow)
 Component.onCompleted:{sampleSettings();syncClock()}
 Component.onDestruction:if(controller&&typeof controller.setLyricsVisible==="function")controller.setLyricsVisible(clockSurface,false)
 clip:!edgeLayer

 component Glyph:Item {
  id:glyph;objectName:"lyricGlyph"
  required property string value
  required property real progress
  required property real limit
  required property font face
  property font metricsFace:face
  required property color inkColor
  property bool highlight:false
  property real lineShift:0
  transform:Translate{x:glyph.lineShift}
  TextMetrics{id:measure;font:glyph.metricsFace;text:glyph.value}
  TextMetrics{id:visibleMeasure;font:glyph.metricsFace;text:glyph.value.replace(/\s+$/,"")}
  readonly property real visibleAdvance:visibleMeasure.advanceWidth
  width:Math.max(1,Math.min(limit,measure.advanceWidth))
  height:Math.max(view.fontSize*view.option("lineHeight",1.25),layoutSizer.implicitHeight)
  Text{id:layoutSizer;visible:false;width:glyph.width;text:glyph.value;textFormat:Text.PlainText;font:glyph.metricsFace;wrapMode:Text.NoWrap;lineHeight:view.option("lineHeight",1.25)}
  Text {id:base;visible:!glyph.highlight;width:glyph.width;clip:view.longLine==="wrap"&&measure.advanceWidth>glyph.width+.5;text:glyph.value;textFormat:Text.PlainText;font:glyph.face;wrapMode:Text.NoWrap;elide:view.longLine==="elide"?Text.ElideRight:Text.ElideNone;color:glyph.inkColor;renderType:view.rendering;renderTypeQuality:104;lineHeight:view.option("lineHeight",1.25)}
  readonly property real fill:view.option("wordFill","continuous")==="step"?(progress>=1?1:0):Math.max(0,Math.min(1,progress))
  readonly property real soft:fill>0&&fill<1?Math.min(fill,.3,view.option("wordSoftness",.04)):0
  Item {
   visible:glyph.highlight;width:Math.max(0,glyph.width*(glyph.fill-glyph.soft));height:glyph.height;clip:true
   Text {width:glyph.width;text:glyph.value;textFormat:Text.PlainText;font:glyph.face;wrapMode:Text.NoWrap;elide:view.longLine==="elide"?Text.ElideRight:Text.ElideNone;color:glyph.inkColor;renderType:view.rendering;renderTypeQuality:104;lineHeight:view.option("lineHeight",1.25)}
  }
  Repeater {model:glyph.highlight&&glyph.soft>0?6:0
   Item {required property int index
    x:Math.max(0,glyph.width*(glyph.fill-glyph.soft))+index*glyph.width*glyph.soft/6
    width:glyph.width*glyph.soft/6;height:glyph.height;clip:true;opacity:1-(index+.5)/6
    Text {x:-parent.x;width:glyph.width;text:glyph.value;textFormat:Text.PlainText;font:glyph.face;wrapMode:Text.NoWrap;color:glyph.inkColor;renderType:view.rendering;renderTypeQuality:104;lineHeight:view.option("lineHeight",1.25)}
   }
  }
 }
 component Line:Item {
  id:rowItem;objectName:"lyricRow-"+index
  required property var modelData
  required property int index
  readonly property var row:modelData
  readonly property bool selected:index===view.line&&(view.untimed||view.controller.currentLine>=0)
  readonly property bool past:index<view.line
  function originalTokens(){
   let prefix="",result=[]
   for(let i=0;i<words.children.length;i++){
    const glyph=words.children[i]
    if(glyph.objectName!=="lyricGlyph")continue
    const token=rowItem.row.words&&rowItem.row.words[result.length]
    const rect=glyph.mapToItem(view,Qt.rect(0,0,glyph.width,glyph.height))
    result.push({text:glyph.value,prefix:prefix,rect:rect,scale:rect.width/Math.max(1,glyph.width),family:glyph.face.family,time:token?token.time:0,end:token?token.end:0,timed:rowItem.wordTimed})
    prefix+=glyph.value
   }
   return result
  }
  property var wrappedLineEdges:({})
  function measureLineEdges(){
   const edges={}
   for(let i=0;i<words.children.length;i++){const glyph=words.children[i];if(glyph.objectName!=="lyricGlyph"||glyph.visibleAdvance<=0)continue;const key=String(Math.round(glyph.y));edges[key]=Math.max(edges[key]||0,glyph.x+glyph.visibleAdvance)}
   wrappedLineEdges=edges
  }
  function primaryGeometry(){return primary.mapToItem(view,Qt.rect(0,0,primary.width,primary.height))}
  readonly property real primaryCenterOffset:view.desktop?(translatedLine.visible?(Math.min(0,originalSlot.height+view.translationGap)+Math.max(originalSlot.height,originalSlot.height+view.translationGap+translationLabel.implicitHeight))/2:originalSlot.height/2):primary.height/2
  // Delegate measurement can finish after the initial follow request. Re-anchor
  // only on layout changes, never on each media-clock tick.
  onYChanged:if(index===view.line)Qt.callLater(timeline.follow)
  onHeightChanged:if(index===view.line)Qt.callLater(timeline.follow)
  readonly property real layoutWidth:Math.max(1,(Math.min(timeline.width,view.option("maxWidth",view.desktop?1800:1000))/view.reserveScale)-view.inkPadding*2-8)
  readonly property font face:Qt.font({family:type.resolvedFamily(rowItem.row.text||"",view.fontFamily),pixelSize:view.fontSize,weight:selected?view.activeWeight:view.fontWeight,letterSpacing:view.option("letterSpacing",0),wordSpacing:!view.desktop&&!rowItem.selected?-view.fontSize*.075:0,contextFontMerging:true,preferTypoLineMetrics:true,hintingPreference:Font.PreferNoHinting})
  readonly property var chunks:view.longLine==="elide"?[{text:row.text||"",time:row.time||0,end:row.end||0}]:row.words&&row.words.length?row.words:view.plainChunks(row)
  readonly property bool wordTimed:view.longLine!=="elide"&&!!row.words&&row.words.length>0
  readonly property font measureFace:Qt.font({family:type.resolvedFamily(rowItem.row.text||"",view.fontFamily),pixelSize:view.fontSize,weight:Math.max(view.fontWeight,view.activeWeight),letterSpacing:view.option("letterSpacing",0),wordSpacing:!view.desktop&&!rowItem.selected?-view.fontSize*.075:0,contextFontMerging:true,preferTypoLineMetrics:true,hintingPreference:Font.PreferNoHinting})
  TextMetrics{id:fullMeasure;font:rowItem.measureFace;text:rowItem.row.text||""}
  readonly property real shrink:view.longLine==="shrink"?Math.max(view.option("minimumScale",.65),Math.min(1,layoutWidth/Math.max(1,flowWidth))):1
  readonly property real naturalWidth:Math.max(1,fullMeasure.advanceWidth)
  readonly property real flowWidth:view.longLine==="wrap"?Math.min(layoutWidth,naturalWidth):naturalWidth
  readonly property real scrollViewportWidth:Math.max(1,Math.min(rowItem.width,view.option("maxWidth",view.desktop?1800:1000)))
  readonly property bool scrolling:view.longLine==="scroll"&&rowItem.naturalWidth*rowItem.visualScale>rowItem.scrollViewportWidth
  readonly property real scrollStart:row.words&&row.words.length?row.words[0].time:row.time||0
  readonly property real scrollEnd:row.words&&row.words.length?row.words[row.words.length-1].end:row.end||scrollStart
  readonly property real scrollFraction:scrollEnd>scrollStart?Math.max(0,Math.min(1,(view.timelinePosition-scrollStart)/(scrollEnd-scrollStart))):0
  TextMetrics{id:tailMeasure;font:rowItem.measureFace;text:Array.from(rowItem.row.text||" ").slice(-1).join("")||" "}
  readonly property real scrollDistance:Math.max(0,(primary.width-view.inkPadding*2)*primary.scale-rowItem.scrollViewportWidth)+Math.max(1,tailMeasure.advanceWidth)*primary.scale
  readonly property point timingControls:view.option("scrollWordTiming",false)&&wordTimed?view.scrollControls(row):Qt.point(1/3,2/3)
  readonly property real scrollOffset:scrolling?scrollDistance*view.scrollCurve(scrollFraction,timingControls):0
  readonly property real sourceWidth:view.longLine==="elide"?Math.min(layoutWidth,naturalWidth):flowWidth
  readonly property color baseColor:selected?view.colorOption("unsungColor",view.inactiveColor):view.untimed?view.textColor:past?view.colorOption("pastColor",view.inactiveColor):view.colorOption("futureColor",view.inactiveColor)
  readonly property real stateOpacity:selected||view.untimed?view.option("activeOpacity",1):view.line<0?.78:Math.min(1,(past?view.option("pastOpacity",view.desktop?.5:.58):view.inactiveOpacity)+view.option("nearbyEmphasis",.12)/Math.max(1,Math.abs(index-view.line)))
  readonly property real visualScale:(selected?view.activeScale:view.option("inactiveScale",1))*shrink
  readonly property real centerY:y-timeline.contentY+height/2
  readonly property real fade:view.compact||view.edgeLayer?1:Math.pow(Math.max(0,Math.min(1,view.topFade>0?centerY/view.topFade:1,view.bottomFade>0?(timeline.height-centerY)/view.bottomFade:1)),view.option("fadeStrength",1))
  width:timeline.width
  height:body.height+(view.desktop?0:view.option("groupSpacing",10))
  opacity:(view.compact&&(index<view.line||index>=view.line+view.compactLines)?0:stateOpacity)*fade*(selected?1:1-view.originalFocus)
  Behavior on opacity {enabled:view.active&&view.motion;NumberAnimation{duration:view.highlightTime(view.option("highlightDuration",180))}}
  Column {
   id:body;width:rowItem.layoutWidth*view.reserveScale+view.inkPadding*2
   x:view.flushAlignment?(rowItem.width-width)*view.alignmentPosition+view.inkPadding*primary.scale*(2*view.alignmentPosition-1):view.alignment==="center"?(rowItem.width-width)/2:view.alignment==="right"?rowItem.width-width:4
   spacing:0
   Item {
    id:originalSlot;objectName:"lyricOriginalSlot"
    readonly property real overhang:rowItem.scrolling&&rowItem.selected&&rowItem.scrollOffset<=0&&view.edgeLayer?view.inkOverhang:0
    x:rowItem.scrolling?(rowItem.width-rowItem.scrollViewportWidth)*view.alignmentPosition-body.x-overhang:0
    width:rowItem.scrolling?rowItem.scrollViewportWidth+overhang:parent.width
    height:primary.height*view.reserveScale*rowItem.shrink;clip:rowItem.scrolling
    layer.enabled:view.active&&rowItem.scrolling&&rowItem.selected&&GraphicsInfo.api!==GraphicsInfo.Software
    layer.smooth:true
    layer.textureSize:Qt.size(Math.ceil(width*view.pixelDensity),Math.ceil(height*view.pixelDensity))
    layer.effect:ShaderEffect{
     property var source
     property vector4d fading:Qt.vector4d(0,0,0,1)
     property vector4d focusWindow:Qt.vector4d(rowItem.scrollOffset>0?Math.min(.45,18/Math.max(1,originalSlot.width)):0,rowItem.scrollDistance-rowItem.scrollOffset>.5?Math.min(.45,18/Math.max(1,originalSlot.width)):0,.001,0)
     fragmentShader:"qrc:/qt/qml/DroidMediaLink/shaders/lyricfocus.frag.qsb"
    }
    LyricInk {
     id:primary
     pixelDensity:view.pixelDensity
     width:(view.longLine==="scroll"?Math.max(1,words.childrenRect.width):rowItem.sourceWidth)+view.inkPadding*2;height:words.height+view.inkPadding*2
     x:rowItem.scrolling?originalSlot.overhang-rowItem.scrollOffset-view.inkPadding*scale:view.flushAlignment?(parent.width-width*scale)*view.alignmentPosition:view.alignment==="center"?(parent.width-width)/2:view.alignment==="right"?parent.width-width:0
     scale:rowItem.visualScale;transformOrigin:view.flushAlignment||rowItem.scrolling?Item.TopLeft:view.alignment==="center"?Item.Center:view.alignment==="right"?Item.Right:Item.Left
     outlineWidth:rowItem.selected&&width<=8192?view.outlineWidth:0;outlineColor:view.outlineColor
     shadow:rowItem.selected&&view.shadow&&width<=8192;shadowBlur:view.option("shadowBlur",6);shadowOffset:view.option("shadowOffset",2);shadowColor:view.option("shadowColor","#80000000")
     glow:rowItem.selected&&view.option("glow",false)&&width<=8192;glowRadius:view.option("glowRadius",8);glowOpacity:view.option("glowOpacity",.2);glowColor:view.colorOption("glowColor",view.automaticGlowColor)
     Behavior on scale {enabled:view.active&&view.motion&&view.option("scaleAnimation",true);NumberAnimation{duration:view.highlightTime(view.option("scaleDuration",280));easing.type:Easing.OutCubic}}
     Flow {
      id:words;objectName:"lyricWords";onPositioningComplete:rowItem.measureLineEdges();x:view.inkPadding;y:view.inkPadding;width:view.longLine==="scroll"?100000:rowItem.sourceWidth
      Repeater {model:rowItem.chunks
       Glyph {required property var modelData;lineShift:view.longLine==="wrap"?(rowItem.sourceWidth-(rowItem.wrappedLineEdges[String(Math.round(y))]||rowItem.sourceWidth))*view.alignmentPosition:0;value:modelData.text||"";limit:rowItem.sourceWidth;face:rowItem.face;metricsFace:rowItem.measureFace;progress:0;inkColor:rowItem.baseColor}
      }
     }
     LyricInk {
      pixelDensity:view.pixelDensity
      objectName:"wordGlow-"+rowItem.index
      x:0;y:0;width:primary.width;height:primary.height;visible:rowItem.selected&&!view.untimed
      glow:view.option("wordGlow",false)&&rowItem.wordTimed&&view.highlightMode!=="line"&&width<=8192
      glowRadius:view.wordGlowRadius;glowOpacity:view.wordGlowOpacity;glowColor:view.wordGlowColor
      Flow {x:view.inkPadding;y:view.inkPadding;width:view.longLine==="scroll"?100000:rowItem.sourceWidth
       Repeater {model:rowItem.chunks
        Glyph {required property var modelData;lineShift:view.longLine==="wrap"?(rowItem.sourceWidth-(rowItem.wrappedLineEdges[String(Math.round(y))]||rowItem.sourceWidth))*view.alignmentPosition:0;value:modelData.text||"";limit:rowItem.sourceWidth;face:rowItem.face;metricsFace:rowItem.measureFace;highlight:true;inkColor:view.colorOption("sungColor",view.colorOption("color",view.textColor))
         progress:!rowItem||!view?0:!rowItem.wordTimed||view.highlightMode==="line"?1:Math.max(0,Math.min(1,(view.position-modelData.time)/Math.max(1,modelData.end-modelData.time)))
         Behavior on progress {enabled:view.motion&&view.active;NumberAnimation{duration:view.highlightTime(view.option("wordDuration",70));easing.type:Easing.Linear}}
        }
       }
      }
     }
    }
   }
   Item {
    id:translatedLine
    visible:view.showTranslation&&(rowItem.row.translation||"").length>0
    width:parent.width;height:Math.max(.001,translationLabel.implicitHeight+view.translationGap)
    Text {
    id:translationLabel;objectName:"lyricTranslationText";y:view.translationGap
    width:view.flushAlignment?Math.min(implicitWidth,parent.width):parent.width
    x:view.flushAlignment?(rowItem.width-width)*(view.option("translationAlignment","inherit")==="inherit"?view.alignmentPosition:horizontalAlignment===Text.AlignRight?1:horizontalAlignment===Text.AlignHCenter?.5:0)-body.x:horizontalAlignment===Text.AlignLeft&&view.alignment==="left"?view.inkPadding*primary.scale:horizontalAlignment===Text.AlignRight&&view.alignment==="right"?-view.inkPadding*primary.scale:0
    text:rowItem.row.translation||"";textFormat:Text.PlainText;wrapMode:Text.Wrap
    horizontalAlignment:view.align(view.option("translationAlignment","inherit")==="inherit"?view.alignment:view.option("translationAlignment","inherit"))
    color:view.colorOption("translationColor",view.textColor);opacity:view.option("translationOpacity",.78)*(1-view.originalFocus)
    font.family:type.resolvedFamily(text,view.option("translationFont",type.uiFamily));font.pixelSize:Math.round(view.fontSize*view.option("translationSize",.60));font.weight:view.option("translationWeight",400);font.contextFontMerging:true
    lineHeight:view.option("translationLineHeight",1.2);renderType:view.rendering;renderTypeQuality:104
    }
   }
   Text {
    visible:view.showRomanization&&(rowItem.row.romanization||"").length>0
    width:view.flushAlignment?Math.min(implicitWidth,parent.width):parent.width;topPadding:view.option("romanizationSpacing",6)
    x:view.flushAlignment?(rowItem.width-width)*(view.option("romanizationAlignment","inherit")==="inherit"?view.alignmentPosition:horizontalAlignment===Text.AlignRight?1:horizontalAlignment===Text.AlignHCenter?.5:0)-body.x:horizontalAlignment===Text.AlignLeft&&view.alignment==="left"?view.inkPadding*primary.scale:horizontalAlignment===Text.AlignRight&&view.alignment==="right"?-view.inkPadding*primary.scale:0
    text:rowItem.row.romanization||"";textFormat:Text.PlainText;wrapMode:Text.Wrap
    horizontalAlignment:view.align(view.option("romanizationAlignment","inherit")==="inherit"?view.alignment:view.option("romanizationAlignment","inherit"))
    color:view.colorOption("romanizationColor",view.textColor);opacity:view.option("romanizationOpacity",.62)*(1-view.originalFocus)
    font.family:type.resolvedFamily(text,view.option("romanizationFont",type.uiFamily));font.pixelSize:Math.round(view.fontSize*view.option("romanizationSize",.52));font.weight:view.option("romanizationWeight",400);font.contextFontMerging:true
    lineHeight:view.option("romanizationLineHeight",1.2);renderType:view.rendering;renderTypeQuality:104
   }
  }
  TapHandler {
   enabled:view.interactive&&!view.compact&&!view.untimed
   gesturePolicy:TapHandler.DragThreshold
   onTapped:function(eventPoint){
    if(!timeline.moving&&!timeline.dragging&&view.canSeekAt(rowItem.mapToItem(view,eventPoint.position))){
     view.seekRequested(rowItem.row.time);view.recover()
    }
   }
  }
 }
 // Size the actual effect quad to the overhang. An expanded sourceRect alone
 // compresses that texture into the list width and moves the visible glyphs.
 Item {
  x:-view.inkOverhang;y:0;width:view.width+2*view.inkOverhang;height:view.height
  layer.enabled:view.edgeLayer
  layer.smooth:true
  layer.textureSize:Qt.size(Math.ceil(width*view.pixelDensity),Math.ceil(height*view.pixelDensity))
  layer.effect:ShaderEffect {
   property var source
   property vector4d fading:Qt.vector4d(view.topFade/Math.max(1,view.height),view.bottomFade/Math.max(1,view.height),view.compact?0:view.option("fadeStrength",1),0)
   property vector4d focusWindow:Qt.vector4d(view.primaryRect.y/Math.max(1,view.height),(view.primaryRect.y+view.primaryRect.height)/Math.max(1,view.height),2/Math.max(1,view.height),view.originalFocus)
   fragmentShader:"qrc:/qt/qml/DroidMediaLink/shaders/lyricfocus.frag.qsb"
  }
 ListView {
  id:timeline;objectName:"lyricTimeline";x:view.inkOverhang;width:view.width;height:view.height
  model:view.active?view.rows:null;spacing:Math.round(view.fontSize*view.lineSpacing);cacheBuffer:Math.max(0,height*.5)
  currentIndex:-1;highlight:null;highlightRangeMode:ListView.NoHighlightRange
  property bool userScrolling:false
  property bool positioned:false
  property int browseIndex:-1
  topMargin:height;bottomMargin:height
  boundsBehavior:view.option("bounds","bounce")==="stop"?Flickable.StopAtBounds:Flickable.DragAndOvershootBounds
  maximumFlickVelocity:view.option("inertia",2500);flickDeceleration:view.option("deceleration",1800)
  rebound:Transition{NumberAnimation{properties:"x,y";duration:view.motion?view.scrollTime(view.option("bounceDuration",240)):0;easing.type:Easing.OutCubic}}
  interactive:view.interactive&&!view.compact;flickableDirection:Flickable.VerticalFlick
  function updateBrowseIndex(){
   if(!userScrolling||view.desktop||view.compact||view.untimed||!count)return
   const target=contentY+height*view.focusPosition
   let index=indexAt(width/2,target)
   if(index<0)index=indexAt(width/2,target+spacing)
   if(index<0)index=indexAt(width/2,target-spacing)
   if(index<0&&target<0)index=0
   if(index<0&&target>=contentHeight)index=count-1
   if(index>=0)browseIndex=index
  }
  function manual(){scrollMotion.stop();userScrolling=true;resumeFollow.stop();updateBrowseIndex()}
  function follow(){
   const i=view.line
   if(i<0||i>=count||userScrolling||!view.active||!view.option("autoScroll",true))return
   const row=itemAtIndex(i)
   if(!row){positionViewAtIndex(i,ListView.Center);Qt.callLater(follow);return}
   const focus=view.centerCurrentText?.5:view.compact?(view.compactLines===1?.5:.18):view.focusPosition
   const target=row.y-height*focus+(view.centerCurrentText?row.primaryCenterOffset:!view.compact||view.compactLines===1?row.height/2:0)
   if(Math.abs(target-contentY)<.5)return
   scrollMotion.stop()
   if(view.motion&&positioned){scrollMotion.from=contentY;scrollMotion.to=target;scrollMotion.duration=view.scrollDuration;scrollMotion.start()}else contentY=target
   positioned=true
  }
  onMovementStarted:manual()
  onMovementEnded:resumeFollow.restart()
  onCountChanged:{currentIndex=-1;browseIndex=-1;Qt.callLater(follow)}
  onContentYChanged:updateBrowseIndex()
  onHeightChanged:Qt.callLater(follow)
  delegate:Line{}
  NumberAnimation{id:scrollMotion;target:timeline;property:"contentY";duration:view.scrollDuration;easing.type:view.easingCurve==="linear"?Easing.Linear:view.easingCurve==="inOutCubic"?Easing.InOutCubic:view.easingCurve==="outQuint"?Easing.OutQuint:Easing.OutCubic}
  WheelHandler {enabled:view.interactive&&!view.compact;target:null;acceptedDevices:PointerDevice.Mouse
   onWheel:function(event){
    const start=timeline.userScrolling&&scrollMotion.running?scrollMotion.to:timeline.contentY
    timeline.manual();timeline.cancelFlick()
    const delta=(event.pixelDelta.y||event.angleDelta.y/120*view.fontSize*2)*view.option("wheelSensitivity",1.4)
    const target=Math.max(timeline.originY-timeline.topMargin,Math.min(timeline.originY+timeline.contentHeight-timeline.height+timeline.bottomMargin,start-delta))
    scrollMotion.from=timeline.contentY;scrollMotion.to=target;scrollMotion.duration=view.motion?Math.min(view.scrollTime(200),view.scrollDuration):0
    scrollMotion.start();resumeFollow.restart();event.accepted=true
   }
  }
  Timer{id:resumeFollow;interval:view.option("recoveryDelay",2400);onTriggered:view.recover()}
  ScrollBar.vertical:ScrollBar{visible:view.interactive&&!view.compact;policy:ScrollBar.AsNeeded;implicitWidth:4;background:Item{}
 contentItem:Rectangle{radius:2;color:view.inactiveColor;opacity:.25}}
 }
 }
 Rectangle {
  id:browseIndicator
  objectName:"lyricBrowseIndicator"
  anchors.right:parent.right;anchors.bottom:parent.bottom;anchors.rightMargin:12;anchors.bottomMargin:Math.min(14,view.height*.07)
  readonly property real timestamp:timeline.browseIndex>=0&&timeline.browseIndex<view.rows.length?view.rows[timeline.browseIndex].time:-1
  readonly property real duration:Math.max(1,Number(view.controller?.duration)||0,view.rows.length?(view.rows[view.rows.length-1].end||view.rows[view.rows.length-1].time):0)
  opacity:timeline.userScrolling&&timestamp>=0&&!view.desktop&&!view.compact?1:0
  visible:opacity>0
  Behavior on opacity{enabled:view.motion;NumberAnimation{duration:view.highlightTime(160)}}
  color:view.light?"#d9ffffff":"#c9202635";radius:13
  width:marker.implicitWidth+18;height:30
  Row {
   id:marker;anchors.centerIn:parent;spacing:7
   Rectangle{
    anchors.verticalCenter:parent.verticalCenter;width:54;height:3;radius:1.5;color:Qt.rgba(view.textColor.r,view.textColor.g,view.textColor.b,.28)
    Rectangle{width:parent.width*Math.max(0,Math.min(1,browseIndicator.timestamp/browseIndicator.duration));height:parent.height;radius:1.5;color:view.textColor}
   }
   Label{
    objectName:"lyricBrowseTime"
    readonly property int seconds:Math.max(0,Math.floor(browseIndicator.timestamp/1000))
    text:Math.floor(seconds/60)+":"+String(seconds%60).padStart(2,"0")
    color:view.textColor;font.pixelSize:12
   }
  }
 }
}

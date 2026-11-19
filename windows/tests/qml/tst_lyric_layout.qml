import QtQuick
import QtTest
import "../../qml" as UI
Item {
    id:host;width:740;height:480
    QtObject {
        id:controller
        property var lyrics:[{time:1000,end:5000,text:"There's gotta be another way out",words:[]},{time:5000,end:9000,text:"歌声穿过夜晚 落在窗边",translation:"A quiet line",words:[]}]
        property int currentLine:0
        property real lyricPosition:1000
        function setting(key,fallback){return fallback}
        function setLyricsVisible(surface,visible){}
    }
    UI.LyricView {
        id:lyricView;anchors.fill:parent;controller:controller;motion:false
        configuration:({fontSize:32,fontWeight:400,activeWeight:700,activeScale:1.1,longLine:"wrap",fadeTop:0,fadeBottom:0})
    }
    SignalSpy{id:seeks;target:lyricView;signalName:"seekRequested"}
    TestCase {
        name:"LyricLayout";when:windowShown
        function init(){controller.lyrics=[{time:1000,end:5000,text:"There's gotta be another way out",words:[]},{time:5000,end:9000,text:"歌声穿过夜晚 落在窗边",translation:"A quiet line",words:[]}];lyricView.configuration=({fontSize:32,fontWeight:400,activeWeight:700,activeScale:1.1,longLine:"wrap",fadeTop:0,fadeBottom:0});lyricView.settingsPrefix="lyrics";lyricView.centerCurrentText=false;lyricView.flushAlignment=false;controller.currentLine=0;controller.lyricPosition=1000;seeks.clear();lyricView.recover();wait(30)}
        function glyphPositions(row){const flow=findChild(row,"lyricWords");verify(flow!==null);return JSON.stringify(flow.children.filter(function(child){return child.objectName==="lyricGlyph"}).map(function(child){return [child.x,child.y,child.width,child.height]}))}
        function test_weight_and_scale_preserve_line_breaks(){
            const timeline=findChild(lyricView,"lyricTimeline");tryVerify(function(){return timeline.itemAtIndex(0)!==null})
            const row=timeline.itemAtIndex(0);const layout=glyphPositions(row);const rowHeight=row.height
            controller.currentLine=1;controller.lyricPosition=5100;wait(40)
            const inactive=JSON.parse(glyphPositions(row)),active=JSON.parse(layout)
            compare(inactive.length,active.length)
            for(let i=0;i<active.length;i++){compare(inactive[i][1],active[i][1]);verify(inactive[i][2]<=active[i][2]+.01)}
            compare(row.height,rowHeight)
            controller.currentLine=0;controller.lyricPosition=1500;wait(40)
            compare(glyphPositions(row),layout);compare(row.height,rowHeight)
        }
        function test_click_requests_timeline_seek(){
            const timeline=findChild(lyricView,"lyricTimeline");const row=timeline.itemAtIndex(0);verify(row!==null)
            mouseClick(row,20,20,Qt.LeftButton);tryCompare(seeks,"count",1);compare(seeks.signalArguments[0][0],1000)
        }
        function test_manual_scroll_defers_follow(){
            const timeline=findChild(lyricView,"lyricTimeline");timeline.manual();verify(timeline.userScrolling)
            const y=timeline.contentY;controller.currentLine=1;controller.lyricPosition=6000;wait(30);compare(timeline.contentY,y)
            lyricView.recover();verify(!timeline.userScrolling)
        }
        function test_before_first_timestamp_positions_without_highlighting(){
            controller.currentLine=-1;controller.lyricPosition=0;wait(60)
            const timeline=findChild(lyricView,"lyricTimeline")
            tryVerify(function(){return timeline.positioned})
            const row=timeline.itemAtIndex(0);verify(row!==null);verify(!row.selected);compare(lyricView.line,0)
        }
        function test_negative_translation_offset_centers_pair(){
            lyricView.settingsPrefix="desktopLyrics";lyricView.centerCurrentText=true;lyricView.configuration=({fontSize:32,translation:true,groupSpacing:-50,romanization:true,fadeTop:0,fadeBottom:0})
            controller.lyrics=[{time:1000,end:5000,text:"TEST original",translation:"测试译文",romanization:"TEST romanization",words:[]}];wait(80)
            compare(lyricView.translationGap,-44)
            const row=findChild(lyricView,"lyricTimeline").itemAtIndex(0);verify(row!==null);verify(row.height>0)
            const original=findChild(row,"lyricOriginalSlot"),translation=findChild(row,"lyricTranslationText")
            verify(original!==null&&translation!==null)
            const top=translation.mapToItem(row,0,0).y,bottom=top+translation.height
            verify(Math.abs(row.primaryCenterOffset-(Math.min(0,top)+Math.max(original.height,bottom))/2)<.1)
            lyricView.settingsPrefix="lyrics";lyricView.configuration=({fontSize:32,translationSpacing:-50,fadeTop:0,fadeBottom:0});wait(30);compare(lyricView.translationGap,-50)
        }
        function test_word_timing_uses_one_continuous_curve(){
            const row={words:[{text:"longword",time:0,end:100},{text:"short",time:100,end:200},{text:"final",time:200,end:10000}]}
            const control=lyricView.scrollControls(row);verify(control.x<=control.y);verify(Math.abs(control.x-1/3)>.01||Math.abs(control.y-2/3)>.01)
            let last=0
            for(let i=0;i<=1000;i++){const next=lyricView.scrollCurve(i/1000,control);verify(next>=last);verify(next-last<.02);last=next}
            compare(lyricView.scrollCurve(0,control),0);compare(lyricView.scrollCurve(1,control),1)
        }
        function test_long_line_scroll_eases_and_is_seekable(){
            const words=[];for(let i=0;i<50;i++)words.push({text:"Longword ",time:1000+i*200,end:1200+i*200})
            controller.lyrics=[{time:1000,end:11000,text:words.map(function(w){return w.text}).join(""),words:words}]
            lyricView.configuration=({fontSize:32,activeScale:1.1,longLine:"scroll",alignment:"left",maxWidth:300,fadeTop:0,fadeBottom:0})
            lyricView.flushAlignment=true;wait(60)
            const row=findChild(lyricView,"lyricTimeline").itemAtIndex(0)
            verify(row.scrolling);verify(row.scrollDistance>0);compare(row.scrollViewportWidth,300)
            const flow=findChild(row,"lyricWords")
            flow.children.filter(function(item){return item.objectName==="lyricGlyph"}).forEach(function(glyph){compare(glyph.y,0)})
            controller.lyricPosition=3500;wait(20);verify(Math.abs(row.scrollOffset-row.scrollDistance*.15625)<.1)
            controller.lyricPosition=8500;wait(20);verify(Math.abs(row.scrollOffset-row.scrollDistance*.84375)<.1)
            const paused=row.scrollOffset;wait(60);compare(row.scrollOffset,paused)
            controller.lyricPosition=2000;wait(20);verify(Math.abs(row.scrollOffset-row.scrollDistance*.028)<.1)
        }
    }
}

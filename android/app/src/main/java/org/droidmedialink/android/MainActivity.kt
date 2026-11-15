package org.droidmedialink.android

import com.kyant.backdrop.backdrops.rememberLayerBackdrop
import com.kyant.backdrop.backdrops.layerBackdrop
import com.kyant.backdrop.drawBackdrop
import com.kyant.backdrop.effects.blur
import com.kyant.backdrop.effects.lens
import android.Manifest
import android.content.*
import android.os.Build
import android.provider.Settings
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.result.contract.ActivityResultContracts
import android.os.Bundle
import androidx.compose.foundation.*
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.animation.AnimatedVisibility
import androidx.compose.ui.*
import androidx.compose.ui.graphics.*
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.lifecycle.lifecycleScope
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import kotlinx.coroutines.*
import kotlinx.coroutines.flow.first
import org.json.JSONObject

class MainActivity:ComponentActivity() {
 override fun attachBaseContext(base:Context) {
  val language=base.getSharedPreferences("locale",0).getString("language","").orEmpty()
  if(language.isEmpty())super.attachBaseContext(base) else {
   val config=android.content.res.Configuration(base.resources.configuration);config.setLocale(java.util.Locale.forLanguageTag(language));super.attachBaseContext(base.createConfigurationContext(config))
  }
 }
 private val exportSettings=registerForActivityResult(ActivityResultContracts.CreateDocument("application/json")) { uri -> if(uri!=null)Hub.scope.launch { runCatching { contentResolver.openOutputStream(uri)?.bufferedWriter()?.use { it.write(SettingsTransfer.export(this@MainActivity)) } }.onFailure { Hub.error.value=getString(R.string.settings_error) } } }
 private val importSettings=registerForActivityResult(ActivityResultContracts.OpenDocument()) { uri -> if(uri!=null)Hub.scope.launch { runCatching { val text=contentResolver.openInputStream(uri)!!.bufferedReader().use { reader -> val out=StringBuilder();val buffer=CharArray(1024);while(out.length<=65536) { val count=reader.read(buffer);if(count<0)break;out.append(buffer,0,count) };require(out.length<=65536);out.toString() };SettingsTransfer.import(this@MainActivity,text);recreate() }.onFailure { Hub.error.value=getString(R.string.settings_error) } } }
 fun exportPreferences()=exportSettings.launch("DroidMediaLink-settings.json")
 fun importPreferences()=importSettings.launch(arrayOf("application/json"))
 fun language(value:String) { getSharedPreferences("locale",0).edit().putString("language",value).apply();Hub.set("language",value);recreate() }
 private var pendingUpdate:java.io.File?=null
 private val installPermission=registerForActivityResult(ActivityResultContracts.StartActivityForResult()) {
  val apk=pendingUpdate;pendingUpdate=null
  if(apk!=null&&packageManager.canRequestPackageInstalls())runCatching{openInstaller(apk)}.onFailure{Hub.error.value=getString(R.string.update_error)}
  else Hub.error.value=getString(R.string.update_install_permission)
 }
 fun installUpdate(apk:java.io.File) {
  if(!packageManager.canRequestPackageInstalls()){
   pendingUpdate=apk
   installPermission.launch(Intent(Settings.ACTION_MANAGE_UNKNOWN_APP_SOURCES,android.net.Uri.parse("package:$packageName")))
  } else openInstaller(apk)
 }
 private fun openInstaller(apk:java.io.File) {
  val uri=androidx.core.content.FileProvider.getUriForFile(this,"$packageName.updates",apk)
  startActivity(Intent(Intent.ACTION_VIEW).setDataAndType(uri,"application/vnd.android.package-archive").addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION))
 }
 private val notifications=registerForActivityResult(ActivityResultContracts.RequestPermission()) { }
 override fun onCreate(savedInstanceState:Bundle?) {
  super.onCreate(savedInstanceState)
  if(savedInstanceState?.getBoolean("pendingUpdate")==true)pendingUpdate=java.io.File(cacheDir,"updates/update.apk").takeIf{it.isFile}
  setContent { DroidUI() }
  lifecycleScope.launch {
   val saved=settings.data.first()
   if(saved[androidx.datastore.preferences.core.booleanPreferencesKey("serviceEnabled")]==true && !Hub.running.value)
    runCatching { startForegroundService(Intent(this@MainActivity,PassiveLinkService::class.java)) }.onFailure { Hub.error.value=getString(R.string.service_error) }
  }
 }
 override fun onSaveInstanceState(outState:Bundle){outState.putBoolean("pendingUpdate",pendingUpdate!=null);super.onSaveInstanceState(outState)}
 fun startLink() {
  if(Build.VERSION.SDK_INT>=33)notifications.launch(Manifest.permission.POST_NOTIFICATIONS)
  runCatching { startForegroundService(Intent(this,PassiveLinkService::class.java)) }.onFailure { Hub.error.value=getString(R.string.service_error) }
 }
 override fun onResume() { super.onResume();MediaMonitor.reconnect(this);Hub.refreshMedia?.invoke() }
}
@OptIn(ExperimentalMaterial3Api::class)
@Composable fun DroidUI() {
 val context=LocalContext.current
 val prefs = Hub.preferences.collectAsStateWithLifecycle().value
 val running by Hub.running.collectAsStateWithLifecycle()
 val listenerEnabled by Hub.mediaAccess.collectAsStateWithLifecycle()
 val error by Hub.error.collectAsStateWithLifecycle()
 val media by Hub.media.collectAsStateWithLifecycle()
 val sources by Hub.sources.collectAsStateWithLifecycle()
 val lastSeen by Hub.lastSeen.collectAsStateWithLifecycle()
 val peers by Hub.peers.collectAsStateWithLifecycle()
 val connected by Hub.connected.collectAsStateWithLifecycle()
 val pending by Hub.pending.collectAsStateWithLifecycle()
 var page by remember { mutableIntStateOf(0) }
 var mediaExpanded by rememberSaveable { mutableStateOf(false) }
 var confirmReset by remember { mutableStateOf(false) }
 var confirmRevoke by remember { mutableStateOf<Pair<String,String>?>(null) }
 var help by remember { mutableStateOf<Int?>(null) }
 var recommendations by remember { mutableStateOf(!context.getSharedPreferences("onboarding",0).getBoolean("permissionsV2",false)) }
 val installedVersion=remember(context){ReleaseUpdates.installedVersion(context)}
 var updateNotesExpanded by remember { mutableStateOf(false) }
 var checking by remember { mutableStateOf(false) }
 var availableUpdate by remember { mutableStateOf<ReleaseAsset?>(null) }
 var downloadingUpdate by remember { mutableStateOf(false) }
 var updateProgress by remember { mutableFloatStateOf(0f) }
 var updateJob by remember { mutableStateOf<Job?>(null) }
 val scope=rememberCoroutineScope()
 val dark=when(prefs[androidx.datastore.preferences.core.stringPreferencesKey("theme")]) { "dark"->true;"light"->false;else->isSystemInDarkTheme() }
 val scheme=if(Build.VERSION.SDK_INT>=31) { if(dark)dynamicDarkColorScheme(context) else dynamicLightColorScheme(context) } else if(dark)darkColorScheme(primary=Color(0xffbccbbd),primaryContainer=Color(0xff304b3b),secondaryContainer=Color(0xff34453a),surface=Color(0xff151c1b),surfaceContainer=Color(0xff1b2420)) else lightColorScheme(primary=Color(0xff486655),primaryContainer=Color(0xffdce8dc),secondaryContainer=Color(0xffe2e8df),onSecondaryContainer=Color(0xff202b22),surface=Color(0xfff4f7f1),surfaceContainer=Color(0xffedf0e9))
 val glass=Hub.bool("liquidGlass") && Build.VERSION.SDK_INT>=33
 val backdrop=rememberLayerBackdrop()
 MaterialTheme(colorScheme=scheme) {
  CompositionLocalProvider(LocalContentColor provides scheme.onSurface) {
  Box(Modifier.fillMaxSize()) {
   Box(Modifier.fillMaxSize().background(Brush.linearGradient(listOf(scheme.primaryContainer,scheme.surface,scheme.secondaryContainer))))
  Scaffold(containerColor=Color.Transparent,contentColor=scheme.onSurface) { padding->
   Column(Modifier.fillMaxSize().then(if(glass) Modifier.layerBackdrop(backdrop) else Modifier).padding(padding).background(Brush.verticalGradient(listOf(scheme.primaryContainer.copy(alpha=.45f),scheme.surface,scheme.surface))).verticalScroll(remember(page) { ScrollState(0) }).padding(horizontal=24.dp).padding(top=24.dp,bottom=130.dp),verticalArrangement=Arrangement.spacedBy(20.dp)) {
    Text(stringResource(when(page){1->R.string.devices;2->R.string.settings;else->R.string.app_name}),style=MaterialTheme.typography.headlineLarge,fontWeight=FontWeight.SemiBold)
    if(error.isNotEmpty() && page!=0)Text(error,color=scheme.error)
    if(page==0) {
     Row(Modifier.fillMaxWidth().padding(vertical=12.dp),verticalAlignment=Alignment.CenterVertically) {
      LucideIcon(Lucide.Wifi,null,Modifier.size(32.dp),tint=scheme.primary)
      Column(Modifier.weight(1f).padding(horizontal=16.dp)) { Text(stringResource(R.string.service),style=MaterialTheme.typography.titleMedium);Text(stringResource(if(running)R.string.running else R.string.stopped),color=scheme.onSurfaceVariant) }
      Switch(running,onCheckedChange={ if(it)(context as MainActivity).startLink() else { Hub.set("serviceEnabled",false);context.stopService(Intent(context,PassiveLinkService::class.java)) } })
     }
     if(error.isNotEmpty()) Text(error,color=scheme.error)
     if(!listenerEnabled) FilledTonalButton(onClick={context.startActivity(Intent(Settings.ACTION_NOTIFICATION_LISTENER_SETTINGS))},modifier=Modifier.fillMaxWidth()) { LucideIcon(Lucide.Notifications,null);Spacer(Modifier.width(10.dp));Text(stringResource(R.string.grant_media)) }
     HorizontalDivider(color=scheme.outlineVariant.copy(alpha=.5f))
     MediaPane(media)
     if(sources.size>1) {
      Text(stringResource(R.string.source),style=MaterialTheme.typography.titleSmall)
      FlowRow(horizontalArrangement=Arrangement.spacedBy(8.dp)) {
       FilterChip(selected=Hub.string("manual").isEmpty(),onClick={Hub.set("manual","");Hub.select?.invoke(null)},label={Text(stringResource(R.string.automatic))})
       sources.forEach { (id,name)->FilterChip(selected=Hub.string("manual")==id,onClick={Hub.set("manual",id);Hub.select?.invoke(id)},label={Text(name)}) }
      }
     }
     Row(Modifier.fillMaxWidth(),verticalAlignment=Alignment.CenterVertically) { Text(stringResource(R.string.connected_computers),Modifier.weight(1f),style=MaterialTheme.typography.titleMedium);Text(connected.size.toString(),style=MaterialTheme.typography.headlineSmall,color=scheme.primary) }
     peers.filter { it.first in connected }.forEach { Text(it.second) }
     if(connected.isEmpty()) OutlinedButton(onClick={page=1}) { Text(stringResource(R.string.pair_computer)) }
    }
    if(page==1) {
     if(running) {
      Row(verticalAlignment=Alignment.CenterVertically,horizontalArrangement=Arrangement.spacedBy(14.dp)) {
       LucideIcon(Lucide.Wifi,null,Modifier.size(32.dp),tint=scheme.primary)
       Text(stringResource(R.string.ready_to_pair),style=MaterialTheme.typography.titleMedium)
      }
     } else Button(onClick={(context as MainActivity).startLink()}) { Text(stringResource(R.string.start)) }
     HorizontalDivider()
     peers.forEach { peer-> Row(Modifier.fillMaxWidth(),verticalAlignment=Alignment.CenterVertically) {
      LucideIcon(Lucide.Computer,null);Column(Modifier.weight(1f).padding(16.dp)) { Text(peer.second,style=MaterialTheme.typography.titleMedium);Text(if(peer.first in connected)stringResource(R.string.connected) else lastSeen[peer.first]?.takeIf { it>0 }?.let { java.text.DateFormat.getDateTimeInstance(java.text.DateFormat.SHORT,java.text.DateFormat.SHORT).format(java.util.Date(it)) } ?: stringResource(R.string.authorized),style=MaterialTheme.typography.bodySmall,color=scheme.onSurfaceVariant) }
      IconButton(onClick={confirmRevoke=peer}) { LucideIcon(Lucide.LinkOff,stringResource(R.string.revoke)) }
     } }
    }
    if(page==2) {
     Text(stringResource(R.string.appearance_settings),style=MaterialTheme.typography.titleMedium,color=scheme.primary)
     SettingSwitch(R.string.liquid_glass,Build.VERSION.SDK_INT>=33 && Hub.bool("liquidGlass"),{Hub.set("liquidGlass",it)},{help=R.string.liquid_glass_help},enabled=Build.VERSION.SDK_INT>=33)

      Text(stringResource(R.string.theme),style=MaterialTheme.typography.titleMedium)
      FlowRow(horizontalArrangement=Arrangement.spacedBy(8.dp)) { listOf("" to R.string.system,"light" to R.string.light,"dark" to R.string.dark).forEach { (value,label)->FilterChip(Hub.string("theme")==value,{Hub.set("theme",value)},label={Text(stringResource(label))}) } }
     
     HorizontalDivider(color=scheme.outlineVariant.copy(alpha=.45f))
     Text(stringResource(R.string.background_settings),style=MaterialTheme.typography.titleMedium,color=scheme.primary)
     TextButton(onClick={recommendations=true}) { Text(stringResource(R.string.recommended_settings)) }

      SettingSwitch(R.string.auto_start,Hub.bool("autoStart"),{Hub.set("autoStart",it)},{help=R.string.background_help})
      TextButton(onClick={context.startActivity(Intent(Settings.ACTION_IGNORE_BATTERY_OPTIMIZATION_SETTINGS))}) { Text(stringResource(R.string.battery_settings)) }
     
     HorizontalDivider(color=scheme.outlineVariant.copy(alpha=.45f))
     TextButton(onClick={mediaExpanded=!mediaExpanded},modifier=Modifier.fillMaxWidth(),contentPadding=PaddingValues(vertical=4.dp)) {
      Text(stringResource(R.string.media_settings),Modifier.weight(1f),style=MaterialTheme.typography.titleMedium)
      LucideIcon(if(mediaExpanded)Lucide.ChevronUp else Lucide.ChevronDown,stringResource(if(mediaExpanded)R.string.collapse_media else R.string.expand_media))
     }
     AnimatedVisibility(visible=mediaExpanded) {
      Column(verticalArrangement=Arrangement.spacedBy(16.dp)) {
      OutlinedTextField(Hub.string("preferred"),{Hub.set("preferred",it)},label={Text(stringResource(R.string.preferred_app))},modifier=Modifier.fillMaxWidth(),singleLine=true)
      OutlinedTextField(Hub.string("excluded"),{Hub.set("excluded",it)},label={Text(stringResource(R.string.excluded_apps))},modifier=Modifier.fillMaxWidth(),singleLine=true)
     
     TextButton(onClick={context.startActivity(Intent(Settings.ACTION_NOTIFICATION_LISTENER_SETTINGS))}) { Text(stringResource(R.string.media_permission)) }
      }
     }
     HorizontalDivider(color=scheme.outlineVariant.copy(alpha=.45f))
     Text(stringResource(R.string.app_settings),style=MaterialTheme.typography.titleMedium,color=scheme.primary)
      Text(stringResource(R.string.language),style=MaterialTheme.typography.titleMedium)
      FlowRow(horizontalArrangement=Arrangement.spacedBy(8.dp)) { listOf("" to R.string.system,"en" to R.string.english,"zh-CN" to R.string.chinese).forEach { (value,label)->FilterChip(Hub.string("language")==value,{(context as MainActivity).language(value)},label={Text(stringResource(label))}) } }
     

      TextButton(enabled=!checking&&!downloadingUpdate,onClick={checking=true;scope.launch {
       try { updateNotesExpanded=false;availableUpdate=ReleaseUpdates.check(context);if(availableUpdate==null)android.widget.Toast.makeText(context,R.string.update_current,android.widget.Toast.LENGTH_SHORT).show() }
       catch(error:CancellationException){throw error}
       catch(_:Exception){android.widget.Toast.makeText(context,R.string.update_error,android.widget.Toast.LENGTH_SHORT).show()}
       finally {checking=false}
      }}) { Text(stringResource(if(checking)R.string.update_checking else R.string.check_updates)) }

     TextButton(onClick={help=R.string.privacy_help}) { Text(stringResource(R.string.privacy)) }

      FlowRow { TextButton(onClick={(context as MainActivity).exportPreferences()}) { Text(stringResource(R.string.export_settings)) };TextButton(onClick={(context as MainActivity).importPreferences()}) { Text(stringResource(R.string.import_settings)) };TextButton(onClick={confirmReset=true}) { Text(stringResource(R.string.reset_settings)) } }
     
     Text("DroidMediaLink ${installedVersion.name} (${installedVersion.code})",style=MaterialTheme.typography.labelMedium,color=scheme.onSurfaceVariant)
    }
   }
  }
  GlassNavigation(page=page,onPage={page=it},backdrop=backdrop,glass=glass,modifier=Modifier.align(Alignment.BottomCenter))
  }
  if(recommendations) PermissionOnboarding(onComplete={recommendations=false;context.getSharedPreferences("onboarding",0).edit().putBoolean("permissionsV2",true).apply()})
  availableUpdate?.let { release->AlertDialog(
   onDismissRequest={availableUpdate=null},title={Text(stringResource(R.string.update_available))},
   text={Column(verticalArrangement=Arrangement.spacedBy(8.dp)) {
    Text(stringResource(R.string.update_confirm,release.version))
    if(release.notes.isNotBlank()) {
     TextButton(onClick={updateNotesExpanded=!updateNotesExpanded}){Text(stringResource(if(updateNotesExpanded)R.string.update_notes_hide else R.string.update_notes))}
     AnimatedVisibility(visible=updateNotesExpanded){Text(release.notes,style=MaterialTheme.typography.bodySmall,modifier=Modifier.heightIn(max=220.dp).verticalScroll(rememberScrollState()))}
    }
   }},
   confirmButton={TextButton(onClick={availableUpdate=null;downloadingUpdate=true;updateProgress=0f;updateJob=scope.launch {
    try { val apk=ReleaseUpdates.download(context,release){value->scope.launch{updateProgress=value}};(context as MainActivity).installUpdate(apk) }
    catch(error:CancellationException){throw error}
    catch(_:Exception){android.widget.Toast.makeText(context,R.string.update_error,android.widget.Toast.LENGTH_SHORT).show()}
    finally{downloadingUpdate=false}
   }}){Text(stringResource(R.string.update_install))}},
   dismissButton={TextButton(onClick={availableUpdate=null}){Text(stringResource(R.string.cancel))}})
  }
  if(downloadingUpdate)AlertDialog(onDismissRequest={},title={Text(stringResource(R.string.update_downloading))},
   text={Column(verticalArrangement=Arrangement.spacedBy(12.dp)){LinearProgressIndicator(progress={updateProgress},modifier=Modifier.fillMaxWidth());Text("${(updateProgress*100).toInt()}%")}},confirmButton={},dismissButton={TextButton(onClick={updateJob?.cancel()}){Text(stringResource(R.string.cancel))}})
  if(confirmReset) AlertDialog(onDismissRequest={confirmReset=false},title={Text(stringResource(R.string.reset_settings))},text={Text(stringResource(R.string.reset_warning))},confirmButton={TextButton(onClick={confirmReset=false;scope.launch{SettingsTransfer.reset(context);(context as MainActivity).recreate()}}){Text(stringResource(R.string.reset_settings))}},dismissButton={TextButton(onClick={confirmReset=false}){Text(stringResource(R.string.cancel))}})
  pending?.let { request->AlertDialog(onDismissRequest={request.approve(false)},icon={LucideIcon(Lucide.Computer,null)},title={Text(stringResource(R.string.approve_pairing))},text={Column(verticalArrangement=Arrangement.spacedBy(16.dp)) { Text(request.name);Text(request.sas,style=MaterialTheme.typography.displayMedium,fontWeight=FontWeight.Medium);Text(stringResource(R.string.compare_sas)) }},confirmButton={TextButton(onClick={request.approve(true)}) { Text(stringResource(R.string.codes_match)) }},dismissButton={TextButton(onClick={request.approve(false)}) { Text(stringResource(R.string.reject)) }}) }
  confirmRevoke?.let { peer->AlertDialog(onDismissRequest={confirmRevoke=null},title={Text(stringResource(R.string.revoke))},text={Text(stringResource(R.string.revoke_warning,peer.second))},confirmButton={TextButton(onClick={PairingStore(context).revoke(peer.first);confirmRevoke=null}) { Text(stringResource(R.string.revoke)) }},dismissButton={TextButton(onClick={confirmRevoke=null}) { Text(stringResource(R.string.cancel)) }}) }
  help?.let { text->AlertDialog(onDismissRequest={help=null},text={Text(stringResource(text))},confirmButton={TextButton(onClick={help=null}) { Text(stringResource(R.string.close)) }}) }
 }
}
}
@Composable fun SettingSwitch(title:Int,value:Boolean,onChange:(Boolean)->Unit,onHelp:()->Unit,enabled:Boolean=true) {
 Row(Modifier.fillMaxWidth(),verticalAlignment=Alignment.CenterVertically) { Text(stringResource(title),Modifier.weight(1f));IconButton(onClick=onHelp) { LucideIcon(Lucide.HelpOutline,stringResource(R.string.help)) };Switch(value,onChange,enabled=enabled) }
}
@Composable fun MediaPane(media:JSONObject) {
 val artwork=media.optString("artwork")
 val bitmap=remember(artwork) { runCatching { val data=android.util.Base64.decode(artwork,android.util.Base64.DEFAULT);android.graphics.BitmapFactory.decodeByteArray(data,0,data.size)?.asImageBitmap() }.getOrNull() }
 Row(Modifier.fillMaxWidth().padding(vertical=12.dp),verticalAlignment=Alignment.CenterVertically) {
  Surface(shape=RoundedCornerShape(24.dp),color=MaterialTheme.colorScheme.secondaryContainer,modifier=Modifier.size(112.dp)) {
   if(bitmap!=null) Image(bitmap,null,contentScale=androidx.compose.ui.layout.ContentScale.Crop) else Box(contentAlignment=Alignment.Center) { LucideIcon(Lucide.MusicNote,null,Modifier.size(42.dp)) }
  }
  Column(Modifier.weight(1f).padding(start=20.dp),verticalArrangement=Arrangement.spacedBy(8.dp)) {
   Text(media.optString("title").ifEmpty { stringResource(R.string.no_media) },style=MaterialTheme.typography.titleLarge,maxLines=2,overflow=TextOverflow.Ellipsis)
   if(media.optString("artist").isNotEmpty()) Text(media.optString("artist"),maxLines=2,overflow=TextOverflow.Ellipsis,style=MaterialTheme.typography.bodyMedium,color=MaterialTheme.colorScheme.onSurfaceVariant)
   if(media.optString("appName").isNotEmpty()) Text(media.optString("appName"),maxLines=1,overflow=TextOverflow.Ellipsis,style=MaterialTheme.typography.labelMedium,color=MaterialTheme.colorScheme.primary)
  }
 }
}

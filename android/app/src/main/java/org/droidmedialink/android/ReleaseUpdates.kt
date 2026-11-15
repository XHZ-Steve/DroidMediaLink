package org.droidmedialink.android

import android.content.Context
import java.io.File
import java.net.URL
import javax.net.ssl.HttpsURLConnection
import java.security.MessageDigest
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.ensureActive
import kotlinx.coroutines.withContext
import kotlin.coroutines.coroutineContext
import org.json.JSONObject

internal data class ReleaseAsset(val version:String,val url:String,val size:Long,val digest:String)
internal object ReleaseUpdates {
 fun newer(remote:String,current:String):Boolean {
  val syntax=Regex("v?[0-9]+(\\.[0-9]+){0,3}")
  if(!syntax.matches(remote)||!syntax.matches(current))return false
  val a=remote.removePrefix("v").split('.').map { it.toLongOrNull()?:return false }
  val b=current.removePrefix("v").split('.').map { it.toLongOrNull()?:return false }
  for(i in 0 until maxOf(a.size,b.size)) { val x=a.getOrElse(i){0};val y=b.getOrElse(i){0};if(x!=y)return x>y }
  return false
 }
 private fun connect(url:String):HttpsURLConnection {
  val connection=URL(url).openConnection() as HttpsURLConnection
  connection.connectTimeout=15000;connection.readTimeout=20000
  connection.setRequestProperty("User-Agent","DroidMediaLink/"+BuildConfig.VERSION_NAME)
  connection.setRequestProperty("Accept","application/vnd.github+json")
  return connection
 }
 suspend fun check():ReleaseAsset?=withContext(Dispatchers.IO) {
  val connection=connect("https://api.github.com/repos/XHZ-Steve/DroidMusicLink/releases/latest")
  try {
   val bytes=connection.inputStream.use { input->val output=java.io.ByteArrayOutputStream();val buffer=ByteArray(8192)
    while(true){val n=input.read(buffer);if(n<0)break;require(output.size()+n<=1024*1024);output.write(buffer,0,n)};output.toByteArray() }
   val release=JSONObject(String(bytes,Charsets.UTF_8));val tag=release.getString("tag_name")
   require(Regex("v?[0-9]+(\\.[0-9]+){0,3}").matches(tag))
   if(release.optBoolean("draft")||release.optBoolean("prerelease")||!newer(tag,BuildConfig.VERSION_NAME))return@withContext null
   val assets=release.getJSONArray("assets")
   for(i in 0 until assets.length()) {
    val asset=assets.getJSONObject(i);val name=asset.getString("name");val url=asset.getString("browser_download_url");val digest=asset.optString("digest");val size=asset.optLong("size")
    if(name.startsWith("DroidMediaLink-")&&name.endsWith("-android-test-signed.apk")&&url.startsWith("https://github.com/XHZ-Steve/DroidMusicLink/releases/download/")&&Regex("sha256:[a-fA-F0-9]{64}").matches(digest)&&size in 1..160L*1024*1024)
     return@withContext ReleaseAsset(tag,url,size,digest.removePrefix("sha256:").lowercase())
   }
   error("Missing verified APK")
  } finally { connection.disconnect() }
 }
 suspend fun download(context:Context,asset:ReleaseAsset,progress:(Float)->Unit):File=withContext(Dispatchers.IO) {
  val directory=File(context.cacheDir,"updates").apply { mkdirs() }
  val partial=File(directory,"update.part");val destination=File(directory,"update.apk")
  val connection=connect(asset.url)
  try {
   val hash=MessageDigest.getInstance("SHA-256");var count=0L;var reported=-1
   connection.inputStream.use { input->partial.outputStream().use { output->
    val buffer=ByteArray(65536)
    while(true){coroutineContext.ensureActive();val n=input.read(buffer);if(n<0)break;count+=n;require(count<=asset.size);output.write(buffer,0,n);hash.update(buffer,0,n)
     val percent=(count*100/asset.size).toInt();if(percent!=reported){reported=percent;progress(percent/100f)}
    }
   } }
   require(count==asset.size&&hash.digest().joinToString(""){"%02x".format(it)}==asset.digest)
   @Suppress("DEPRECATION") val pkg=context.packageManager.getPackageArchiveInfo(partial.path,0)
   require(pkg?.packageName==context.packageName&&newer(pkg.versionName.orEmpty(),BuildConfig.VERSION_NAME))
   if(destination.exists())check(destination.delete());check(partial.renameTo(destination));destination
  } finally { connection.disconnect();partial.delete() }
 }
}

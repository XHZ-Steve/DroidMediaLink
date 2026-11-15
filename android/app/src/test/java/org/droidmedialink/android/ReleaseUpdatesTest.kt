package org.droidmedialink.android
import org.junit.Assert.*
import org.junit.Test
class ReleaseUpdatesTest {
 @Test fun normalizedStableVersions(){
  assertFalse(ReleaseUpdates.newer("v1.1","1.1.0"))
  assertFalse(ReleaseUpdates.newer("v1.0.9","1.1.0"))
  assertTrue(ReleaseUpdates.newer("v1.10","1.9.9"))
  assertFalse(ReleaseUpdates.newer("v1.2-beta","1.1.0"))
  assertFalse(ReleaseUpdates.newer("garbage","1.1.0"))
  assertFalse(ReleaseUpdates.newer("v99999999999999999999999999","1.1.0"))
 }
 @Test fun packageVersionAndBuildMustAdvance(){
  assertTrue(ReleaseUpdates.canInstall("1.1.0",4,"0.1.0",1))
  assertTrue(ReleaseUpdates.canInstall("1.1.0",4,"1.1",3))
  assertFalse(ReleaseUpdates.canInstall("1.1.0",4,"1.1.0",4))
  assertFalse(ReleaseUpdates.canInstall("1.0.0",5,"1.1.0",4))
  assertFalse(ReleaseUpdates.canInstall("1.2.0",3,"1.1.0",4))
  assertFalse(ReleaseUpdates.canInstall("bad",5,"1.1.0",4))
 }
 @Test fun releaseNotesAreReadableAndBounded(){
  assertEquals("Changes\nFix updates",ReleaseUpdates.releaseSummary("## Changes\n**Fix** [updates](https://example.com)"))
  assertEquals(4000,ReleaseUpdates.releaseSummary("x".repeat(10000)).length)
 }

}

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
}

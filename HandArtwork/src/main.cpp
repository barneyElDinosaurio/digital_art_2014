#include "ofMain.h"
#include "ofApp.h"
#include "ofAppGLFWWindow.h"

#ifdef TARGET_OPENGLES
#include "ofGLProgrammableRenderer.h"
#endif


//========================================================================
int main( ){
	
	ofSetLogLevel(OF_LOG_VERBOSE);
	#ifdef TARGET_OPENGLES
		ofSetCurrentRenderer(ofGLProgrammableRenderer::TYPE);
	#endif
    
    // NEW VERSION: TWO SCREENS
    // See: http://forum.openframeworks.cc/t/dual-monitor-full-screen/13654
    // be sure to go to system preferences >> mission control >> and set "Displays have separate Spaces" to false.
    // Log out and Log in again, and it will work.
    //
    ofAppGLFWWindow win;
    win.setMultiDisplayFullscreen(true); //this makes the fullscreen window span across all your monitors
    ofSetupOpenGL(&win, 1280,800, OF_FULLSCREEN);
    ofRunApp(new ofApp());
	
    
    /*
    // OLD VERSION: SINGLE SCREEN
	ofSetupOpenGL(1280,800,OF_FULLSCREEN);
	ofRunApp(new ofApp());
    */

}

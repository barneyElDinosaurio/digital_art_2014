#include "testApp.h"

/*
// OPENFRAMEWORKS + LEAP MOTION SDK 2.0 HAND SKELETON DEMO 
// By Golan Levin (@golan), http://github.com/golanlevin
// Uses ofxLeapMotion addon by Theo Watson, with assistance from Dan Wilcox
// Supported in part by the Frank-Ratchye STUDIO for Creative Inquiry at CMU
*/



/* Note on OS X, you must have this in the Run Script Build Phase of your project. 
where the first path ../../../addons/ofxLeapMotion/ is the path to the ofxLeapMotion addon. 

cp -f ../../../addons/ofxLeapMotion/libs/lib/osx/libLeap.dylib "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/MacOS/libLeap.dylib"; install_name_tool -change ./libLeap.dylib @executable_path/libLeap.dylib "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/MacOS/$PRODUCT_NAME";

   If you don't have this you'll see an error in the console: dyld: Library not loaded: @loader_path/libLeap.dylib
*/




//--------------------------------------------------------------
void testApp::setup(){
    
    // all data will be in SharedData
    string basePath = ofToDataPath("", true);
    ofSetDataPathRoot("../../../../../SharedData/");
    
    cameraWidth = 1024;
    cameraHeight = 768;
	drawW = 640;
    drawH = 480;
    
#ifdef _USE_LIBDC_GRABBER
	
    // For the ofxLibdc::PointGrey cameraLibdc;
	cout << "libdc cameras found: " << cameraLibdc.getCameraCount() << endl;

    cameraLibdc.setImageType(OF_IMAGE_COLOR);
    cameraLibdc.setSize (cameraWidth, cameraHeight);
    //cameraLibdc.setBayerMode(DC1394_COLOR_FILTER_GRBG); // check this, why grayscale???

    cameraLibdc.setup();
	//cameraLibdc.setBlocking(true);
    cameraLibdc.setShutterAbs(1. / 31.0);
	cameraLibdc.setExposure(1.0);
	cameraLibdc.setBrightness(0);
	cameraLibdc.setGain(0);
	cameraLibdc.setGammaAbs(1);
    
#else
	cameraVidGrabber.setVerbose(true);
	cameraVidGrabber.initGrabber(cameraWidth,cameraHeight);
#endif
	
	// Setup video saver
	bRecording = false;
	currentFrameNumber = 0;
	imageSequence.resize(500);

    currentFrameImg.allocate(cameraWidth, cameraHeight, OF_IMAGE_COLOR);
    processFrameImg.allocate(cameraWidth, cameraHeight, OF_IMAGE_COLOR);
    
	for(int i = 0; i < imageSequence.size(); i++) {
		imageSequence[i].allocate(cameraWidth,cameraHeight, OF_IMAGE_COLOR);
	}
	
#ifdef _USE_CORRECTED_CAMERA
    ofxCv::FileStorage settings (ofToDataPath("settingsForCameraCalibrator.yml"), ofxCv::FileStorage::READ);
	if(settings.isOpened()) {
        
        int patternXCount = settings["patternXCount"];
        int patternYCount = settings["patternYCount"];
        myCalibration.setPatternSize(patternXCount, patternYCount);
        float squareSize = settings["squareSize"];
        myCalibration.setSquareSize(squareSize);
		
        ofxCv::CalibrationPattern patternType = ofxCv::CHESSBOARD;
        switch(settings["patternType"]) {
            default:
            case 0: patternType = ofxCv::CHESSBOARD; break;
            case 1: patternType = ofxCv::CIRCLES_GRID; break;
            case 2: patternType = ofxCv::ASYMMETRIC_CIRCLES_GRID; break;
        }
        myCalibration.setPatternType(patternType);
		
	}
    
    ofxCv::FileStorage prevCalibrationFile (ofToDataPath("calibration.yml"), ofxCv::FileStorage::READ);
    if (prevCalibrationFile.isOpened()){
        prevCalibrationFile.release();
        myCalibration.load("calibration.yml");
        myCalibration.calibrate();
        if (myCalibration.isReady()){
            cout << "calibration ready" << endl;
        }
    }
#endif
    
    // Setup leap
	leap.open();
	cam.setOrientation(ofPoint(-55, 0, 0));
    
    leapVisualizer.setup();
    leapRecorder.setup();

	bPlaying = false;
	playingFrame = 0;
	bEndRecording = false;
    playing = false;
    bUseVirtualProjector = false;
    bUseFbo = true;
    bInputMousePoints = false;
    bShowCalibPoints = true;
    bRecordingForCalibration = false;
    bRecordThisCalibFrame = false;
    bUseCorrectedCamera = true;
    bShowLargeCamImageOnTop = false;
    bShowText = true;
    
    folderName = ofGetTimestampString();
    lastIndexVideoPos.set(0,0,0);
    lastIndexLeapPos.set(0,0,0);
    
    fbo.allocate(cameraWidth,cameraHeight,GL_RGBA);
    
    
    
    
    // ofSetFrameRate(60);
    ofSetVerticalSync(false);
	ofSetLogLevel(OF_LOG_WARNING);
	ofSetCylinderResolution (16, 1, 16);
    
    
    ofEnableAlphaBlending();
    glEnable(GL_NORMALIZE); // needed??
	glEnable(GL_DEPTH);
    // glEnable(GL_DEPTH_TEST); // why is this messing the render up in the projector cam??????
    
    string versionDisplay = "Using openFrameworks version: " + ofToString( ofGetVersionInfo());
	cout << versionDisplay;
	

    
}


//--------------------------------------------------------------
void testApp::update(){
    
    
    
    //------------- Playback
    if(bPlaying && !bRecording){
        video.update();
        video.setPlaying(playing);
    }
    
    
    
    //------------- Recording
    bool bCameraHasNewFrame = false;
    #ifdef _USE_LIBDC_GRABBER
        if (cameraLibdc.grabVideo(currentFrameImg)) {
            bCameraHasNewFrame = true;
            currentFrameImg.update();
        }
    #else
        cameraVidGrabber.update();
        bCameraHasNewFrame = cameraVidGrabber.isFrameNew();
        if (bCameraHasNewFrame){
            currentFrameImg.setFromPixels(cameraVidGrabber.getPixels(), cameraWidth,cameraHeight, OF_IMAGE_COLOR);
        }
    #endif
    
    
    if(useCorrectedCam()){
        if (myCalibration.size() > 0) {
            myCalibration.undistort(ofxCv::toCv(currentFrameImg), ofxCv::toCv(processFrameImg));
            processFrameImg.update();
        }
    }else{
        processFrameImg = currentFrameImg;
        processFrameImg.update();
    }
    
    if(bRecording && !bPlaying){
        
        if(!bEndRecording && currentFrameNumber >= imageSequence.size()){
            bEndRecording = true;
        }
        
        if(!bEndRecording) {
            
            if(bCameraHasNewFrame){
                
                // if in calibration record mode only record some frames
                if( (bRecordingForCalibration && bRecordThisCalibFrame) || !bRecordingForCalibration){
                    leapRecorder.recordFrameXML(leap);
                    
                    ofPixels& target = imageSequence[currentFrameNumber];
                    memcpy (target.getPixels(),
                            processFrameImg.getPixels(),
                            target.getWidth() * target.getHeight() * target.getNumChannels());
                   
                    currentFrameNumber++;
                    
                    if(bRecordingForCalibration){
                        bRecordThisCalibFrame = false;
                    }
                    
                    // leap.markFrameAsOld();??????
                }
            }
            
        } else {
            
            finishRecording();

        }
    }
    

    //-------------  Leap update
	leap.markFrameAsOld();
    
}


//--------------------------------------------------------------
void testApp::draw(){

    ofBackground(0,0,0);
    
    if(!bPlaying){
        drawLiveForRecording();
    }else{
        drawPlayback();
        
        // draw mouse cursor for calibration input
        if(bPlaying && bInputMousePoints){
            ofPushStyle();
            ofNoFill();
            ofSetColor(0,255,0);
            ofEllipse(mouseX, mouseY-20, 16, 16);
            ofLine(mouseX,mouseY,mouseX,mouseY-12);
            ofLine(mouseX,mouseY-28,mouseX,mouseY-40);
            ofLine(mouseX-8,mouseY-20,mouseX-20,mouseY-20);
            ofLine(mouseX+8,mouseY-20,mouseX+20,mouseY-20);
            ofPopStyle();
        }
    }
	
    if(bShowText){
       drawText();
    }
    
    if(bShowLargeCamImageOnTop){
        ofSetColor(255);
        processFrameImg.draw(0,0,1024,768);
    }

}

//--------------------------------------------------------------
void testApp::drawLiveForRecording(){
    
    // darw live image
    ofSetColor(ofColor::white);
#ifdef _USE_LIBDC_GRABBER
    processFrameImg.draw(drawW,0,drawW,drawH);
#else
    processFrameImg.draw(drawW,0,drawW,drawH);
#endif
            
    
    // darw leap
    drawLeapWorld();
    
}

//--------------------------------------------------------------
void testApp::drawPlayback(){
    
    drawLeapWorld();
    
    // if calibrated and want to see view from projector
    if (leapCameraCalibrator.calibrated && bShowCalibPoints && bUseVirtualProjector){
        ofPushMatrix();
        ofScale(drawW/(float)cameraWidth, drawH/(float)cameraHeight);
        leapCameraCalibrator.drawImagePoints();
        ofPopMatrix();
    }
    
    
    if(bPlaying && !bRecording){
        ofPushStyle();
        ofSetColor(255);
        video.draw(drawW, 0,drawW,drawH);
        
        if(bInputMousePoints){
            indexRecorder.drawPointHistory(video.getCurrentFrameID() );
        }

        ofPopStyle();
    }
}

void testApp::drawLeapWorld(){
    
    // draw video below calibration
    if(leapCameraCalibrator.calibrated && bUseVirtualProjector){
        ofSetColor(255);
        if(bPlaying ){
            video.draw(0, 0, drawW,drawH);
        }else{
            #ifdef _USE_LIBDC_GRABBER
                processFrameImg.draw(0,0,drawW,drawH);
            #else
                processFrameImg.draw(0,0,drawW,drawH);
            #endif
        }
    }
    
    
    // start fbo
    if (bUseFbo){
        fbo.begin();
        ofClear(0,0,0,0);
    }
    
    // start camera
    if (leapCameraCalibrator.calibrated && bUseVirtualProjector){
        leapCameraCalibrator.projector.beginAsCamera();
    }else {
        glEnable(GL_DEPTH_TEST);
        cam.begin();
    }
    
    // draw grid
    ofSetColor(255);
    leapVisualizer.drawGrid();
    
    // draw world points
    if(leapCameraCalibrator.calibrated && bShowCalibPoints){
        leapCameraCalibrator.drawWorldPoints();
    }
    
    // draw leap from xml
    if (bPlaying && !bRecording){
        int nFrameTags = leapVisualizer.XML.getNumTags("FRAME");
        if (nFrameTags > 0){
            ofFill();
            playingFrame = video.getCurrentFrameID();
            leapVisualizer.drawFrameFromXML(playingFrame);
            
            if( lastIndexVideoPos.x > 0 && lastIndexVideoPos.y > 0){
                ofSetColor(ofColor::red);
                ofDrawSphere(lastIndexLeapPos, 5.0f);
            }
            
        }
    }else{
        // draw live leap
        leapVisualizer.drawFrame(leap);
    }
    
    // end camera
	if (leapCameraCalibrator.calibrated && bUseVirtualProjector){
        leapCameraCalibrator.projector.endAsCamera();
    }else {
        
        // draw calibration projector
        if (leapCameraCalibrator.calibrated){
            leapCameraCalibrator.projector.draw();
        }
        cam.end();
        glDisable(GL_DEPTH_TEST);
    }
    
    // end fbo
    if (bUseFbo){
        fbo.end();
    }
    
    
    if (leapCameraCalibrator.calibrated) ofSetColor(255,255,255,200);
    else ofSetColor(255,255,255,255);
    
    ofPushMatrix();
   if(bUseVirtualProjector){
        ofScale(1,-1,1);
        fbo.draw(0,-drawH,drawW,drawH);
    }else{
        fbo.draw(0,0,drawW,drawH);
   }
    ofPopMatrix();
    

}

//--------------------------------------------------------------
void testApp::drawText(){
	
	float textY = 500;
	
	ofSetColor(ofColor::white);
	//ofDrawBitmapString("Display, record & playback Leap 2.0 Controller data.", 20, textY); textY+=15;
	//ofDrawBitmapString("Built in openFrameworks by Golan Levin, golan@flong.com", 20, textY); textY+=15;
	//textY+=15;

	ofSetColor( (leap.isConnected()) ?  ofColor::green : ofColor(255,51,51)) ;
	ofDrawBitmapString( (leap.isConnected() ? "Leap is Connected!" : "Leap is NOT CONNECTED!"), 20, textY);
	ofDrawBitmapString( (leap.isConnected() ? "Leap is Connected!" : "Leap is NOT CONNECTED!"), 21, textY); textY+=15;
	textY+=15;
	
	ofSetColor(ofColor::white);
	ofDrawBitmapString("Press 's' (Space key) to flip display mode", 20, textY); textY+=15;
	ofDrawBitmapString("Press 'c' to restore camera", 20, textY); textY+=15;
	ofDrawBitmapString("Press 'g' to toggle grid", 20, textY); textY+=15;
    ofDrawBitmapString("Press 'v' to toggle virtual proejctor", 20, textY); textY+=15;
    ofDrawBitmapString("Press '1' to select recording from directory", 20, textY); textY+=15;
    ofDrawBitmapString("Press '2' to select calibration from directory", 20, textY); textY+=15;
    ofDrawBitmapString("Press '3' to select calibration recording from directory", 20, textY); textY+=15;
    ofDrawBitmapString("Press 'w' to toggle calibration points draw", 20, textY); textY+=15;
    ofDrawBitmapString("Press 'C' to load current folder's calibration", 20, textY); textY+=15;
    ofDrawBitmapString("Press 'm' to allow mouse input points", 20, textY); textY+=15;
    ofDrawBitmapString("Press 'left/right' to advance frame by frame", 20, textY); textY+=15;


	textY+=15;
	
    textY = 500;
    int textX = 610;
    
    ofDrawBitmapString("All folders must be in SharedData/recordings", textX, textY); textY+=15;
    textY+=15;
    
    ofDrawBitmapString("Press 'l' to return to live mode", textX, textY); textY+=15;
    
    if (leap.isConnected()){
		ofDrawBitmapString("Press 'r' to toggle RECORDING", textX, textY); textY+=15;
        ofDrawBitmapString("Press 'R' to toggle RECORDING for CALIBRATION", textX, textY); textY+=15;
        ofDrawBitmapString("Press ' ' to record CALIBRATION frame", textX, textY); textY+=15;

	}
	if (leapVisualizer.XML.getNumTags("FRAME") > 0){
		ofDrawBitmapString("Press 'p' to pause PLAYBACK",  textX, textY); textY+=15;
	}
	
	if (bPlaying){
		ofSetColor(ofColor::green);
		ofDrawBitmapString("PLAYING! " + ofToString(playingFrame), textX, textY); textY+=15;
	} else if (bRecording){
		ofSetColor(ofColor::red);
		ofDrawBitmapString("RECORDING! " + ofToString(leapRecorder.recordingFrameCount), textX, textY); textY+=15;
	}
    
    textY+=15;
    ofSetColor(ofColor::white);
    ofDrawBitmapString("Playback folder: "+folderName,  textX, textY); textY+=15;
    ofDrawBitmapString("Calibration file: "+leapCameraCalibrator.dirNameLoaded,  textX, textY); textY+=15;

	
}

//--------------------------------------------------------------
void testApp::loadPlaybackFromDialogForCalibration(){
    
    //Open the Open File Dialog
    ofFileDialogResult openFileResult= ofSystemLoadDialog("Choose a recording folder:",true);
    
    if (openFileResult.bSuccess){
        
        string filePath = openFileResult.getName();
        folderName = filePath;
        loadAndPlayRecording(filePath);
        video.setPlaying(true);
        video.update();
        playing = false;
        indexRecorder.setup("recordings/"+folderName+"/calib","fingerCalibPts.xml");
        indexRecorder.setDrawOffsets(drawW,0,cameraWidth/drawW,cameraHeight/drawH);    }
}

//--------------------------------------------------------------
void testApp::loadPlaybackFromDialog(){
    
    //Open the Open File Dialog
    ofFileDialogResult openFileResult= ofSystemLoadDialog("Choose a recording folder:",true);
    
    if (openFileResult.bSuccess){
        
        string filePath = openFileResult.getName();
        folderName = filePath;
        loadAndPlayRecording(filePath);
        
    }
}

//--------------------------------------------------------------
void testApp::loadCalibrationFromDialog(){
    //Open the Open File Dialog
    ofFileDialogResult openFileResult= ofSystemLoadDialog("Choose a recording folder:",true);
    
    if (openFileResult.bSuccess){
        string filePath = openFileResult.getName();
        calibrateFromXML(filePath);
    }
}

//--------------------------------------------------------------
void testApp::finishRecording(){
    
    bRecording = false;
    bEndRecording = false;
    
    ofFileDialogResult openFileResult= ofSystemSaveDialog(folderName,"Make a folder in SharedData/recordings:");
    
    if (openFileResult.bSuccess){
        folderName = openFileResult.getName();
        
        int totalImage = MIN(currentFrameNumber,imageSequence.size());
        for(int i = 0; i < totalImage; i++) {
            if(imageSequence[i].getWidth() == 0) break;
            //imageSequence[i].rotate90(1);
            ofSaveImage(imageSequence[i], "recordings/"+folderName+"/camera/"+ofToString(i, 3, '0') + ".jpg");
        }
        
        ofDirectory dir;
        dir.open("recordings/"+folderName+"/leap");
        if(!dir.exists())dir.createDirectory("recordings/"+folderName+"/leap");
        leapRecorder.endRecording("recordings/"+folderName+"/leap/leap.xml");
        
        loadAndPlayRecording(folderName);

        if(bRecordingForCalibration){
            playing = false;
        }

    }
    
    
    
    imageSequence.clear();
    imageSequence.resize(300);
	for(int i = 0; i < imageSequence.size(); i++) {
		imageSequence[i].allocate(cameraWidth,cameraHeight, OF_IMAGE_COLOR);
	}
    currentFrameImg.clear();
    currentFrameImg.allocate(cameraWidth,cameraHeight, OF_IMAGE_COLOR);
    
}

//--------------------------------------------------------------
void testApp::loadAndPlayRecording(string folder){
    
    leapVisualizer.loadXmlFile("recordings/"+folder+"/leap/leap.xml");
    video.load("recordings/"+folder+"/camera");
    
    indexRecorder.setup("recordings/"+folder+"/calib","fingerCalibPts.xml");
    indexRecorder.setDrawOffsets(drawW,0,cameraWidth/drawW,cameraHeight/drawH);
    
    // open calibration if exists?
    
    bPlaying = true;
    playing = true;
    currentFrameNumber = 0;
    
}

//--------------------------------------------------------------
void testApp::calibrateFromXML( string calibFolderName ){
    
    leapCameraCalibrator.setup(cameraWidth, cameraHeight);
    leapCameraCalibrator.loadFingerTipPoints("recordings/"+calibFolderName+"/calib/fingerCalibPts.xml");
    
    if(useCorrectedCam()){
        leapCameraCalibrator.correctCameraPNP(myCalibration);
    }else{
        leapCameraCalibrator.correctCamera();

    }
}

bool testApp::useCorrectedCam(){

#ifdef _USE_CORRECTED_CAMERA
    if(bUseCorrectedCamera) return true;
    else return false;
#else
    return false;
#endif

}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

	if (key == 'c'){
		// Reset the camera if the user presses 'c'.
		cam.reset();
	} else if ((key == 'r') || (key == 'R')){
		if (leap.isConnected()){
			// Toggle Recording.
			//reset so we don't store extra tags
			if(bRecording){
                bEndRecording = true;
            }else{
                bRecording = !bRecording;
                folderName = ofGetTimestampString();
				leapRecorder.startRecording();
                leapVisualizer.XML.clear();
                bPlaying = false;
                currentFrameNumber = 0;
                if( key == 'R') bRecordingForCalibration = true;
                else bRecordingForCalibration = false;
			}

		}
	
	}
    
    switch(key){
        case OF_KEY_LEFT:
            video.goToPrevious();
            break;
        case OF_KEY_RIGHT:
            video.goToNext();
            break;
        case 'C':
            calibrateFromXML(folderName);
            break;
        case 'F':
            bUseFbo = !bUseFbo;
            break;
        case 'g':
            leapVisualizer.bDrawGrid = !leapVisualizer.bDrawGrid;
            break;
        case 'l':
            if(bPlaying) bPlaying = false;
            break;
        case 'm':
            bInputMousePoints = !bInputMousePoints;
            break;
        case 'p':
            if(bPlaying) playing = !playing;
            break;
        case 's':
            leapVisualizer.bDrawSimple = !leapVisualizer.bDrawSimple;
            break;
        case 'v':
            bUseVirtualProjector = !bUseVirtualProjector;
            break;
        case '1':
            loadPlaybackFromDialog();
            break;
        case '2':
            loadCalibrationFromDialog();
            bUseVirtualProjector = true;
            break;
        case '3':
            loadPlaybackFromDialogForCalibration();
            break;
        case 'w':
            bShowCalibPoints = !bShowCalibPoints;
            break;
        case ' ':
            if(bRecordingForCalibration) bRecordThisCalibFrame = true;
            break;
        case 'u':
            bUseCorrectedCamera = !bUseCorrectedCamera;
            break;
        case '0':
            bShowLargeCamImageOnTop = !bShowLargeCamImageOnTop;
            break;
        case 'f':
            ofToggleFullscreen();
            break;
        case '9':
            if(drawW == 640){
                drawW = 1024;
                drawH = 768;
                bShowText = false;
                
            }else{
                drawW = 640;
                drawH = 480;
                bShowText = true;
            }
            break;
            
    }
    
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
    
    
    
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
    
    if(bPlaying && bInputMousePoints){
        if(x > indexRecorder.xOffset && y > indexRecorder.yOffset &&
           x < indexRecorder.xOffset+drawW && y < drawH){
            indexRecorder.recordPosition(x, y-20, leapVisualizer.getIndexFingertipFromXML(video.getCurrentFrameID()),video.getCurrentFrameID());
            lastIndexVideoPos.set(x,y-20);
            lastIndexLeapPos = leapVisualizer.getIndexFingertipFromXML(video.getCurrentFrameID());
        }
    }
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){
}

//--------------------------------------------------------------
void testApp::exit(){
    // let's close down Leap and kill the controller
    leap.close();
}





/*
 
 // WORKING, FROM KYLE'S RECORDER
 
 void testApp::update() {
 // grabVideo() will place the most recent frame into curFrame. If it's a new
 // frame, grabFrame returns true. If there are multiple frames available, it
 // will drop old frames and only give you the newest. This guarantees the
 // lowest latency. If you prefer to not drop frames, set the second argument
 // (dropFrames) to false. By default, capture is non-blocking.
 if(camera.grabVideo(curFrame)) {
 curFrame.update();
 
 
 if(recording) {
 if(currentFrame < imageSequence.size()) {
 ofPixels& target = imageSequence[currentFrame];
 memcpy(target.getPixels(), curFrame.getPixels(), target.getWidth() * target.getHeight() * target.getNumChannels());
 currentFrame++;
 } else {
 recording = false;
 for(int i = 0; i < imageSequence.size(); i++) {
 imageSequence[i].rotate90(1);
 ofSaveImage(imageSequence[i], ofToString(i, 3, '0') + ".jpg");
 }
 }
 }
 }
 }
 
 void testApp::draw() {
 // If the camera isn't ready, the curFrame will be empty.
 if(camera.isReady()) {
 // Camera doesn't draw itself, curFrame does.
 curFrame.draw(0, 0);
 ofDrawBitmapString(ofToString((int) ofGetFrameRate()) + " fps " + (recording ? "recording" : "") , 10, 20);
 }
 }
 
 void testApp::keyPressed(int key) {
 if(key == 'r') {
 recording = true;
 currentFrame = 0;
 }
 if(key == 'i') {
 printf("Attempting save\n");
 curFrame.saveImage(ofToString(ofGetFrameNum()) + ".png");
 }
 }
 */




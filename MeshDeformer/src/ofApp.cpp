#include "ofApp.h"
#include "SharedUtils.h"

void updatePuppet(Skeleton* skeleton, ofxPuppet& puppet) {
	for(int i = 0; i < skeleton->size(); i++) {
		puppet.setControlPoint(skeleton->getControlIndex(i), skeleton->getPositionAbsolute(i));
	}
}

void ofApp::setup() {
	
    
	//----------------------------
	// Create all of the scenes
	scenes.push_back(new NoneScene(&puppet, &handSkeleton, &immutableHandSkeleton));
	scenes.push_back(new WaveScene(&puppet, &handSkeleton, &immutableHandSkeleton));
    scenes.push_back(new WiggleScene(&puppet, &handSkeleton, &immutableHandSkeleton));
	scenes.push_back(new WobbleScene(&puppet, &handSkeleton, &immutableHandSkeleton));
	scenes.push_back(new EqualizeScene(&puppet, &handSkeleton, &immutableHandSkeleton));
	scenes.push_back(new NorthScene(&puppet, &handSkeleton, &immutableHandSkeleton));
	scenes.push_back(new LissajousScene(&puppet, &threePointSkeleton, &immutableThreePointSkeleton));
	scenes.push_back(new MeanderScene(&puppet, &handSkeleton, &immutableHandSkeleton));
	scenes.push_back(new PropogatingWiggleScene(&puppet, &handWithFingertipsSkeleton, &immutableHandWithFingertipsSkeleton));
	scenes.push_back(new SinusoidalLengthScene(&puppet, &handSkeleton, &immutableHandSkeleton));
	scenes.push_back(new PulsatingPalmScene(&puppet, &palmSkeleton, &immutablePalmSkeleton));
	scenes.push_back(new RetractingFingersScene(&puppet, &handWithFingertipsSkeleton, &immutableHandWithFingertipsSkeleton));
	scenes.push_back(new SinusoidalWiggleScene(&puppet, &handWithFingertipsSkeleton, &immutableHandWithFingertipsSkeleton));
	scenes.push_back(new MiddleDifferentLengthScene(&puppet, &handSkeleton, &immutableHandSkeleton));
	scenes.push_back(new GrowingMiddleFingerScene(&puppet, &handWithFingertipsSkeleton, &immutableHandWithFingertipsSkeleton));
	scenes.push_back(new StartrekScene(&puppet, &handSkeleton, &immutableHandSkeleton));
	scenes.push_back(new StraightenFingersScene(&puppet, &handWithFingertipsSkeleton, &immutableHandWithFingertipsSkeleton));
	scenes.push_back(new SplayFingersScene(&puppet, &handWithFingertipsSkeleton, &immutableHandWithFingertipsSkeleton));
	scenes.push_back(new TwitchScene(&puppet, &handSkeleton, &immutableHandSkeleton));
	scenes.push_back(new PinkyPuppeteerScene(&puppet, &handWithFingertipsSkeleton, &immutableHandWithFingertipsSkeleton));
	scenes.push_back(new FingerLengthPuppeteerScene(&puppet, &handWithFingertipsSkeleton, &immutableHandWithFingertipsSkeleton));
    
	sharedSetup();
    
    // don't use SharedData for now, use 2013 folder
    string basePath = ofToDataPath("", true);
    ofSetDataPathRoot("../../../../../SharedData2013/");
    
	setupGui();
    
	// set up the mesh
	//hand.loadImage("hand/genericHandCentered.jpg");
	//mesh.load("hand/handmarks.ply");
	
	
	// This is the NEW mesh, of Kyle's hand, produced by the MeshGenerator
	hand.loadImage("hand/genericHandCenteredNew.jpg");
	mesh.load("hand/handmarksNew.ply");
    
    // are we sure we have the mesh:
    cout << "num vertices in mesh " << mesh.getNumVertices() << endl;
    
	/*
     // This is golan's hand
     hand.loadImage("hand/golanHand.jpg");
     mesh.load("hand/golanHand.ply");
	 */
	
	for (int i = 0; i < mesh.getNumVertices(); i++) {
		mesh.addTexCoord(mesh.getVertex(i));
	}
	
	//----------------------------
	// Set up the puppet.
	puppet.setup (mesh);
    
	//----------------------------
	// Set up all of the skeletons.
	previousSkeleton = NULL;
	currentSkeleton  = NULL;
	
	handWithFingertipsSkeleton.setup(mesh);
	immutableHandWithFingertipsSkeleton.setup(mesh);
	handSkeleton.setup(mesh);
	immutableHandSkeleton.setup(mesh);
	threePointSkeleton.setup(mesh);
	immutableThreePointSkeleton.setup(mesh);
	palmSkeleton.setup(mesh);
	immutablePalmSkeleton.setup(mesh);
	wristSpineSkeleton.setup(mesh);
	immutableWristSpineSkeleton.setup(mesh);
    
	//----------------------------
	// Initialize gui features
	mouseControl  = false;
	showImage     = true;
	showWireframe = true;
	showSkeleton  = true;
	frameBasedAnimation = false;
	showGuis = true;
    
	// set the initial skeleton
	setSkeleton(&handSkeleton);
}

void ofApp::setupGui() {
	// set up the guis for each scene
	for (int i=0; i < scenes.size(); i++) {
		sceneNames.push_back(scenes[i]->getName());
		sceneWithSkeletonNames.push_back(scenes[i]->getNameWithSkeleton());
		scenes[i]->setupGui();
		scenes[i]->setupMouseGui();
	}
    
	// create the main gui
	gui = new ofxUICanvas();
	gui->setFont("GUI/NewMedia Fett.ttf");
	gui->addLabel("Mesh Deformer");
	gui->addSpacer();
	gui->addFPS();
	gui->addSpacer();
	sceneRadio = gui->addRadio("Scene", sceneNames);
	//sceneRadio = gui->addRadio("Scene", sceneWithSkeletonNames);
	gui->addSpacer();
	gui->addLabelToggle("Show Image", &showImage);
	gui->addLabelToggle("Show Wireframe", &showWireframe);
	gui->addLabelToggle("Show Skeleton",  &showSkeleton);
	gui->addLabelToggle("Mouse Control",  &mouseControl);
	gui->addLabelToggle("FrameBasedAnim", &frameBasedAnimation);
	gui->addSpacer();
	gui->autoSizeToFitWidgets();
	
	// set the initial scene
    sceneRadio->getToggles()[0/*scenes.size()-1*/]->setValue(true);
}

int getSelection(ofxUIRadio* radio) {
	vector<ofxUIToggle*> toggles = radio->getToggles();
	for(int i = 0; i < toggles.size(); i++) {
		if(toggles[i]->getValue()) {
			return i;
		}
	}
	return -1;
}

void ofApp::update() {
	
    
    
    // every frame we get a new mesh from the hand tracker
	handWithFingertipsSkeleton.setup(mesh);
	immutableHandWithFingertipsSkeleton.setup(mesh);
	handSkeleton.setup(mesh);
	immutableHandSkeleton.setup(mesh);
	threePointSkeleton.setup(mesh);
	immutableThreePointSkeleton.setup(mesh);
	palmSkeleton.setup(mesh);
	immutablePalmSkeleton.setup(mesh);
	wristSpineSkeleton.setup(mesh);
	immutableWristSpineSkeleton.setup(mesh);
    
	// get the current scene
	int scene = getSelection(sceneRadio);
    
	// turn off all other scenes
	for (int i=0; i < scenes.size(); i++) {
		if (i != scene) scenes[i]->turnOff();
	}
	
	if (mouseControl) {
		// turn on mouse gui for current scene
		if (!scenes[scene]->mouseGuiIsOn()) scenes[scene]->turnOnMouse();
		// update the mouse
		scenes[scene]->updateMouse(mouseX, mouseY);
	}
	else {
		scenes[scene]->turnOffMouse();
	}
    
	// turn on the gui for the current scene
	if (!scenes[scene]->guiIsOn()) {
		scenes[scene]->turnOn();
		showImage = scenes[scene]->isStartShowImage();
		showWireframe = scenes[scene]->isStartShowWireframe();
		showSkeleton = scenes[scene]->isStartShowSkeleton();
		mouseControl = scenes[scene]->isStartMouseControl();
	}
    
	if (!showGuis) {
		this->gui->setVisible(false);
		for(int i=0; i < scenes.size(); i++) {
			scenes[i]->turnOffGui();
			scenes[i]->turnOffMouse();
		}
	}
	else {
		this->gui->setVisible(true);
	}
    
	// update skeleton
	scenes[scene]->update();
	setSkeleton(scenes[scene]->getSkeleton());
    
	// update the puppet using the current scene's skeleton
	updatePuppet(currentSkeleton, puppet);
	puppet.update();
}

void ofApp::draw() {
	
    
    ofBackground(0);
    
    ofSetColor(255);
    
	if (showImage) {
		hand.bind();
		puppet.drawFaces();
		hand.unbind();
	}
	if(showWireframe) {
		puppet.drawWireframe();
		puppet.drawControlPoints();
	}
	if(showSkeleton) {
		currentSkeleton->draw();
	}
    
	int scene = getSelection(sceneRadio);
	scenes[scene]->draw();
}

void ofApp::keyPressed(int key) {
	if (key == 'F'){
		ofToggleFullscreen();
	}
	if (key == 'G'){
		showGuis = !showGuis;
		for (int i=0; i < scenes.size(); i++) {
			scenes[i]->setShowGuis(showGuis);
		}
	}
}

void ofApp::setSkeleton(Skeleton* skeleton) {
	if(skeleton != currentSkeleton) {
		previousSkeleton = currentSkeleton;
		currentSkeleton = skeleton;
		if(previousSkeleton != NULL) {
			vector<int>& previousControlIndices = previousSkeleton->getControlIndices();
			for(int i = 0; i < previousControlIndices.size(); i++) {
				puppet.removeControlPoint(previousControlIndices[i]);
			}
		}
		vector<int>& currentControlIndices = currentSkeleton->getControlIndices();
		for(int i = 0; i < currentControlIndices.size(); i++) {
			puppet.setControlPoint(currentControlIndices[i]);
		}
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

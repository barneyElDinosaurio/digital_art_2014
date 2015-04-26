#include "FractalHand.h"

const IndexSet pinkyIndices  = IndexSet() /IndexRange(21,41);
const IndexSet ringIndices   = IndexSet() /IndexRange(42,62);
const IndexSet middleIndices = IndexSet() /IndexRange(63,83);
const IndexSet indexIndices  = IndexSet() /IndexRange(84,104);
const IndexSet thumbIndices  = IndexSet() /IndexRange(0,20);

const IndexSet allFingerIndices = IndexSet()
/IndexRange(21,41)
/IndexRange(42,62)
/IndexRange(63,83)
/IndexRange(84,104)
/IndexRange(0,20);

const IndexSet miniHandIndices = IndexSet()
/IndexRange(0,104)
/IndexRange(108,150);

const IndexSet fingertipIndices = IndexSet()
/IndexRange(9,20);

const IndexSet wristIndices = IndexSet()
/IndexRange(105,113);

string FractalHand::getName() const {
    return "FractalHand";
}

void FractalHand::initialize(){
    
    FractalHand::initializeGui();
    this->gui->addIntSlider("sourceBlendOpIndex", 0, 8, &sourceBlendOpIndex);
    this->gui->addIntSlider("destBlendOpIndex", 0, 7, &destBlendOpIndex);
    this->gui->addToggle("wireframe", &wireframe);
    this->gui->addToggle("squish", &squish);
    this->gui->autoSizeToFitWidgets();
    
    final.load("models/mesh_FractalHand_final.ply");
    for (int i = 0; i < final.getNumVertices(); i++) {
        // This is important
        ofVec2f aMeshVertex;
        aMeshVertex.x =  final.getVertex(i).y;
        aMeshVertex.y =  768 - final.getVertex(i).x;
        final.addTexCoord( aMeshVertex );
    }
    
    fingerMeshes.resize(5);
    fingertipMeshes.resize(5);
    miniHandMeshes.resize(5);
    wristMeshes.resize(5);
    
    // Cache the topology subdivision.
    butterflySubdivider.topology_start(final);
    butterflySubdivider.topology_subdivide_boundary(2);
    refinedMesh = butterflySubdivider.topology_end();
    
    bUseButterfly = true;
    
    squish = true;
    
    // if window ever resizes, this might be bad... be careful
    // might need to do width&height some other way
    miniHandsFBO.allocate(1440,900);
    
    // blurShader.load("shaders/blur.vert", "shaders/blur.frag");
    
}

void FractalHand::update(const ofMesh& mesh) {
    
    ofMesh handMesh = mesh;
    
    // make copies of the fingers, so we can stitch little hands onto them later
    fingerMeshes[0] = copySubmesh(handMesh, pinkyIndices);
    fingerMeshes[1] = copySubmesh(handMesh, ringIndices);
    fingerMeshes[2] = copySubmesh(handMesh, middleIndices);
    fingerMeshes[3] = copySubmesh(handMesh, indexIndices);
    fingerMeshes[4] = copySubmesh(handMesh, thumbIndices);
    
    float widthRatioAverage = 0;
    
    for(int i = 0; i < 5; i++) {
        
        bool isThumb = i==4;
        
        // make a copy of the hand to attach to finger
        
        miniHandMeshes[i] = copySubmesh(handMesh, miniHandIndices);
        
        // get width of finger
        
        int fingertipBottomEdgeIndex = 9;//12;
        int fingertipTopEdgeIndex    = 11;//14;
        
        ofVec3f fingertipBottomEdgeVert = fingerMeshes[i].getVertex(fingertipBottomEdgeIndex);
        ofVec3f fingertipTopEdgeVert    = fingerMeshes[i].getVertex(fingertipTopEdgeIndex);
        float fingertipWidth = fingertipBottomEdgeVert.distance(fingertipTopEdgeVert);
        
        // get width of wrist
        
        int wristBottomEdgeIndex = 111;
        int wristTopEdgeIndex    = 113;
        
        ofVec3f wristBottomEdgeVert = miniHandMeshes[i].getVertex(wristBottomEdgeIndex);
        ofVec3f wristTopEdgeVert    = miniHandMeshes[i].getVertex(wristTopEdgeIndex);
        
        float wristWidth = wristBottomEdgeVert.distance(wristTopEdgeVert);
        
        // use ratio of finger width to wrist width to scale hand
        // (or if it's the thumb, use the average finger width)
        
        float widthRatio;
        
        if(!isThumb) {
            widthRatio = fingertipWidth / wristWidth;
            widthRatioAverage += widthRatio;
        } else {
            widthRatio = widthRatioAverage/4;
        }
        
        for(int v = 0; v < miniHandMeshes[i].getVertices().size(); v++) {
            ofVec3f newPosition = miniHandMeshes[i].getVertex(v);
            newPosition = newPosition * widthRatio;
            miniHandMeshes[i].setVertex(v, newPosition);
        }
        
        fingertipMeshes[i] = copySubmesh(fingerMeshes[i], fingertipIndices);
        
        fingerMeshes[i] = removeSubmesh(fingerMeshes[i], fingertipIndices);
        
        float fingerDir = 0;
        for(int v = 0; v < 4; v++) {
            ofVec3f fingerDirBase = fingerMeshes[i].getVertex(v*3+1);
            ofVec3f fingerDirTop = fingerMeshes[i].getVertex(v*3+4);
            fingerDir += atan((fingerDirBase.y-fingerDirTop.y) / (fingerDirBase.x-fingerDirTop.x));
        }
        fingerDir /= 4;
        
        for(int v = 0; v < miniHandMeshes[i].getNumVertices(); v++) {
            ofVec3f cur = miniHandMeshes[i].getVertex(v);
            cur.rotate(fingerDir*RAD_TO_DEG, ofVec3f(0,0,1));
            miniHandMeshes[i].setVertex(v, cur);
        }
        
        // move minihand on top of fingertip (new method w averages)
        
        ofVec3f fingerAttachPosition = fingerMeshes[i].getVertex(10);
        ofVec3f minihandAttachPosition = miniHandMeshes[i].getVertex(112);
        
        for(int v = 0; v < miniHandMeshes[i].getNumVertices(); v++) {
            ofVec3f cur = miniHandMeshes[i].getVertex(v);
            cur -= minihandAttachPosition;
            cur += fingerAttachPosition;
            miniHandMeshes[i].setVertex(v, cur);
        }
        
        // 'squish' minihand onto finger
        
        ofVec3f origWristPosTop = miniHandMeshes[i].getVertex(111);
        ofVec3f origWristPosBottom = miniHandMeshes[i].getVertex(113);
        
        miniHandMeshes[i].setVertex(111, fingerMeshes[i].getVertex(9));
        miniHandMeshes[i].setVertex(113, fingerMeshes[i].getVertex(11));
        
        float squishRadius = fingertipWidth;
        
        if(squish){
            for(int v = 0; v < miniHandMeshes[i].getNumVertices(); v++) {
                if(v != 111 && v != 113) {
                    //ofVec3f d = miniHandMeshes[i].getVertex(105)-miniHandMeshes[i].getVertex(v);
                    ofVec3f d = miniHandMeshes[i].getVertex(111)-origWristPosTop;
                    
                    float h = (miniHandMeshes[i].getVertex(v)-origWristPosTop).length();
                    
                    float squishFactor = (1.0-(h/squishRadius));
                    if(squishFactor < 0.0) squishFactor = 0.0;
                    
                    ofVec3f cur = miniHandMeshes[i].getVertex(v);
                    cur += d * squishFactor;
                    miniHandMeshes[i].setVertex(v, cur);
                }
            }
            
            for(int v = 0; v < miniHandMeshes[i].getNumVertices(); v++) {
                if(v != 111 && v != 113) {
                    //ofVec3f d = miniHandMeshes[i].getVertex(107)-miniHandMeshes[i].getVertex(v);
                    ofVec3f d = miniHandMeshes[i].getVertex(113)-origWristPosBottom;
                    
                    float h = (miniHandMeshes[i].getVertex(v)-origWristPosBottom).length();
                    
                    float squishFactor = (1.0-(h/squishRadius));
                    if(squishFactor < 0.0) squishFactor = 0.0;
                    
                    ofVec3f cur = miniHandMeshes[i].getVertex(v);
                    cur += d * squishFactor;
                    miniHandMeshes[i].setVertex(v, cur);
                }
            }
        }
        
        // attach wrist to finger
        
        miniHandMeshes[i].setVertex(105, fingerMeshes[i].getVertex(6));
        miniHandMeshes[i].setVertex(106, fingerMeshes[i].getVertex(7));
        miniHandMeshes[i].setVertex(107, fingerMeshes[i].getVertex(8));
        
        miniHandMeshes[i].setVertex(108, (fingerMeshes[i].getVertex(9) +fingerMeshes[i].getVertex(6))*0.5);
        miniHandMeshes[i].setVertex(109, (fingerMeshes[i].getVertex(10)+fingerMeshes[i].getVertex(7))*0.5);
        miniHandMeshes[i].setVertex(110, (fingerMeshes[i].getVertex(11)+fingerMeshes[i].getVertex(8))*0.5);
        
        // remove wrist to add blend colors to finger
        
        wristMeshes[i] = copySubmesh(miniHandMeshes[i], wristIndices);
        miniHandMeshes[i] = removeSubmesh(miniHandMeshes[i], wristIndices);
        
        vector<ofFloatColor> colors(9);
        colors[0] = ofFloatColor(1,0);
        colors[1] = ofFloatColor(1,0);
        colors[2] = ofFloatColor(1,0);
        colors[3] = ofFloatColor(1,0.5);
        colors[4] = ofFloatColor(1,0.5);
        colors[5] = ofFloatColor(1,0.5);
        colors[6] = ofFloatColor(1,1);
        colors[7] = ofFloatColor(1,1);
        colors[8] = ofFloatColor(1,1);
        wristMeshes[i].addColors(colors);
        
    }
    
    // get rid of the fingers on the main hand mesh
    handMesh = removeSubmesh(handMesh, allFingerIndices);
    
    final = handMesh;
    
    if(saveFinalMesh) final.save("models/mesh_FractalHand_final.ply");
    
}

ofMesh& FractalHand::getModifiedMesh() {
    return final;
}

void FractalHand::drawBlends() {
    
}

void FractalHand::draw (const ofTexture& texture) {
    
    //butterflySubdivider.fixMesh (final, refinedMesh);
    //refinedMesh.drawFaces();
    
    if(!wireframe) {
        texture.bind();
        ofSetColor(255, 255, 255, 255);
        
        for(int i = 0; i < 5; i++) {
            fingerMeshes[i].drawFaces();
            //fingertipMeshes[i].drawFaces();
        }
        for(int i = 0; i < 5; i++) {
            miniHandMeshes[i].drawFaces();
            wristMeshes[i].drawFaces();
        }
        
    } else {
        texture.unbind();
        
        for(int i = 0; i < 5; i++) {
            ofSetColor(255, 0, 0, 255);
            fingerMeshes[i].drawWireframe();
            fingertipMeshes[i].drawWireframe();
            ofSetColor(0, 255, 0, 255);
            miniHandMeshes[i].drawWireframe();
            wristMeshes[i].drawWireframe();
        }
        
    }
    
    final.drawFaces();
    
    texture.unbind();
    
}

void FractalHand::saveMeshes(){
    
}
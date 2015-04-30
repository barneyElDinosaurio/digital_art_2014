#include "Stubby.h"

const IndexSet pinkyIndices  = IndexSet() /IndexRange(21,41);
const IndexSet ringIndices   = IndexSet() /IndexRange(42,62);
const IndexSet middleIndices = IndexSet() /IndexRange(63,83);
const IndexSet indexIndices  = IndexSet() /IndexRange(84,104);

const IndexSet allFingerIndices = IndexSet()
/IndexRange(21,41)
/IndexRange(42,62)
/IndexRange(63,83)
/IndexRange(84,104);

const IndexSet blendRemoval = IndexSet()
/IndexRange(3,8);

const IndexSet knuckeIndices = IndexSet() /IndexRange(3,11);

string Stubby::getName() const {
    return "Stubby";
}

void Stubby::update(const ofMesh& mesh) {
    ofMesh handMesh = mesh;
    
    // create individual meshes for each finger
    pinkyMesh  = copySubmesh(handMesh, pinkyIndices);
    ringMesh   = copySubmesh(handMesh, ringIndices);
    middleMesh = copySubmesh(handMesh, middleIndices);
    indexMesh  = copySubmesh(handMesh, indexIndices);
    
    // take fingers off of hand mesh
    handMesh = removeSubmesh(handMesh, allFingerIndices);
    
    // so now we have 4 finger meshes and a palm mesh.
    
    // save the piece of finger we're going to remove to use as a blend
    pinkyBlendMesh  = copySubmesh(pinkyMesh,  knuckeIndices);
    ringBlendMesh   = copySubmesh(ringMesh,   knuckeIndices);
    middleBlendMesh = copySubmesh(middleMesh, knuckeIndices);
    indexBlendMesh  = copySubmesh(indexMesh,  knuckeIndices);
    // we only need the first half of it though
    pinkyBlendMesh  = removeSubmesh(pinkyBlendMesh, blendRemoval);
    ringBlendMesh   = removeSubmesh(ringBlendMesh, blendRemoval);
    middleBlendMesh = removeSubmesh(middleBlendMesh, blendRemoval);
    indexBlendMesh  = removeSubmesh(indexBlendMesh, blendRemoval);
    
    // set the edge of the blends to be transparent for a smooth transition
    vector<ofFloatColor> colors(6);
    colors[0] = ofFloatColor(1,1);
    colors[1] = ofFloatColor(1,1);
    colors[2] = ofFloatColor(1,1);
    colors[3] = ofFloatColor(1,0);
    colors[4] = ofFloatColor(1,0);
    colors[5] = ofFloatColor(1,0);
    pinkyBlendMesh.addColors(colors);
    ringBlendMesh.addColors(colors);
    middleBlendMesh.addColors(colors);
    indexBlendMesh.addColors(colors);
    
    // remove the piece of the finger
    pinkyMesh  = removeSubmesh(pinkyMesh,  knuckeIndices);
    ringMesh   = removeSubmesh(ringMesh,   knuckeIndices);
    middleMesh = removeSubmesh(middleMesh, knuckeIndices);
    indexMesh  = removeSubmesh(indexMesh,  knuckeIndices);
    
    // attach the tip of the finger to the base, filling the gap
    // we created when we removed the middle piece of the finger
    ofVec3f pinkyDistanceAway  = pinkyMesh.getVertex(7)  - pinkyMesh.getVertex(4);
    ofVec3f ringDistanceAway   = ringMesh.getVertex(7)   - ringMesh.getVertex(4);
    ofVec3f middleDistanceAway = middleMesh.getVertex(7) - middleMesh.getVertex(4);
    ofVec3f indexDistanceAway  = indexMesh.getVertex(7)  - indexMesh.getVertex(4);
    
    for(int i = 6; i <= 17; i++) {
        pinkyMesh.setVertex (i, pinkyMesh.getVertex (i)-pinkyDistanceAway);
        ringMesh.setVertex  (i, ringMesh.getVertex  (i)-ringDistanceAway);
        middleMesh.setVertex(i, middleMesh.getVertex(i)-middleDistanceAway);
        indexMesh.setVertex (i, indexMesh.getVertex (i)-indexDistanceAway);
    }
    
    pinkyMesh.setVertex(8, pinkyMesh.getVertex(5));
    pinkyMesh.setVertex(6, pinkyMesh.getVertex(3));
    
    ringMesh.setVertex(8, ringMesh.getVertex(5));
    ringMesh.setVertex(6, ringMesh.getVertex(3));
    
    middleMesh.setVertex(8, middleMesh.getVertex(5));
    middleMesh.setVertex(6, middleMesh.getVertex(3));
    
    indexMesh.setVertex(8, indexMesh.getVertex(5));
    indexMesh.setVertex(6, indexMesh.getVertex(3));
    
    // align the blend meshes to the fingers
    pinkyBlendMesh.setVertex(3, pinkyMesh.getVertex(9));
    pinkyBlendMesh.setVertex(4, pinkyMesh.getVertex(10));
    pinkyBlendMesh.setVertex(5, pinkyMesh.getVertex(11));
    
    ringBlendMesh.setVertex(3, ringMesh.getVertex(9));
    ringBlendMesh.setVertex(4, ringMesh.getVertex(10));
    ringBlendMesh.setVertex(5, ringMesh.getVertex(11));
    
    middleBlendMesh.setVertex(3, middleMesh.getVertex(9));
    middleBlendMesh.setVertex(4, middleMesh.getVertex(10));
    middleBlendMesh.setVertex(5, middleMesh.getVertex(11));
    
    indexBlendMesh.setVertex(3, indexMesh.getVertex(9));
    indexBlendMesh.setVertex(4, indexMesh.getVertex(10));
    indexBlendMesh.setVertex(5, indexMesh.getVertex(11));
    
    // create a single mesh that contains everything, we need this for the subdivided mesh
    final = handMesh;
    final.append(pinkyMesh);
    final.append(ringMesh);
    final.append(middleMesh);
    final.append(indexMesh);
    
    if(saveFinalMesh) final.save("models/mesh_Stubby_final.ply");
}

ofMesh& Stubby::getModifiedMesh() {
    return final;
}

void Stubby::drawBlends() {
	pinkyBlendMesh.drawFaces();
    ringBlendMesh.drawFaces();
    middleBlendMesh.drawFaces();
    indexBlendMesh.drawFaces();
}

//-------------------------------------------
void Stubby::draw (const ofTexture& texture) {
	
	texture.bind();
	if (bUseButterfly){
		
		butterflySubdivider.fixMesh (final, refinedMesh);
		refinedMesh.drawFaces();
		
        final.drawFaces();
        
        pinkyMesh.drawFaces();
        ringMesh.drawFaces();
        middleMesh.drawFaces();
        indexMesh.drawFaces();

		drawBlends();
		
	} else {
		final.drawFaces();     // was drawBase();
		//blendMesh.drawFaces(); // was drawBlends();
	}
	texture.unbind();
}

//-------------------------------------------
void Stubby::saveMeshes(){
    ofMesh entireStubbyHand = final;
    entireStubbyHand.append(pinkyMesh);
    entireStubbyHand.append(ringMesh);
    entireStubbyHand.append(middleMesh);
    entireStubbyHand.append(indexMesh);
    
	entireStubbyHand.save("models/mesh_Stubby_final.ply");
    
    // these were used for debugging...
    
	//blendMesh.save    ("mesh_Stubby_blendMesh.ply");
    
    pinkyMesh.save("models/mesh_Stubby_pinky.ply");
    
    //pinkyBlendMesh.save("mesh_Stubby_pinkyBlend.ply");
    
    //ringMesh.save("mesh_Stubby_ring.ply");
    //middleMesh.save("mesh_Stubby_middle.ply");
    //indexMesh.save("mesh_Stubby_index.ply");
}

//-------------------------------------------
void Stubby::initialize(){
    Stubby::initializeGui();
    this->gui->autoSizeToFitWidgets();
    
	final.load("models/mesh_Stubby_final.ply");
	for (int i = 0; i < final.getNumVertices(); i++) {
        // This is important
		ofVec2f aMeshVertex;
		aMeshVertex.x =  final.getVertex(i).y;
		aMeshVertex.y =  768 - final.getVertex(i).x;
		final.addTexCoord( aMeshVertex );
	}
    
	// Cache the topology subdivision.
    butterflySubdivider.topology_start(final);
    butterflySubdivider.topology_subdivide_boundary(2);
    refinedMesh = butterflySubdivider.topology_end();
	 
	bUseButterfly = true;
}

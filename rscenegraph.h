#ifndef RSCENEGRAPH_H
#define RSCENEGRAPH_H

#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>
#include <rcamera.h>



// BoundingBoxCornersFlag 
// NOTE: bitwise flags to select corners
typedef enum 
{
	BOX_NO_CORNER          = 0 ,

	BOX_FRONT_BOTTOM_LEFT  = 1 ,
	BOX_FRONT_BOTTOM_RIGHT = 2 ,
	BOX_FRONT_TOP_LEFT     = 4 ,
	BOX_FRONT_TOP_RIGHT    = 8 ,

	BOX_BACK_BOTTOM_LEFT  = 16 ,
	BOX_BACK_BOTTOM_RIGHT = 32 ,
	BOX_BACK_TOP_LEFT     = 64 ,
	BOX_BACK_TOP_RIGHT    = 128,

	BOX_ALL_CORNERS       = 255

} BoundingBoxCornersFlag;


typedef struct Frustum 
{
	Camera *camera ;

	float aspect ;

	Matrix proj ;
	Matrix view ;

	// Frustum planes :

	union 
	{
		Vector4 plane[6] ;
		struct 
		{
			Vector4 up ;
			Vector4 down ; 
			Vector4 left ;
			Vector4 right ;
			Vector4 near ;
			Vector4 far ;
		};
	};

} Frustum;


typedef enum
{
	NODE_ANIMATION_EVENT_LOOP = 1 ,
	NODE_ANIMATION_EVENT_COMPLETE = 2 ,

} NodeAnimationEvent;


typedef struct Node3D Node3D;
typedef struct Node3D Node;

typedef void (*NodeAnimationEventCallback)( Node *node , NodeAnimationEvent event );



typedef struct Node3D 
{
	Model *model ;

	Color tint ;

	// Node transforms :
	Vector3    position ;
	Vector3    scale    ;
	Matrix     rotation ; // Contains the rotation matrix only // TODO use Quaternion instead ?

	int        positionRelativeToParentBoneId ;   // TODO : explain
	char      *positionRelativeToParentBoneName ;

	Matrix transform ; // Computed using position, scale and rotation and parent's transform /!\

	// Untransformed boundings are in model's space
	// and are computed once in LoadNodeFromModel()
	BoundingBox untransformedBox ;
	Vector3 untransformedCenter ;
	float untransformedRadius ;

	// Transformed boundings are in World's space
	// and are updated when required.
	BoundingBox transformedBox ;
	Vector3 transformedCenter ;
	float transformedRadius ;

	// Basic scenegraph bindings ...

	Node3D *parent;
	Node3D *firstChild;
	Node3D *nextSibling;
	Node3D *prevSibling;

	// Frustum visibility ...

	Frustum * lastFrustum ; // Points the last frustum relative to which the node was drawn
	bool insideFrustum ; // Tells if the node was visible in the frustum
	float distanceToCamera ; // Tells at which distance the node was from the camera

	Node3D *nextLOD ;
	float nextDistance ;
	Node3D *activeLOD ;

	// Animation stuff ...

	ModelAnimation *anims ;
	int animsCount ;           // How many anims in the anims list
	int animId ;               // Id of the currently selected anim

	float animPosition;      // Position in the animation timeline (will be converted to int for current frame)
	float animSpeed;         // Speed at which animPosition is updated by NodeUpdateAnimation(delta)
	int animRemainingLoops ; // How many time do we want to loop the currently selected animation ? (-1 inifinity)

	NodeAnimationEventCallback animEventCallback ; // If set, will be called on animation events

	// User data :
	void *userData ;

} Node3D;




#if defined(__cplusplus)
extern "C" {            // Prevents name mangling of functions
#endif

// AABB BoundingBox stuff :

RLAPI BoundingBox BoundingBoxTransform( BoundingBox box , Matrix transform );

// Matrix space travels :

RLAPI Matrix MatrixNormalize( Matrix m ); // Normalize the scales of the transform matrix
RLAPI Matrix MatrixRotation( Matrix m ); // Normalize the scales, and nullify the translation of the transform matrix


// Plane stuff :

RLAPI float PlaneDistanceToPoint( Vector4 plane , Vector3 point );

RLAPI bool CheckCollisionPlanePoint( Vector4 plane , Vector3 point );
RLAPI bool CheckCollisionPlaneSphere( Vector4 plane , Vector3 center , float radius );
RLAPI bool CheckCollisionPlaneBox( Vector4 plane , BoundingBox box );
RLAPI int  CheckCollisionPlaneBoxEx( Vector4 plane , BoundingBox box ); // Return a BoundingBoxCornersFlag bitfield

// Frustum stuff :

RLAPI Frustum GetCameraFrustum( Camera *camera , float aspect );
#define FrustumFromCamera GetCameraFrustum

RLAPI bool FrustumContainsPoint( Frustum *frustum , Vector3 point );
RLAPI bool FrustumContainsSphere( Frustum *frustum , Vector3 center , float radius );
RLAPI bool FrustumContainsBox( Frustum *frustum , BoundingBox box );


// Node's scenegraph :

RLAPI Node3D NodeAsGroup();
#define NodeAsRoot NodeAsGroup
RLAPI Node3D NodeAsModel( Model *model );

RLAPI void NodeAttachChild( Node *parent, Node *child );
RLAPI void NodeAttachChildToBone( Node *parent, Node *child , char *boneName );

RLAPI void NodeDetachBranch( Node *node ); // Detach the node and its children from the tree.
#define DetachNodeBranch NodeDetachBranch
RLAPI void NodeRemove( Node *node ); // Remove the node from its tree so that its children take its place.
#define RemoveNode NodeRemove

typedef void (*NodeTreeTraversalCallback)( Node *node , void *userData );

RLAPI void NodeTreeTraversal( Node *root , NodeTreeTraversalCallback callback , void *userData );
#define TraverseNodeTree NodeTreeTraversal

// Node's transforms :

RLAPI void NodeTreeUpdateTransforms( Node *root ); // Update the transform matrix of every nodes in the tree at once
#define UpdateNodeTreeTransforms NodeTreeUpdateTransforms

RLAPI void NodeUpdateTransforms( Node *node ); // Update the transform matrix of the node from its position, scale and rotation
#define UpdateNodeTransforms NodeUpdateTransforms

RLAPI void NodeUnpackTransforms( Node *node ); // Decompose the transform matrix into position, scale and rotation
#define UnpackNodeTransforms NodeUnpackTransforms


// NOTE : the transforms below are not immediately effective, till the transform matrix is updated.

RLAPI void NodeSetPosition( Node *node , Vector3 pos );
#define SetNodePosition NodeSetPosition

RLAPI void NodeRotate( Node *node , Vector3 axis , float angle );
#define RotateNode NodeRotate

RLAPI void NodeRotateX( Node *node , float angle ); // Rotate along world's X axis
RLAPI void NodeRotateY( Node *node , float angle ); // Rotate along world's Y axis
RLAPI void NodeRotateZ( Node *node , float angle ); // Rotate along world's Z axis

RLAPI void NodePitch( Node *node , float angle ); // Rotate along its own X axis
#define PitchNode NodePitch
#define RotateNodeAlongOwnX NodePitch
#define NodeRotateAlongOwnX NodePitch
RLAPI void NodeYaw( Node *node , float angle ); // Rotate along its own Y axis
#define YawNode NodeYaw
#define RotateNodeAlongOwnY NodeYaw
#define NodeRotateAlongOwnY NodeYaw
RLAPI void NodeRoll( Node *node , float angle ); // Rotate along its own Z axis
#define RollNode NodeRoll
#define RotateNodeAlongOwnZ NodeRoll
#define NodeRotateAlongOwnZ NodeRoll

RLAPI void NodeMoveForward( Node *node , float distance ); // Move in direction of its own Z axis
#define MoveNodeForward NodeMoveForward
#define NodeMoveTowardOwnZ NodeMoveForward
#define MoveNodeTowardOwnZ NodeMoveForward
RLAPI void NodeMoveUpward( Node *node , float distance ); // Move in direction of its own Y axis
#define MoveNodeUpward NodeMoveUpward
#define NodeMoveTowardOwnY NodeMoveUpward
#define MoveNodeTowardOwnY NodeMoveUpward
RLAPI void NodeMoveSideward( Node *node , float distance ); // Move in direction of its own X axis
#define MoveNodeSideward NodeMoveSideward
#define NodeStrafe NodeMoveSideward
#define NodeMoveTowardOwnX NodeMoveSideward
#define MoveNodeTowardOwnX NodeMoveSideward


// Node animations :

RLAPI void NodeUpdateAnimationTimeline( Node *node , float delta );
#define UpdateNodeAnimationTimeline NodeUpdateAnimationTimeline

// Node drawing :

RLAPI bool NodeDrawInFrustum( Node *node , Frustum *frustum ); // Draw the single node if visible inside the frustum and return true, else false
#define DrawNodeInFrustum NodeDrawInFrustum
RLAPI int NodeTreeDrawInFrustum( Node *root , Frustum *frustum ); // Draw the node's tree hierachy that is visible inside the frustum, and return how mùany nodes were drawn
#define DrawNodeTreeInFrustum NodeTreeDrawInFrustum 

#if defined(__cplusplus)
}
#endif

#endif // RSCENEGRAPH_H

#if defined(RSCENEGRAPH_IMPLEMENTATION)


Matrix MatrixNormalize( Matrix m )
{
	float len = Vector3Length( (Vector3){ m.m0 , m.m1 , m.m2 } );
	m.m0 /= len ;
	m.m1 /= len ;
	m.m2 /= len ;

	len = Vector3Length( (Vector3){ m.m4 , m.m5 , m.m6 } );
	m.m4 /= len ;
	m.m5 /= len ;
	m.m6 /= len ;

	len = Vector3Length( (Vector3){ m.m8 , m.m9 , m.m10 } );
	m.m8 /= len ;
	m.m9 /= len ;
	m.m10 /= len ;

	return m ;
}

Matrix MatrixRotation( Matrix m )
{
	m.m12 = 0.0f ;
	m.m13 = 0.0f ;
	m.m14 = 0.0f ;

	return MatrixNormalize( m );
}

Node3D NodeAsGroup()
{
	Node3D node ;

	node.model = NULL ;

	node.tint = WHITE ;

	node.position = Vector3Zero();
	node.rotation = MatrixIdentity();
	node.scale    = Vector3One();

	node.positionRelativeToParentBoneId = -1 ;
	node.positionRelativeToParentBoneName = NULL ;

	node.transform = MatrixIdentity();

	node.untransformedBox.min = Vector3Zero();
	node.untransformedBox.max = Vector3Zero();

	node.untransformedCenter = Vector3Zero();
	node.untransformedRadius = 0.0f;

	node.parent = NULL ;
	node.firstChild = NULL ;
	node.nextSibling = NULL ;
	node.prevSibling = NULL ;

	node.nextLOD = NULL ;
	node.nextDistance = 0.0f ;
	node.activeLOD = NULL ;

	node.lastFrustum = NULL ;
	node.insideFrustum = false ;

	node.anims = NULL ;
	node.animsCount = 0 ;
	node.animId = -1;
	node.animPosition = 0.0f;
	node.animSpeed = 1.0f;
	node.animRemainingLoops = -1 ;
	node.animEventCallback = NULL ;

	node.userData = NULL ;

	return node;
}


Node3D NodeAsModel( Model *model )
{
	Node3D node = NodeAsRoot();

	node.model = model ;

	// Get the untransformed boundings :

	{
		Matrix temp = model->transform ;
		model->transform = MatrixIdentity();
		node.untransformedBox = GetModelBoundingBox( *model );
		model->transform = temp;
	}

	node.untransformedCenter.x = ( node.untransformedBox.min.x + node.untransformedBox.max.x )*0.5f ;
	node.untransformedCenter.y = ( node.untransformedBox.min.y + node.untransformedBox.max.y )*0.5f ;
	node.untransformedCenter.z = ( node.untransformedBox.min.z + node.untransformedBox.max.z )*0.5f ;

	node.untransformedRadius = Vector3Distance( node.untransformedBox.min , node.untransformedBox.max )*0.5f ;

	// Update transform matrix and transformed boundings :

	NodeUpdateTransforms( &node );

	return node;
}

void NodeRemoveLOD( Node *node , Node *lod )
{
	Node3D *depth = node ;

	while( depth->nextLOD != NULL && depth->nextLOD != lod )
	{
		depth = depth->nextLOD ;
	}

	if ( depth->nextLOD != NULL && depth->nextLOD == lod )
	{
		depth->nextDistance = lod->nextDistance ;
		depth->nextLOD = lod->nextLOD ;
	}

	lod->nextLOD = NULL ;
}

void NodeInsertLOD( Node *node , Node *lod , float distance )
{
	if ( node->nextLOD == NULL )
	{
		node->nextLOD = lod ;
		node->nextDistance = distance ;
	}
	else
	{
		// We need to scan the LOD chain to find where to insert our new LOD.

		Node3D *link = node ; 
		while( link->nextLOD != NULL && link->nextDistance < distance )
		{
			link = link->nextLOD ;
		}

		// Reached the end ?
		if ( link->nextLOD == NULL )
		{
			// Just add the lod at this end :
			link->nextLOD = lod ;
			link->nextDistance = distance ;
		}
		else // new LOD is closer than current LOD ?
		if ( link->nextDistance > distance )
		{
			// We need to insert the new lod into the chain :

			// 1) connect the rest of the chain to the new lod :
			lod->nextLOD = link->nextLOD ;
			lod->nextDistance = link->nextDistance ;

			// 2) connect the new lod to the head of the chain :
			link->nextLOD = lod ;
			link->nextDistance = distance ;
		}
		else // Same distance ?
		{
			// We need to replace the current LOD with new LOD.

			// 1) Connect the tail of the LOD chain to the new LOD by skipping the LOD we want to replace :
			lod->nextLOD      = link->nextLOD->nextLOD ;
			lod->nextDistance = link->nextLOD->nextDistance ;

			// Disconnect the old LOD from the rest of the chain
			link->nextLOD->nextLOD = NULL ;

			// Connect the new LOD to the head chain :
			link->nextLOD = lod ;
		}
	}

	// Unless there is a bug in my algorithm
	// the new LOD should be in place
}

void NodeUnpackTransforms( Node *node )
{
	node->position = (Vector3){ node->transform.m12 , node->transform.m13 , node->transform.m14 };

	node->scale = (Vector3){
		sqrtf( node->transform.m0*node->transform.m0 + node->transform.m1*node->transform.m1 + node->transform.m2*node->transform.m2 ),
		sqrtf( node->transform.m4*node->transform.m4 + node->transform.m5*node->transform.m5 + node->transform.m6*node->transform.m6 ),
		sqrtf( node->transform.m8*node->transform.m8 + node->transform.m9*node->transform.m9 + node->transform.m10*node->transform.m10 )
	};

	node->rotation = MatrixRotation( node->transform );
}

// Detach a node's branch from its parent
void NodeDetachBranch( Node *node )
{
	// Node's branch is the node and its own childrens.
	// We want to detach this branch from the parent.
	// If the node has siblings, we must also extract it from the siblings chain /!\

	Node3D *prev = node->prevSibling ;
	Node3D *next = node->nextSibling ;

	//                                [parent ]--> *firstChild --> ?
	//                                [_______]
	//                                    A
	//     ______                      ___|___                      ______
	//    [      ]--> *nextSibling -->[       ]--> *nextSibling -->[      ]
	//    [ prev ]                    [ node  ]                    [ next ]
	//    [______]<-- *prevSibling <--[_______]<-- *prevSibling <--[______]
	//                                   | A
	//                       *firstChild | | *parent
	//                                 __V_|__
	//                                [       ]
	//                                [ child ]

	// Extraction from the siblings chain :

	if ( prev != NULL ) prev->nextSibling = next ;
	if ( next != NULL )	next->prevSibling = prev ;

	// Extraction from the parent :
	// All siblings have de same parent, but the parent only refers to the first child.

	if ( node->parent != NULL )
	{
		if ( node->parent->firstChild == node )
		{
			node->parent->firstChild = node->nextSibling ;
		}
	}

	// Cleanup the orphaned node :

	node->parent = NULL;
	node->nextSibling = NULL ;
	node->prevSibling = NULL ;
	node->positionRelativeToParentBoneId = -1 ;
	node->positionRelativeToParentBoneName = NULL ;
}

// Remove a node from its parent, siblings and children
void NodeRemove( Node *node )
{
	Node3D *prev = node->prevSibling ;
	Node3D *next = node->nextSibling ;
	Node3D *child = node->firstChild ;

	//                                [parent ]--> *firstChild --> ?
	//                                [_______]
	//                                    A
	//     ______                      ___|___                      ______
	//    [      ]--> *nextSibling -->[       ]--> *nextSibling -->[      ]
	//    [ prev ]                    [ node  ]                    [ next ]
	//    [______]<-- *prevSibling <--[_______]<-- *prevSibling <--[______]
	//                                   | A
	//                       *firstChild | | *parent
	//                                 __V_|__
	//                                [       ]
	//                                [ child ]--> *nextSibling --> ...

	// If the node has a child, the child must take its place.
	// But if this child already has siblings, we must insert them into the sibling chain too !

	if ( child != NULL )
	{
		// Remember who's the parent :

		Node3D *parent = node->parent ;

		// Detach the node and its children from the parent :

		NodeDetachBranch( node );

		// Each node's children are reattached to the parent as children :

		do
		{
			// Attach the child to the new parent :
			// NOTE : The child will be automaticly detached from its sibling chain in the node.

			NodeAttachChild( parent , child ); 

			// Proceed with the next node's child :
			// NOTE : the node's firstChild was updated with next child in the sibling chain.

			child = node->firstChild  ;
		}
		while( child != NULL );
	}
	else // If the node has no child, we just remove it :
	{
		// Shortcircuit the node in the siblings chain :
		if ( prev != NULL ) prev->nextSibling = next ;
		if ( next != NULL )	next->prevSibling = prev ;

		// All siblings have de same parent, but the parent only refers to the first child.
		if ( node->parent != NULL )
		{
			if ( node->parent->firstChild == node )
			{
				node->parent->firstChild = next ;
			}
		}
	}


	// Cleanup the lonely removed node :

	node->parent      = NULL ;
	node->prevSibling = NULL ;
	node->nextSibling = NULL ;
	node->firstChild  = NULL ;
	node->positionRelativeToParentBoneId = -1 ;
	node->positionRelativeToParentBoneName = NULL ;
}

void NodeAttachChildToBone( Node *parent , Node *child , char *boneName )
{
	if ( child->parent != NULL )
	{
		NodeDetachBranch( child );
	}

	if ( parent->model != NULL )
	{
		for( int i = 0 ; i < parent->model->boneCount ; i++ )
		{
			if ( TextIsEqual( parent->model->bones[i].name , boneName ) )
			{
				child->position = parent->model->bindPose[i].translation ;
				child->positionRelativeToParentBoneId = i ;
				child->positionRelativeToParentBoneName = parent->model->bones[i].name ;
				NodeUpdateTransforms( child );
				break;
			}
		}
	}

	NodeAttachChild( parent, child );
}


void NodeAttachChild( Node *parent, Node *child )
{

	//                                [parent ]--> *firstChild --> ?
	//                                [_______]
	//                                    A
	//     ______                      ___|___                      ______
	//    [      ]--> *nextSibling -->[       ]--> *nextSibling -->[      ]
	//    [ prev ]                    [ node  ]                    [ next ]
	//    [______]<-- *prevSibling <--[_______]<-- *prevSibling <--[______]
	//                                   | A
	//                       *firstChild | | *parent
	//                                 __V_|__
	//                                [       ]
	//                                [ child ]


	// If the child already has a parent, we detach it automaticly as a branch :
	// NOTE : by detaching it, its transform are updated into global space.

	if ( child->parent != NULL )
	{
		NodeDetachBranch( child );
	}

	// Child's adoption procedure :

	child->parent = parent ;

	if ( parent != NULL )
	{
		if ( parent->firstChild == NULL )
		{
			parent->firstChild = child ;
		}
		else
		{
			// We insert the new child as firstChild 
			// so we dont have to traverse the sibling's chain :

			child->nextSibling = parent->firstChild ;
			parent->firstChild->prevSibling = child ;
			parent->firstChild = child ;
		}

		// Move child's transforms to parent's space :
		
		child->transform = MatrixMultiply( child->transform , MatrixInvert( parent->transform ) );
		NodeUnpackTransforms( child );
	}
}

// Update the current animation timeline and call the event callback if set.
void NodeUpdateAnimationTimeline( Node *node , float delta )
{
	// TODO? Update the timeline of the active lod only ? of the main LOD only ? or of all lods ?
	//if ( node->activeLOD ) node = node->activeLOD ; 

	if ( node->animId < 0 ) return ;
	if ( node->animId >= node->animsCount ) return ;
	if ( node->animPosition < 0.0f ) return ;

	// Are we done already ?

	if ( node->animRemainingLoops == 0 ) return ;

	// Update the position in the timeline :

	node->animPosition += delta * node->animSpeed ;

	// Calculate the frame :
	
	int frame = (int)node->animPosition;

	// If reaching the end of the animation :

	if ( frame >= node->anims[ node->animId ].frameCount )
	{
		// We're going to rewind the animation.
		// But because the position on the timeline is a float, we must keep the decimal part.

		node->animPosition = fmodf( node->animPosition , (float)node->anims[ node->animId ].frameCount );

		// Callback events :

		if ( node->animEventCallback != NULL )
		{
			// The number of remaining loops will be decrement later,
			// and the function already returned if it was 0.
			// So, here, either it is a positive number or a negative one.
			// Negative means infinite loop.
			// Positive tells how many loop are remaining.
			// If it is 1, then it is the last one, and the anim is complete.

			if ( node->animRemainingLoops != 1 ) // Not the last one
			{
				node->animEventCallback( node , NODE_ANIMATION_EVENT_LOOP );
			}
			else // The last one :
			{
				node->animEventCallback( node , NODE_ANIMATION_EVENT_COMPLETE );
			}
		}
	}

	// If remaining loop is negative, it means we want infinite loop.
	// So we decrement it only if greater than 0.

	if ( node->animRemainingLoops > 0 )
	{
		node->animRemainingLoops--;
	}
}

void NodeUpdateTransforms( Node *node )
{
	// Update animations :

	if ( node->model != NULL && node->anims != NULL && node->animId >= 0 && node->animId < node->animsCount )
	{
		if ( node->activeLOD == NULL || node->activeLOD == node )
		{
			UpdateModelAnimation( *node->model , node->anims[ node->animId ] , (int)node->animPosition );
		}
	}

	// Calculate node's transformation matrix
	// Get transform matrix (rotation -> scale -> translation)

	Matrix matScale       = MatrixScale( node->scale.x , node->scale.y , node->scale.z );
	Matrix matTranslation = MatrixTranslate( node->position.x , node->position.y , node->position.z );

	node->transform = MatrixMultiply( MatrixMultiply( matScale , node->rotation ) , matTranslation );

/*	if ( node->model ) TODO ???
	{
		print_Matrix( node->transform , "node->transform (before)" );
		print_Matrix( node->model->transform , "node->model->transform" );

		// Combine model transforms with node transforms
		node->transform = MatrixMultiply( node->model->transform , node->transform );

		print_Matrix( node->transform , "node->transform (after)" );
	}
*/
	if ( node->parent )
	{
		node->transform = MatrixMultiply( node->transform , node->parent->transform );
	}

	// Update transformed boundings :
	node->transformedBox = BoundingBoxTransform( node->untransformedBox , node->transform );

	node->transformedCenter.x = ( node->transformedBox.min.x + node->transformedBox.max.x )*0.5f ;
	node->transformedCenter.y = ( node->transformedBox.min.y + node->transformedBox.max.y )*0.5f ;
	node->transformedCenter.z = ( node->transformedBox.min.z + node->transformedBox.max.z )*0.5f ;

	node->transformedRadius = Vector3Distance( node->transformedBox.min , node->transformedBox.max )*0.5f ;
}

void NodeTreeTraversal( Node *root , NodeTreeTraversalCallback callback , void *userData )
{
	Node3D *sibling ;
	Node3D *node = root ;

	while( node )
	{
		callback( node , userData );

		sibling = node ;
		while( sibling = sibling->nextSibling )
		{
			NodeTreeTraversal( sibling , callback , userData );
		}

		node = node->firstChild ;
	}
}

void NodeTreeUpdateTransforms( Node *root )
{
	Node3D *sibling ;
	Node3D *node = root ;

	while( node )
	{
		NodeUpdateTransforms( node );

		sibling = node ;
		while( sibling = sibling->nextSibling )
		{
			NodeTreeUpdateTransforms( sibling );
		}

		node = node->firstChild ;
	}
}

int NodeTreeDrawInFrustum( Node *root , Frustum *frustum )
{
	Node3D *sibling ;
	Node3D *node = root ;
	int nodeDrawn = 0 ;

	while( node )
	{
		if ( NodeDrawInFrustum( node , frustum ) ) nodeDrawn++;

		sibling = node ;
		while( sibling = sibling->nextSibling )
		{
			nodeDrawn += NodeTreeDrawInFrustum( sibling , frustum );
		}

		node = node->firstChild ;
	}

	return nodeDrawn;
}


BoundingBox BoundingBoxTransform( BoundingBox box , Matrix transform )
{
/*
      F---------G
     /|        /|
  { B---------C |
  { | |       | |
 Y{ | E-------|-H  /
  { |/        |/  /Z
  { A---------D  /
    <====X====>
*/

	const int A=0;
	const int B=1;
	const int C=2;
	const int D=3;
	const int E=4;
	const int F=5;
	const int G=6;
	const int H=7;

	const int X=8; // X relative axis = D - A
	const int Y=9; // Y relative axis = B - A
//	const int Z=10; // Z relative axis (not required)

	Vector3 v[10]; // contains A to Y

	// Transform axis corners :

	v[A] = Vector3Transform( box.min , transform );
	v[B] = Vector3Transform( (Vector3){ box.min.x , box.max.y , box.min.z } , transform );
	//C later
	v[D] = Vector3Transform( (Vector3){ box.max.x , box.min.y , box.min.z } , transform );
	v[E] = Vector3Transform( (Vector3){ box.min.x , box.min.y , box.max.z } , transform );

	// Calculate relative axis :

	v[X] = Vector3Subtract( v[D] , v[A] );
	v[Y] = Vector3Subtract( v[B] , v[A] );
	//v[Z] = Vector3Subtract( v[E] , v[A] );

	// Calculate missing corners :

	v[C] = Vector3Add( v[B] , v[X] );

	v[F] = Vector3Add( v[E] , v[Y] );
	v[G] = Vector3Add( v[F] , v[X] );
	v[H] = Vector3Add( v[E] , v[X] );

	// Calculate new AABB :

	box.min = (Vector3){ 9999999.0f , 9999999.0f, 9999999.0f };
	box.max = (Vector3){-9999999.0f ,-9999999.0f,-9999999.0f };

	for( int i = A ; i <= H ; i++ )
	{
		box.min = Vector3Min( box.min , v[i] );
		box.max = Vector3Max( box.max , v[i] );
	}

	return box ;
}

void NodeSetPosition( Node *node , Vector3 pos )
{
	node->transform.m12 = pos.x ;
	node->transform.m13 = pos.y ;
	node->transform.m14 = pos.z ;
}


void NodeRotateX( Node *node , float angle )
{
	NodeRotate( node , (Vector3){ 1.0f , 0.0f, 0.0f } , angle );
}

void NodeRotateY( Node *node , float angle )
{
	NodeRotate( node , (Vector3){ 0.0f , 1.0f, 0.0f } , angle );
}

void NodeRotateZ( Node *node , float angle )
{
	NodeRotate( node , (Vector3){ 0.0f , 0.0f, 1.0f } , angle );
}

void NodeRotate( Node *node , Vector3 axis , float angle )
{
	node->rotation = MatrixMultiply( node->rotation , MatrixRotate( axis , angle ) );
}

void NodePitch( Node *node , float angle )
{
	Vector3 axis = { 
		node->transform.m0 , 
		node->transform.m1 ,
		node->transform.m2 };

	NodeRotate( node , axis , angle );
}

void NodeYaw( Node *node , float angle )
{
	Vector3 axis = { 
		node->transform.m4 , 
		node->transform.m5 ,
		node->transform.m6 };

	NodeRotate( node , axis , angle );
}

void NodeRoll( Node *node , float angle )
{
	Vector3 axis = { 
		node->transform.m8 , 
		node->transform.m9 ,
		node->transform.m10 };

	NodeRotate( node , axis , angle );
}

void NodeMoveSideward( Node *node , float distance )
{
	node->position.x += node->transform.m0 * distance ;
	node->position.y += node->transform.m1 * distance ;
	node->position.z += node->transform.m2 * distance ;
}

void NodeMoveUpward( Node *node , float distance )
{
	node->position.x += node->transform.m4 * distance ;
	node->position.y += node->transform.m5 * distance ;
	node->position.z += node->transform.m6 * distance ;
}


void NodeMoveForward( Node *node , float distance )
{
	node->position.x += node->transform.m8 * distance ;
	node->position.y += node->transform.m9 * distance ;
	node->position.z += node->transform.m10 * distance ;
}

bool NodeDrawInFrustum( Node *node , Frustum *frustum )
{
	node->lastFrustum = frustum ;
	node->insideFrustum = false ;

	node->distanceToCamera = Vector3Distance( node->position , frustum->camera->position );

	// Find the active LOD :

	node->activeLOD = node ;
	while( node->activeLOD->nextLOD != NULL && node->activeLOD->nextDistance < node->distanceToCamera )
	{
		node->activeLOD = node->activeLOD->nextLOD ;
	}

	// Draw the active LOD meshes if inside the frustum :

	Node3D *lod = node->activeLOD ;
	if ( lod->model )
	{
		// Frustum clipping using the main boundings of the node (not of the activeLOD ):

		if ( ! FrustumContainsSphere( frustum , node->transformedCenter , node->transformedRadius ) ) return false ;

		// Draw the meshes of the active LOD :

		for ( int i = 0 ; i < lod->model->meshCount ; i++ )
		{
			Color color = lod->model->materials[ lod->model->meshMaterial[i] ].maps[MATERIAL_MAP_DIFFUSE].color ;

			Color colorTint = WHITE;
			colorTint.r = (unsigned char)( ( (int)color.r*(int)lod->tint.r )/255 );
			colorTint.g = (unsigned char)( ( (int)color.g*(int)lod->tint.g )/255 );
			colorTint.b = (unsigned char)( ( (int)color.b*(int)lod->tint.b )/255 );
			colorTint.a = (unsigned char)( ( (int)color.a*(int)lod->tint.a )/255 );
			
			lod->model->materials[ lod->model->meshMaterial[i] ].maps[MATERIAL_MAP_DIFFUSE].color = colorTint ;

			// Draw lod's mesh using node's transform :
			DrawMesh( lod->model->meshes[i] , lod->model->materials[ lod->model->meshMaterial[i] ] , node->transform );

			lod->model->materials[ lod->model->meshMaterial[i] ].maps[MATERIAL_MAP_DIFFUSE].color = color;
		}

		node->insideFrustum = true ;
	}

	// Did we draw something ?

	return node->insideFrustum ; 
}



// Return the frustum of the camera.
// NOTE : The returned frustum is in World Space coordinates.
Frustum CameraGetFrustum( Camera *camera , float aspect )
{
	Frustum frustum ;

	frustum.camera = camera;
	frustum.aspect = aspect;

	frustum.view = GetCameraViewMatrix( camera );
	frustum.proj = GetCameraProjectionMatrix( camera , aspect );

	Matrix clip = MatrixMultiply( frustum.view , frustum.proj ); // The frustum is calculated in World Space

	frustum.left  = Vector4Normalize( (Vector4){ clip.m3 + clip.m0 , clip.m7 + clip.m4 , clip.m11 + clip.m8 , clip.m15 + clip.m12 } );
	frustum.right = Vector4Normalize( (Vector4){ clip.m3 - clip.m0 , clip.m7 - clip.m4 , clip.m11 - clip.m8 , clip.m15 - clip.m12 } );

	frustum.down  = Vector4Normalize( (Vector4){ clip.m3 + clip.m1 , clip.m7 + clip.m5 , clip.m11 + clip.m9 , clip.m15 + clip.m13 } );
	frustum.up    = Vector4Normalize( (Vector4){ clip.m3 - clip.m1 , clip.m7 - clip.m5 , clip.m11 - clip.m9 , clip.m15 - clip.m13  } );

	frustum.near  = Vector4Normalize( (Vector4){ clip.m3 + clip.m2 , clip.m7 + clip.m6 , clip.m11 + clip.m10 , clip.m15 + clip.m14 } );
	frustum.far   = Vector4Normalize( (Vector4){ clip.m3 - clip.m2 , clip.m7 - clip.m6 , clip.m11 - clip.m10 , clip.m15 - clip.m14 } );

	return frustum;
}

// Return the closest (orthogonal) signed distance between a point and a plane.
// NOTE : A negative distance means the point is under the plane.
float PlaneDistanceToPoint( Vector4 plane , Vector3 point )
{
	float d = point.x * plane.x + point.y * plane.y + point.z * plane.z + plane.w ;
	float e = sqrt( plane.x * plane.x + plane.y * plane.y + plane.z * plane.z );
	float distance = d/e ;
	return distance ;
}

// Check if the point is touching or is under the plane.
bool CheckCollisionPlanePoint( Vector4 plane , Vector3 point )
{
	return PlaneDistanceToPoint( plane , point ) <= 0.0f ;
}

// Check if the sphere is touching or is under the plane.
bool CheckCollisionPlaneSphere( Vector4 plane , Vector3 center , float radius )
{
	return PlaneDistanceToPoint( plane , center ) <= radius ;
}

// Check if the box is touching or is under the plane.
bool CheckCollisionPlaneBox( Vector4 plane , BoundingBox box )
{
/*
      F---------G
     /|        /|
    B---------C |
    | |       | |
    | E-------|-H
    |/        |/
    A---------D
*/

	// A and G corners :
	if ( CheckCollisionPlanePoint( plane , box.min ) ) return true;
	if ( CheckCollisionPlanePoint( plane , box.max ) ) return true;

	// B corner :
	if ( CheckCollisionPlanePoint( plane , (Vector3){ box.min.x , box.max.y , box.min.z } ) ) return true;

	// C corner :
	if ( CheckCollisionPlanePoint( plane , (Vector3){ box.max.x , box.max.y , box.min.z } ) ) return true;

	// D corner :
	if ( CheckCollisionPlanePoint( plane , (Vector3){ box.max.x , box.min.y , box.min.z } ) ) return true;

	// E corner :
	if ( CheckCollisionPlanePoint( plane , (Vector3){ box.min.x , box.min.y , box.max.z } ) ) return true;

	// F corner :
	if ( CheckCollisionPlanePoint( plane , (Vector3){ box.min.x , box.max.y , box.max.z } ) ) return true;

	// H corner :
	if ( CheckCollisionPlanePoint( plane , (Vector3){ box.max.x , box.min.y , box.max.z } ) ) return true;

	return false;
}

// Check which box corners are touching or are under the plane.
int CheckCollisionPlaneBoxEx( Vector4 plane , BoundingBox box )
{
/*
      F---------G
     /|        /|
    B---------C |
    | |       | |
    | E-------|-H
    |/        |/
    A---------D
*/
	int corners = BOX_NO_CORNER;

	// A and G corners :
	if ( CheckCollisionPlanePoint( plane , box.min ) ) corners |= BOX_FRONT_BOTTOM_LEFT;
	if ( CheckCollisionPlanePoint( plane , box.max ) ) corners |= BOX_BACK_TOP_RIGHT;

	// B corner :
	if ( CheckCollisionPlanePoint( plane , (Vector3){ box.min.x , box.max.y , box.min.z } ) ) corners |= BOX_FRONT_TOP_LEFT;

	// C corner :
	if ( CheckCollisionPlanePoint( plane , (Vector3){ box.max.x , box.max.y , box.min.z } ) ) corners |= BOX_FRONT_TOP_RIGHT;

	// D corner :
	if ( CheckCollisionPlanePoint( plane , (Vector3){ box.max.x , box.min.y , box.min.z } ) ) corners |= BOX_FRONT_BOTTOM_RIGHT;

	// E corner :
	if ( CheckCollisionPlanePoint( plane , (Vector3){ box.min.x , box.min.y , box.max.z } ) ) corners |= BOX_BACK_BOTTOM_LEFT;

	// F corner :
	if ( CheckCollisionPlanePoint( plane , (Vector3){ box.min.x , box.max.y , box.max.z } ) ) corners |= BOX_BACK_TOP_LEFT;

	// H corner :
	if ( CheckCollisionPlanePoint( plane , (Vector3){ box.max.x , box.min.y , box.max.z } ) ) corners |= BOX_BACK_BOTTOM_RIGHT;

	return corners;
}

bool FrustumContainsSphere( Frustum *frustum , Vector3 center , float radius )
{
	for( int i = 0 ; i < 6 ; i++ )
	{
		if ( PlaneDistanceToPoint( frustum->plane[i] , center ) <= -radius ) return false;
	}

	return true;
}


bool FrustumContainsPoint( Frustum *frustum , Vector3 point )
{
	return FrustumContainsSphere( frustum , point , 0.0f );
}

bool FrustumContainsBox( Frustum *frustum , BoundingBox box )
{
	// A box is outside the frustum if all its corners are outside a single plane

	for( int i = 0 ; i < 6 ; i++ )
	{
		if ( CheckCollisionPlaneBoxEx( frustum->plane[i] , box ) == BOX_ALL_CORNERS ) return false ;
	}

	return true;
}

#endif //RSCENEGRAPH_IMPLEMENTATION

#ifndef RNODES_H
#define RNODES_H

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "rcamera.h"

#include "rfrustum.h"


typedef enum
{
	NODE_ANIMATION_EVENT_LOOP = 1 ,
	NODE_ANIMATION_EVENT_COMPLETE = 2 ,

} NodeAnimationEvent;


typedef struct Node3D Node3D;
typedef struct Node3D Node;

typedef void (*NodeAnimationEventCallback)( Node *node , NodeAnimationEvent event );

#ifndef NODE3D_NAME_SIZE_MAX
#define NODE3D_NAME_SIZE_MAX 256
#endif

typedef struct AnimationsList
{
	ModelAnimation *list ;
	int count ;
} AnimationsList;

typedef struct Node3D 
{
	char name[ NODE3D_NAME_SIZE_MAX ] ; 

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

	AnimationsList animations ;
	int currentAnimationIndex ;               // Id of the currently selected anim

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


// Node's scenegraph manual API :

RLAPI Node3D NodeAsGroup( char *name );
#define NodeAsRoot NodeAsGroup
RLAPI Node3D NodeAsModel( char *name , Model *model );

RLAPI void NodeSetName( Node *node , char *name );
#define SetNodeName NodeSetName

RLAPI void NodeAttachChild( Node *parent, Node *child );
RLAPI void NodeAttachChildToBone( Node *parent, Node *child , char *boneName );

RLAPI void NodeDetachBranch( Node *node ); // Detach the node and its children from the tree.
#define DetachNodeBranch NodeDetachBranch
RLAPI void NodeRemove( Node *node ); // Remove the node from its tree so that its children take its place.
#define RemoveNode NodeRemove

typedef void (*NodeTreeTraversalCallback)( Node *node , void *userData );

RLAPI void NodeTreeTraversal( Node *root , NodeTreeTraversalCallback callback , void *userData );
#define TraverseNodeTree NodeTreeTraversal

RLAPI void NodeInsertLOD( Node *node , Node *lod , float distance ); // TODO explain
RLAPI void NodeRemoveLOD( Node *node , Node *lod );

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

RLAPI AnimationsList AnimationsListLoad( char *fileName );
#define LoadAnimationsList AnimationsListLoad
RLAPI void NodeSetAnimationsList( Node *node , AnimationsList anims );
#define SetNodeAnimationsList NodeSetAnimationsList
RLAPI void NodeLoadAnimationsList( Node *node , char *fileName );
#define LoadNodeAnimationsList NodeLoadAnimationsList

RLAPI void NodePlayAnimationName( Node *node , char *name );
#define PlayNodeAnimationName NodePlayAnimationName
RLAPI void NodePlayAnimationIndex( Node *node , int index );
#define PlayNodeAnimationIndex NodePlayAnimationIndex

RLAPI void NodeSetAnimationEventCallback( Node *node , NodeAnimationEventCallback callback );
#define SetNodeAnimationsEventCallback NodeSetAnimationEventCallback

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

#endif // RNODES_H

#if defined(RNODES_IMPLEMENTATION)


void NodeSetName( Node *node , char *name )
{
	if ( TextLength( name ) >= NODE3D_NAME_SIZE_MAX )
	{
		TRACELOG( LOG_WARNING , "NODE: [%s,%s,%i] Node name too long. Will be cliped to %d." , __func__ , __FILE__ , __LINE__ , NODE3D_NAME_SIZE_MAX );
	}

	for( int c = 0 ; c < NODE3D_NAME_SIZE_MAX ; c++ )
	{
		node->name[c] = name[c];

		if ( name[c] == 0 ) break;
	}

	node->name[NODE3D_NAME_SIZE_MAX-1] = 0 ;

	if ( TextLength( name ) >= NODE3D_NAME_SIZE_MAX )
	{
		TRACELOG( LOG_WARNING , "%s conseqence : node name is `%s` instead of `%s`" , __func__ , node->name , name );
	}
}

Node3D NodeAsGroup( char * name )
{
	Node3D node ;

	NodeSetName( &node , name );

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

	node.animations.list = NULL ;
	node.animations.count = 0 ;
	node.currentAnimationIndex = -1;
	node.animPosition = 0.0f;
	node.animSpeed = 1.0f;
	node.animRemainingLoops = -1 ;
	node.animEventCallback = NULL ;

	node.userData = NULL ;

	return node;
}


Node3D NodeAsModel( char *name , Model *model )
{
	Node3D node = NodeAsRoot( name );

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


AnimationsList AnimationsListLoad( char *fileName )
{
	AnimationsList anims ;

	anims.list = LoadModelAnimations( fileName , &anims.count );

	return anims ;
}

void NodeSetAnimationsList( Node *node , AnimationsList anims )
{
	node->animations = anims ;
}

void NodeLoadAnimationsList( Node *node , char *fileName )
{
	node->animations = AnimationsListLoad( fileName );
}


void NodePlayAnimationIndex( Node *node , int index )
{
	node->currentAnimationIndex = index ;
	node->animPosition = 0.0f ;
}

void NodePlayAnimationName( Node *node , char *name )
{
	for( int i = 0 ; i < node->animations.count ; i++ )
	{
		if ( TextIsEqual( node->animations.list[ i ].name , name ) )
		{
			NodePlayAnimationIndex( node , i );
			return ;
		}
	}
}

void NodeSetAnimationEventCallback( Node *node , NodeAnimationEventCallback callback )
{
	node->animEventCallback = callback ;
}

// Update the current animation timeline and call the event callback if set.
void NodeUpdateAnimationTimeline( Node *node , float delta )
{
	// TODO? Update the timeline of the active lod only ? of the main LOD only ? or of all lods ?
	//if ( node->activeLOD ) node = node->activeLOD ; 

	if ( node->currentAnimationIndex < 0 ) return ;
	if ( node->currentAnimationIndex >= node->animations.count ) return ;
	if ( node->animPosition < 0.0f ) return ;

	// Are we done already ?

	if ( node->animRemainingLoops == 0 ) return ;

	// Update the position in the timeline :

	node->animPosition += delta * node->animSpeed ;

	// Calculate the frame :
	
	int frame = (int)node->animPosition;

	// If reaching the end of the animation :

	if ( frame >= node->animations.list[ node->currentAnimationIndex ].frameCount )
	{
		// We're going to rewind the animation.
		// But because the position on the timeline is a float, we must keep the decimal part.

		node->animPosition = fmodf( node->animPosition , (float)node->animations.list[ node->currentAnimationIndex ].frameCount );

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

	if ( node->model != NULL && node->animations.list != NULL && node->currentAnimationIndex >= 0 && node->currentAnimationIndex < node->animations.count )
	{
		if ( node->activeLOD == NULL || node->activeLOD == node )
		{
			UpdateModelAnimation( *node->model , node->animations.list[ node->currentAnimationIndex ] , (int)node->animPosition );
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







#endif //RNODES_IMPLEMENTATION

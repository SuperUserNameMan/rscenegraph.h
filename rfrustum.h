#ifndef RFRUSTUM_H
#define RFRUSTUM_H

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

typedef struct Node3D 
{
	Model *model ;

	Vector3    position ;
	Vector3    scale ;
	Quaternion rotation ;

	Matrix transform ; // Computed using position, scale and rotation 

	Color tint ;

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


} Node3D;

typedef struct Node3D Node;


#if defined(__cplusplus)
extern "C" {            // Prevents name mangling of functions
#endif

RLAPI BoundingBox BoundingBoxTransform( BoundingBox box , Matrix transform );

RLAPI float PlaneDistanceToPoint( Vector4 plane , Vector3 point );

RLAPI bool CheckCollisionPlanePoint( Vector4 plane , Vector3 point );
RLAPI bool CheckCollisionPlaneSphere( Vector4 plane , Vector3 center , float radius );
RLAPI bool CheckCollisionPlaneBox( Vector4 plane , BoundingBox box );
RLAPI int  CheckCollisionPlaneBoxEx( Vector4 plane , BoundingBox box ); // Return a BoundingBoxCornersFlag bitfield

RLAPI Frustum CameraGetFrustum( Camera *camera , float aspect );

RLAPI bool FrustumContainsPoint( Frustum *frustum , Vector3 point );
RLAPI bool FrustumContainsSphere( Frustum *frustum , Vector3 center , float radius );
RLAPI bool FrustumContainsBox( Frustum *frustum , BoundingBox box );

RLAPI bool FrustumDrawNode(Frustum *frustum, Node *node); // Draw the node's model if visible inside the frustum

RLAPI Node3D LoadNodeFromModel( Model *model );
RLAPI void NodeUpdateTranforms( Node *node );

// Node's transformations

RLAPI void NodeSetPosition( Node *node , Vector3 pos );

RLAPI void NodeRotate( Node *node , Vector3 axis , float angle );
RLAPI void NodeRotateX( Node *node , float angle ); // Rotate along world's X axis
RLAPI void NodeRotateY( Node *node , float angle ); // Rotate along world's Y axis
RLAPI void NodeRotateZ( Node *node , float angle ); // Rotate along world's Z axis

RLAPI void NodeRotateAlongX( Node *node , float angle ); // Rotate along its own X axis
RLAPI void NodeRotateAlongY( Node *node , float angle ); // Rotate along its own Y axis
RLAPI void NodeRotateAlongZ( Node *node , float angle ); // Rotate along its own Z axis

RLAPI void NodeMoveAlongX( Node *node , float distance ); // Move in direction of its own X axis
RLAPI void NodeMoveAlongY( Node *node , float distance ); // Move in direction of its own Y axis
RLAPI void NodeMoveAlongZ( Node *node , float distance ); // Move in direction of its own Z axis

#if defined(__cplusplus)
}
#endif

#endif // RFRUSTUM_H

#if defined(RFRUSTUM_IMPLEMENTATION)

Node3D LoadNodeFromModel( Model *model )
{
	Node3D node ;
	node.model = model ;
	node.tint = WHITE ;

	node.position = Vector3Zero();
	node.rotation = QuaternionIdentity();
	node.scale    = Vector3One();

	node.transform = MatrixIdentity();

	// Get the untransformed boundings :

	Matrix temp = model->transform ;
	model->transform = MatrixIdentity();
	node.untransformedBox = GetModelBoundingBox( *model );
	model->transform = temp;

	node.untransformedCenter.x = ( node.untransformedBox.min.x + node.untransformedBox.max.x )*0.5f ;
	node.untransformedCenter.y = ( node.untransformedBox.min.y + node.untransformedBox.max.y )*0.5f ;
	node.untransformedCenter.z = ( node.untransformedBox.min.z + node.untransformedBox.max.z )*0.5f ;

	node.untransformedRadius = Vector3Distance( node.untransformedBox.min , node.untransformedBox.max )*0.5f ;

	// Update transform matrix and transformed boundings :

	NodeUpdateTranforms( &node );

	return node;
}

void NodeUpdateTranforms( Node *node )
{
	// Calculate node's transformation matrix
	// Get transform matrix (rotation -> scale -> translation)

	Matrix matRotation    = QuaternionToMatrix( node->rotation );
	Matrix matScale       = MatrixScale( node->scale.x , node->scale.y , node->scale.z );
	Matrix matTranslation = MatrixTranslate( node->position.x , node->position.y , node->position.z );

	node->transform = MatrixMultiply( MatrixMultiply( matScale , matRotation ) , matTranslation );

	// Combine model transformation matrix (model.transform) with matrix generated by function parameters (matTransform)
	node->transform = MatrixMultiply( node->model->transform, node->transform );

	// Update transformed boundings :
	node->transformedBox = BoundingBoxTransform( node->untransformedBox , node->transform );

	node->transformedCenter.x = ( node->transformedBox.min.x + node->transformedBox.max.x )*0.5f ;
	node->transformedCenter.y = ( node->transformedBox.min.y + node->transformedBox.max.y )*0.5f ;
	node->transformedCenter.z = ( node->transformedBox.min.z + node->transformedBox.max.z )*0.5f ;

	node->transformedRadius = Vector3Distance( node->transformedBox.min , node->transformedBox.max )*0.5f ;
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
	node->rotation = QuaternionMultiply( node->rotation , QuaternionFromAxisAngle( axis , angle ) );
}

void NodeRotateAlongX( Node *node , float angle )
{
	Vector3 axis = { 
		node->transform.m0 , 
		node->transform.m1 ,
		node->transform.m2 };

	NodeRotate( node , axis , angle );
}

void NodeRotateAlongY( Node *node , float angle )
{
	Vector3 axis = { 
		node->transform.m4 , 
		node->transform.m5 ,
		node->transform.m6 };

	NodeRotate( node , axis , angle );
}

void NodeRotateAlongZ( Node *node , float angle )
{
	Vector3 axis = { 
		node->transform.m8 , 
		node->transform.m9 ,
		node->transform.m10 };

	NodeRotate( node , axis , angle );
}

void NodeMoveAlongX( Node *node , float distance )
{
	node->position.x += node->transform.m0 * distance ;
	node->position.y += node->transform.m1 * distance ;
	node->position.z += node->transform.m2 * distance ;
}

void NodeMoveAlongY( Node *node , float distance )
{
	node->position.x += node->transform.m4 * distance ;
	node->position.y += node->transform.m5 * distance ;
	node->position.z += node->transform.m6 * distance ;
}


void NodeMoveAlongZ( Node *node , float distance )
{
	node->position.x += node->transform.m8 * distance ;
	node->position.y += node->transform.m9 * distance ;
	node->position.z += node->transform.m10 * distance ;
}


bool FrustumDrawNode( Frustum *frustum , Node *node )
{
	NodeUpdateTranforms( node );


	// Frustum clipping :

	if ( ! FrustumContainsSphere( frustum , node->transformedCenter , node->transformedRadius ) ) return false;

	// Draw the meshes :

	for ( int i = 0 ; i < node->model->meshCount ; i++ )
	{
		Color color = node->model->materials[ node->model->meshMaterial[i] ].maps[MATERIAL_MAP_DIFFUSE].color ;

		Color colorTint = WHITE;
		colorTint.r = (unsigned char)( ( (int)color.r*(int)node->tint.r )/255 );
		colorTint.g = (unsigned char)( ( (int)color.g*(int)node->tint.g )/255 );
		colorTint.b = (unsigned char)( ( (int)color.b*(int)node->tint.b )/255 );
		colorTint.a = (unsigned char)( ( (int)color.a*(int)node->tint.a )/255 );
		
		node->model->materials[ node->model->meshMaterial[i] ].maps[MATERIAL_MAP_DIFFUSE].color = colorTint ;
		DrawMesh( node->model->meshes[i] , node->model->materials[ node->model->meshMaterial[i] ] , node->transform );
		node->model->materials[ node->model->meshMaterial[i] ].maps[MATERIAL_MAP_DIFFUSE].color = color;
	}

	return true;
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

	if (1) // TODO : Perspective mode condition
	{
		frustum.left  = Vector4Normalize( (Vector4){ clip.m3 + clip.m0 , clip.m7 + clip.m4 , clip.m11 + clip.m8 , clip.m15 + clip.m12 } );
		frustum.right = Vector4Normalize( (Vector4){ clip.m3 - clip.m0 , clip.m7 - clip.m4 , clip.m11 - clip.m8 , clip.m15 - clip.m12 } );

		frustum.down  = Vector4Normalize( (Vector4){ clip.m3 + clip.m1 , clip.m7 + clip.m5 , clip.m11 + clip.m9 , clip.m15 + clip.m13 } );
		frustum.up    = Vector4Normalize( (Vector4){ clip.m3 - clip.m1 , clip.m7 - clip.m5 , clip.m11 - clip.m9 , clip.m15 - clip.m13  } );

		frustum.near  = Vector4Normalize( (Vector4){ clip.m3 + clip.m2 , clip.m7 + clip.m6 , clip.m11 + clip.m10 , clip.m15 + clip.m14 } );
		frustum.far   = Vector4Normalize( (Vector4){ clip.m3 - clip.m2 , clip.m7 - clip.m6 , clip.m11 - clip.m10 , clip.m15 - clip.m14 } );
	}
	else
	{
		// TODO : Orthogonal mode
	}

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

#endif //RFRUSTUM_IMPLEMENTATION

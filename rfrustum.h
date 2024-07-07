#ifndef RFRUSTUM_H
#define RFRUSTUM_H

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "rcamera.h"


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
	// Note : their normals point inside the frustum
	// so that if a point is touching or is "under" a plane,
	// it is considered "oustide" the frustum.

	union 
	{
		Vector4 plane[6] ; // x , y , z = normal , w = distance
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



#if defined(__cplusplus)
extern "C" {            // Prevents name mangling of functions
#endif

// AABB BoundingBox stuff :

RLAPI BoundingBox BoundingBoxTransform( BoundingBox box , Matrix transform );

// Matrix space travels :

RLAPI Matrix MatrixNormalize( Matrix m ); // Normalize the scales of the transform matrix (Note : does not check division by zero)

RLAPI Matrix MatrixRotation( Matrix m ); // Normalize the scales, and nullify the translation of the transform matrix
#define MatrixExtractRotationMatrix MatrixRotation
#define RotationMatrixFromMatrix MatrixRotation
#define ExtractRotationMatrixFromMatrix MatrixRotation

RLAPI Vector3 MatrixExtractTranslation( Matrix m );
#define TranslationFromMatrix MatrixExtractTranslation
#define ExtractTranslationFromMatrix MatrixExtractTranslation

RLAPI Vector3 MatrixExtractScale( Matrix m ); // Return the absolute scale of each axis (Note : negative scales not supported)
#define ScaleFromMatrix MatrixExtractScale
#define ExtractScaleFromMatrix MatrixExtractScale

// Plane stuff :

RLAPI float PlaneDistanceToPoint( Vector4 plane , Vector3 point ); // Signed distance between point and plane. Negative distance means the point is under the plane.

RLAPI bool CheckCollisionPlanePoint( Vector4 plane , Vector3 point ); // True if point is touching or under the plane
RLAPI bool CheckCollisionPlaneSphere( Vector4 plane , Vector3 center , float radius ); // True if sphere is touching or under the plane
RLAPI bool CheckCollisionPlaneBox( Vector4 plane , BoundingBox box ); // True if any corner if touching or under the plane
RLAPI int  CheckCollisionPlaneBoxEx( Vector4 plane , BoundingBox box ); // Return a BoundingBoxCornersFlag bitfield to tell which corners

// Frustum stuff :

RLAPI Frustum FrustumFromCamera( Camera *camera , float aspect ); // Compute the furstum of a perspective or orthogonal camera.
#define GetCameraFrustum FrustumFromCamera
#define CameraGetFrustum FrustumFromCamera

RLAPI bool FrustumContainsPoint( Frustum *frustum , Vector3 point ); // True if above all planes
RLAPI bool FrustumContainsSphere( Frustum *frustum , Vector3 center , float radius ); // True if at least a surface is above all planes
RLAPI bool FrustumContainsBox( Frustum *frustum , BoundingBox box ); // True if at least one corner is above all planes

#if defined(__cplusplus)
}
#endif

#endif // RFRUSTUM_H

#if defined(RFRUSTUM_IMPLEMENTATION)

// Normalize each axis of the transform matrix (cancel the scale factor)
// Note : this function does not check for division by zero
Matrix MatrixNormalize( Matrix m )
{
	// Normalize X axis :

	float len = Vector3Length( (Vector3){ m.m0 , m.m1 , m.m2 } ); // Length of X axis
	m.m0 /= len ;
	m.m1 /= len ;
	m.m2 /= len ;

	// Normalize Y axis :

	len = Vector3Length( (Vector3){ m.m4 , m.m5 , m.m6 } );  // Length of Y axis
	m.m4 /= len ;
	m.m5 /= len ;
	m.m6 /= len ;

	// Normalize Z axis :

	len = Vector3Length( (Vector3){ m.m8 , m.m9 , m.m10 } );  // Length of Z axis
	m.m8 /= len ;
	m.m9 /= len ;
	m.m10 /= len ;

	return m ;
}

// Convert a tranform matrix into a rotation matrix :
// Note : does not check for division by zero
Matrix MatrixRotation( Matrix m )
{
	// Nullify the translation :

	m.m12 = 0.0f ;
	m.m13 = 0.0f ;
	m.m14 = 0.0f ;

	// Nullify the scale :

	return MatrixNormalize( m );
}


Vector3 MatrixExtractTranslation( Matrix m )
{
	return (Vector3){ m.m12 , m.m13 , m.m14 };
}

// Extract absolute scales of a tranform matrix :
// Note : won't work as expected with negative scales used for mirroring effects etc
Vector3 MatrixExtractScale( Matrix m )
{
	return (Vector3){
		sqrtf( m.m0*m.m0 + m.m1*m.m1 + m.m2*m.m2 ),  // Absolute length of X axis
		sqrtf( m.m4*m.m4 + m.m5*m.m5 + m.m6*m.m6 ),  // Absolute length of Y axis
		sqrtf( m.m8*m.m8 + m.m9*m.m9 + m.m10*m.m10 ) // Absolute length of Z axis
	};
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

	box.min = (Vector3){ 999999999.0f , 999999999.0f, 999999999.0f }; // TODO FIXME use some limit constants instead ?
	box.max = (Vector3){-999999999.0f ,-999999999.0f,-999999999.0f };

	for( int i = A ; i <= H ; i++ )
	{
		box.min = Vector3Min( box.min , v[i] );
		box.max = Vector3Max( box.max , v[i] );
	}

	return box ;
}


// Return the frustum of the camera.
// NOTE : The returned frustum is in World Space coordinates.
Frustum FrustumFromCamera( Camera *camera , float aspect )
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

// Return true if any part of the sphere is above all planes :
bool FrustumContainsSphere( Frustum *frustum , Vector3 center , float radius )
{
	for( int i = 0 ; i < 6 ; i++ )
	{
		if ( PlaneDistanceToPoint( frustum->plane[i] , center ) <= -radius ) return false;
	}

	return true;
}

// Return true if the point is above all planes :
bool FrustumContainsPoint( Frustum *frustum , Vector3 point )
{
	return FrustumContainsSphere( frustum , point , 0.0f );
}

// Return true if at least one corner is above all planes :
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

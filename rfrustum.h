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
	BOX_NO_CORNER          = 0,

	BOX_FRONT_BOTTOM_LEFT  = 1,
	BOX_FRONT_BOTTOM_RIGHT = 2,
	BOX_FRONT_TOP_LEFT     = 4,
	BOX_FRONT_TOP_RIGHT    = 8,

	BOX_BACK_BOTTOM_LEFT  = 16,
	BOX_BACK_BOTTOM_RIGHT = 32,
	BOX_BACK_TOP_LEFT     = 64,
	BOX_BACK_TOP_RIGHT    = 128,

	BOX_ALL_CORNERS       = 255
} BoundingBoxCornersFlag;

typedef struct Frustum {
	Camera *camera;

	float aspect;

	Matrix proj;
	Matrix view;

	// Frustum planes :
	Vector4 up;
	Vector4 down;
	Vector4 left;
	Vector4 right;
	Vector4 near;
	Vector4 far;

} Frustum;

typedef struct Node3D {
	Model *model;
	Color tint;
	BoundingBox box;
	Vector3 center;
	float radius;
} Node3D;

typedef struct Node3D Node;


#if defined(__cplusplus)
extern "C" {            // Prevents name mangling of functions
#endif

//Matrix GetBoundingBoxMatrix( BoundingBox box );

RLAPI float PlaneDistanceToPoint( Vector4 plane , Vector3 point );

RLAPI bool CheckCollisionPlanePoint( Vector4 plane , Vector3 point );
RLAPI bool CheckCollisionPlaneSphere( Vector4 plane , Vector3 center , float radius );
RLAPI bool CheckCollisionPlaneBox( Vector4 plane , BoundingBox box );
RLAPI int  CheckCollisionPlaneBoxEx( Vector4 plane , BoundingBox box ); // Return a BoundingBoxCornersFlag bitfield

RLAPI Frustum CameraGetFrustum( Camera *camera , float aspect );

RLAPI bool FrustumContainsPoint( Frustum *frustum , Vector3 point );
RLAPI bool FrustumContainsSphere( Frustum *frustum , Vector3 center , float radius );
RLAPI bool FrustumContainsBox( Frustum *frustum , BoundingBox box );

RLAPI bool FrustumDrawNode(Frustum *frustum, Node *node);

RLAPI Node3D LoadNodeFromModel( Model *model );

RLAPI void NodeRotate( Node *node , Vector3 axis , float angle );
RLAPI void NodeRotateY( Node *node , float angle );

#if defined(__cplusplus)
}
#endif

#endif // RFRUSTUM_H

#if defined(RFRUSTUM_IMPLEMENTATION)

Node3D LoadNodeFromModel( Model *model )
{
	Node3D node;
	node.model = model ;
	node.tint = WHITE;
	node.box = GetModelBoundingBox( *model );

	node.center.x = ( node.box.min.x + node.box.max.x )*0.5f;
	node.center.y = ( node.box.min.y + node.box.max.y )*0.5f;
	node.center.z = ( node.box.min.z + node.box.max.z )*0.5f;

	node.radius = Vector3Distance( node.box.min , node.box.max )*0.5f;

	return node;
}

void NodeRotateY( Node * node , float angle )
{
	NodeRotate( node , (Vector3){ 0.0f , 1.0f, 0.0f } , angle );
}

void NodeRotate( Node * node , Vector3 axis , float angle )
{
	node->model->transform = MatrixMultiply( node->model->transform , MatrixRotate( axis , angle ) );
}

bool FrustumDrawNode( Frustum *frustum , Node *node )
{
    Vector3 center = Vector3Transform( node->center , node->model->transform );

    if ( ! FrustumContainsSphere( frustum, node->center, node->radius ) ) return false;

    for (int i = 0; i < node->model->meshCount; i++)
    {
        Color color = node->model->materials[node->model->meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color;

        Color colorTint = WHITE;
        colorTint.r = (unsigned char)(((int)color.r*(int)node->tint.r)/255);
        colorTint.g = (unsigned char)(((int)color.g*(int)node->tint.g)/255);
        colorTint.b = (unsigned char)(((int)color.b*(int)node->tint.b)/255);
        colorTint.a = (unsigned char)(((int)color.a*(int)node->tint.a)/255);

        node->model->materials[node->model->meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = colorTint;
        DrawMesh(node->model->meshes[i], node->model->materials[node->model->meshMaterial[i]], node->model->transform);
        node->model->materials[node->model->meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = color;
    }

	DrawBoundingBox( node->box , RED );

	return true;
}

/*Matrix GetBoundingBoxMatrix( BoundingBox box )
{
	float xAxis = box.max.x - box.min.x ;
	float yAxis = box.max.y - box.min.y ;
	float zAxis = box.max.z - box.min.z ;

	Matrix m = { 
		xAxis , 0.0f , 0.0f , box.min.x ,
		0.0f , yAxis , 0.0f , box.min.y ,
		0.0f , 0.0f , zAxis , box.min.z ,
		0.0f , 0.0f ,  0.0f , 1.0f 
	};

	return m;
}*/




/*
void DrawMatrix( Matrix m )
{
	Vector3 pos   = { m.m12 , m.m13 , m.m14 };
	Vector3 zAxis = { m.m8 + pos.x , m.m9 + pos.y , m.m10 + pos.z };
	Vector3 yAxis = { m.m4 + pos.x , m.m5 + pos.y , m.m6  + pos.z };
	Vector3 xAxis = { m.m0 + pos.x , m.m1 + pos.y , m.m2  + pos.z };

	DrawLine3D( pos , xAxis , RED );
	DrawLine3D( pos , yAxis , GREEN );
	DrawLine3D( pos , zAxis , BLUE );
}
*/

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
	if ( PlaneDistanceToPoint( frustum->left , center ) <= -radius ) return false;
	if ( PlaneDistanceToPoint( frustum->right, center ) <= -radius ) return false;
	if ( PlaneDistanceToPoint( frustum->up   , center ) <= -radius ) return false;
	if ( PlaneDistanceToPoint( frustum->down , center ) <= -radius ) return false;
	if ( PlaneDistanceToPoint( frustum->far  , center ) <= -radius ) return false;
	if ( PlaneDistanceToPoint( frustum->near , center ) <= -radius ) return false;
	return true;
}


bool FrustumContainsPoint( Frustum *frustum , Vector3 point )
{
	return FrustumContainsSphere( frustum , point , 0.0f );
}

bool FrustumContainsBox( Frustum *frustum , BoundingBox box )
{
	// A box is outside the frustum if all its corners are outside a single plane

	if ( CheckCollisionPlaneBoxEx( frustum->up   , box ) == BOX_ALL_CORNERS ) return false ;
	if ( CheckCollisionPlaneBoxEx( frustum->down , box ) == BOX_ALL_CORNERS ) return false ;
	if ( CheckCollisionPlaneBoxEx( frustum->left , box ) == BOX_ALL_CORNERS ) return false ;
	if ( CheckCollisionPlaneBoxEx( frustum->right, box ) == BOX_ALL_CORNERS ) return false ;
	if ( CheckCollisionPlaneBoxEx( frustum->near , box ) == BOX_ALL_CORNERS ) return false ;
	if ( CheckCollisionPlaneBoxEx( frustum->far  , box ) == BOX_ALL_CORNERS ) return false ;

	return true;
}

#endif //RFRUSTUM_IMPLEMENTATION
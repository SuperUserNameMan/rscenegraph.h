#include "raylib.h"

#define RFRUSTUM_IMPLEMENTATION
#include "rfrustum.h"

//--------

int main( int argc , char** argv )
{
	const int W = 800 ;
	const int H = 450;

	InitWindow( W , H , "rfrustum.h example - Basic frustum clipping test" );

	Camera camera = { 0 };
	camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
	camera.target   = (Vector3){ 0.0f, 0.0f, 0.0f };    // Camera looking at point
	camera.up       = (Vector3){ 0.0f, 1.0f, 0.0f };    // Camera up vector (rotation towards target)
	camera.fovy     = 45.0f;                            // Camera field-of-view Y
	camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type

	Vector3 position = (Vector3){5.0, 3.0, -10.0};      // Set model positions

	SetTargetFPS( 60 );

	while( ! WindowShouldClose() )
	{
		UpdateCamera( &camera , CAMERA_ORBITAL ); 

		Frustum frustum = GetCameraFrustum( &camera , (float)GetScreenWidth()/(float)GetScreenHeight() );

		BeginDrawing();

			ClearBackground( RAYWHITE );

			BeginMode3D( camera );

				if ( FrustumContainsSphere( &frustum, position , 1.0 ) )
				{
					// Draw a cube larger than the bounding sphere so we can actually see when it is frustum-clipped
					DrawCubeWires( position , 3,3,3, RED ); 
					DrawSphereWires( position , 1 , 32 , 32 , BLUE );
				}

				
				DrawGrid( 20 , 1.0f );

			EndMode3D();

			DrawText( "The Cube and the Sphere are drawn ..." , 10 , 10 , 24 , BLACK );
			DrawText( "... only when the Sphere is inside the camera frustum." , 10 , 40 , 24 , BLACK );

		EndDrawing();
	}


	CloseWindow();
	

	return 0;
}

//EOF

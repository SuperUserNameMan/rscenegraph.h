#ifndef RSCENEGRAPH_H
#define RSCENEGRAPH_H

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "rcamera.h"

#include "rfrustum.h"
#include "rnodes.h"

#ifndef SCENE3D_NAME_SIZE_MAX
#define SCENE3D_NAME_SIZE_MAX NODE3D_NAME_SIZE_MAX
#endif

typedef struct Scene3D
{
	char name[ SCENE3D_NAME_SIZE_MAX ];
	Node3D *root ;

	Node3D *nodeSlots ;
	int nodeSlotsSize ;
	int nodeSlotsIndex ;

	Model *modelSlots ;
	int modelSlotsSize ;
	int modelSlotsIndex ;

	ModelAnimation **animSlots ;
	int animSlotsSize ;
	int anomSlotsIndex ;

	void *userData ;

} Scene3D ;

typedef Scene3D Scene ;

#if defined(__cplusplus)
extern "C" {            // Prevents name mangling of functions
#endif


// Scenegraph :

RLAPI void SceneSetName( Scene3D *scene , char *name );
#define SetSceneName SceneSetName

RLAPI Scene3D *SceneCreate( char *name , int numberOfSlots );
#define CreateScene SceneCreate

//RLAPI Scene3D SceneLoad( char *name , char *filename );
//RLAPI void SceneUnload( Scene3D *scene );
//RLAPI void SceneSave( Scene3D *scene , char *filename );

RLAPI Scene3D *SceneRelease( Scene3D *scene );
#define ReleaseScene SceneRelease
#define UnloadScene SceneRelease

#if defined(__cplusplus)
}
#endif

#endif // RSCENEGRAPH_H

#if defined(RSCENEGRAPH_IMPLEMENTATION)


Scene3D *SceneCreate( char *name , int numberOfSlots )
{
	Scene3D *scene = (Scene3D*)MemAlloc( sizeof( Scene3D ) );

	SceneSetName( scene , name );

	scene->root = NULL ;

	scene->nodeSlots = (Node3D*)MemAlloc( sizeof( Node3D ) * numberOfSlots );
	scene->nodeSlotsSize = numberOfSlots ;
	scene->nodeSlotsIndex = 0 ;

	scene->modelSlots = (Model*)MemAlloc( sizeof( Model ) * numberOfSlots );
	scene->modelSlotsSize = numberOfSlots ;
	scene->modelSlotsIndex = 0 ;

	scene->animSlots = (ModelAnimation**)MemAlloc( sizeof( ModelAnimation* ) * numberOfSlots );
	scene->animSlotsSize = numberOfSlots ;
	scene->animSlotsIndex = 0 ;

	scene->userData = NULL ;

	return scene ;
}

Scene3D *SceneRelease( Scene3D *scene )
{
	MemFree( scene->nodeSlots );
	MemFree( scene->modelSlots );
	MemFree( scene->animSlots );

	MemFree( scene );
	return NULL ;
}


void SceneSetName( Scene3D *scene , char *name )
{
	if ( TextLength( name ) >= SCENE3D_NAME_SIZE_MAX )
	{
		TRACELOG( LOG_WARNING , "SCENE: [%s,%s,%i] Scene name too long. Will be cliped to %d bytes." , __func__ , __FILE__ , __LINE__ , SCENE3D_NAME_SIZE_MAX );
	}

	for( int c = 0 ; c < SCENE3D_NAME_SIZE_MAX ; c++ )
	{
		scene->name[c] = name[c];

		if ( name[c] == 0 ) break;
	}

	scene->name[SCENE3D_NAME_SIZE_MAX-1] = 0 ;

	if ( TextLength( name ) >= SCENE3D_NAME_SIZE_MAX )
	{
		TRACELOG( LOG_WARNING , "%s conseqence : scene name is `%s` instead of `%s`" , __func__ , scene->name , name );
	}
}

/*
Scene3D SceneLoad( char *name , char *filename );
RLAPI void SceneUnload( Scene3D *scene );
RLAPI void SceneSave( Scene3D *scene , char *filename );

Node * NodeTreeLoad( char *fileName )
{
	char *text = LoadFileText( fileName );
	int length = TextLength( text );

	int at = 0 ;
	int lineCount = 1 ;

	Node tree = NodeAsRoot();

#define SKIP_LINE_NodeTreeLoad() { while( text[ at ] != 0 && at < length && TextFindIndex( "\r\n" , text[at] ) == -1 ) at++ ;}
#define SKIP_SPACES_NodeTreeLoad() { while( text[ at ] != 0 && at < length && TextFindIndex( " \t" , text[at] ) != -1 ) at++ ;}
#define STOP_AT_NodeTreeLoad( s ) { while( text[ at ] != 0 && at < length && TextFindIndex( (s) , text[at] ) == -1 ) at++; }

	while( text[ at ] != 0 && at < length ) 
	{
		if ( text[ at ] == '\r' ) // MAC new line
		{
			lineCount++;
			at++;
			if ( text[ at ] == '\n' ) at++; // Windows new line
			continue ;
		}

		if ( text[ at ] == '\n' ) // Linux new line
		{
			lineCount++;
			at++;
			continue;
		}

		if ( text[ at ] > 0 && text[ at ] <= ' ' ) // blank spaces 
		{
			at++;
			continue ;
		}

		if ( text[ at ] == ';' ) // comment
		{
			SKIP_LINE_NodeTreeLoad();
			continue ;
		}

		if ( text[ at ] == '[' ) // node
		{
			at++;

			SKIP_SPACES_NodeTreeLoad();

			int nameStart = at ;
			int nameEnd = at ;

			STOP_AT_NodeTreeLoad( "\r\n]" );

			if ( text[ at ] != ']' )
			{
				TRACELOG( LOG_ERROR , "NODE: [%s] expecting ']' at line %d" , fileName , lineCount );
				return tree ;
			}
			else
			{
				nameEnd = at-1 ;
			}
			continue;
		}
	}

#undef SKIP_LINE_NodeTreeLoad
#undef SKIP_SPACES_NodeTreeLoad

	UnloadFileText( text );
}
*/




#endif //RSCENEGRAPH_IMPLEMENTATION

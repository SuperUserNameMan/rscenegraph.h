#ifndef RSCENEGRAPH_H
#define RSCENEGRAPH_H

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "rcamera.h"

#include "rfrustum.h"
#include "rnodes.h"


typedef struct Scene3D
{
	char name[ NODE3D_NAME_SIZE_MAX ];
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

} Scene3D ;

#if defined(__cplusplus)
extern "C" {            // Prevents name mangling of functions
#endif


// Node's scenegraph :

//RLAPI Scene3D SceneLoad( char *name , char *filename );
//RLAPI void SceneUnload( Scene3D *scene );
//RLAPI void SceneSave( Scene3D *scene , char *filename );



#if defined(__cplusplus)
}
#endif

#endif // RSCENEGRAPH_H

#if defined(RSCENEGRAPH_IMPLEMENTATION)


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

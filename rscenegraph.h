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


typedef Node3D* SceneNode ;
typedef Model* SceneModel ;
typedef AnimationsList* SceneAnimationsList ;


typedef struct Scene3D
{
	char name[ SCENE3D_NAME_SIZE_MAX ];
	
	Node3D *nodeSlots ;
	int nodeSlotsSize ;
	int nodeSlotsIndex ;

	Model *modelSlots ;
	char **modelFileNames ;
	int modelSlotsSize ;
	int modelSlotsIndex ;

	AnimationsList *animationsSlots ;
	char **animationsFileNames ;
	int animationsSlotsSize ;
	int animationsSlotsIndex ;

	int numberOfNewSlotsOnResize ;

	void *userData ;

} Scene3D ;

typedef Scene3D Scene ;
typedef Scene3D* Scene3DSlot ;
typedef Scene3D* SceneSlot ;

#if defined(__cplusplus)
extern "C" {            // Prevents name mangling of functions
#endif


// Scenegraph :

RLAPI void SceneSetName( Scene3D *scene , char *name );
#define SetSceneName SceneSetName

RLAPI Scene3D *SceneCreate( char *name , int numberOfSlots , int numberOfNewSlotsOnResize );
#define CreateScene SceneCreate

//RLAPI Scene3D SceneLoad( char *name , char *fileName );
//RLAPI void SceneUnload( Scene3D *scene );
RLAPI bool SceneSave( Scene3D *scene , char *fileName );

RLAPI Scene3D *SceneRelease( Scene3D *scene );
#define ReleaseScene SceneRelease
#define UnloadScene SceneRelease

RLAPI Node *SceneFindNode( Scene3D *scene , char *name );
#define FindSceneNode SceneFindNode

RLAPI Node3D *SceneGetNewNodeSlot( Scene3D *scene );
RLAPI Model *SceneGetNewModelSlot( Scene3D *scene );
RLAPI AnimationsList *SceneGetNewAnimationsSlot( Scene3D *scene );

RLAPI Model *SceneLoadModel( Scene3D *scene , char *fileName );
RLAPI AnimationsList *SceneLoadAnimations( Scene3D *scene , char *fileName );

RLAPI Node3D *SceneCreateNodeAsGroup( Scene3D *scene , char *name );
#define SceneCreateNodeAsRoot SceneCreateNodeAsGroup
#define SceneNodeAsRoot SceneCreateNodeAsGroup
#define SceneNodeAsGroup SceneCreateNodeAsGroup
#define CreateSceneNodeAsRoot SceneCreateNodeAsGroup
#define CreateSceneNodeAsGroup SceneCreateNodeAsGroup
RLAPI Node3D *SceneCreateNodeAsModel( Scene3D *scene , char *name , Model *model );
#define SceneNodeAsModel SceneCreateNodeAsModel
#define CreateSceneNodeAsModel SceneCreateNodeAsModel

RLAPI int SceneFindNodeIndex( Scene3D *scene , Node3D *node );
RLAPI int SceneFindModelIndex( Scene3D *scene , Model *model );
RLAPI int SceneFindAnimationsIndex( Scene3D *scene , AnimationsList *anims );

#if defined(__cplusplus)
}
#endif

#endif // RSCENEGRAPH_H

#if defined(RSCENEGRAPH_IMPLEMENTATION)


Scene3D *SceneCreate( char *name , int numberOfSlots , int numberOfNewSlotsOnResize )
{
	Scene3D *scene = (Scene3D*)MemAlloc( sizeof( Scene3D ) );

	SceneSetName( scene , name );

	scene->nodeSlots = (Node3D*)MemAlloc( sizeof( Node3D )*numberOfSlots );
	scene->nodeSlotsSize = numberOfSlots ;
	scene->nodeSlotsIndex = 0 ;

	scene->modelSlots = (Model*)MemAlloc( sizeof( Model )*numberOfSlots );
	scene->modelFileNames = (char**)MemAlloc( sizeof( char* )*numberOfSlots );
	scene->modelSlotsSize = numberOfSlots ;
	scene->modelSlotsIndex = 0 ;

	scene->animationsSlots = (AnimationsList*)MemAlloc( sizeof( AnimationsList* )*numberOfSlots );
	scene->animationsFileNames = (char**)MemAlloc( sizeof( char* )*numberOfSlots );
	scene->animationsSlotsSize = numberOfSlots ;
	scene->animationsSlotsIndex = 0 ;

	scene->numberOfNewSlotsOnResize = numberOfNewSlotsOnResize ;

	scene->userData = NULL ;

	return scene ;
}

Scene3D *SceneRelease( Scene3D *scene )
{
	MemFree( scene->nodeSlots );

	MemFree( scene->modelSlots );

	for( int i = 0 ; i < scene->modelSlotsSize ; i++ )
	{
		if ( scene->modelFileNames[ i ] != NULL )
		{
			MemFree( scene->modelFileNames[ i ] );
		}
	}

	MemFree( scene->animationsSlots );

	for( int i = 0 ; i < scene->animationsSlotsSize ; i++ )
	{
		if ( scene->animationsFileNames[ i ] != NULL )
		{
			MemFree( scene->animationsFileNames[ i ] );
		}
	}

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

Node *SceneFindNode( Scene3D *scene , char *name )
{
	// TODO use hash 

	for( int i = 0 ; i < scene->nodeSlotsIndex ; i++ )
	{
		if ( TextIsEqual( name , scene->nodeSlots[ i ].name ) )
		{
			return &scene->nodeSlots[ i ];
		}
	}

	return NULL ;
}

Node *SceneGetNewNodeSlot( Scene3D *scene )
{
	if ( scene->nodeSlotsIndex >= scene->nodeSlotsSize )
	{
		if ( scene->numberOfNewSlotsOnResize <= 0 ) return NULL ;

		scene->nodeSlotsSize += scene->numberOfNewSlotsOnResize ;
		scene->nodeSlots = (Node3D*)MemRealloc( scene->nodeSlots , sizeof(Node3D)*scene->nodeSlotsSize );
	}

	Node *node = &( scene->nodeSlots[ scene->nodeSlotsIndex ] );

	scene->nodeSlotsIndex++;

	return node ;
}

Model *SceneGetNewModelSlot( Scene3D *scene )
{
	if ( scene->modelSlotsIndex >= scene->modelSlotsSize )
	{
		if ( scene->numberOfNewSlotsOnResize <= 0 ) return NULL ;

		scene->modelSlotsSize += scene->numberOfNewSlotsOnResize ;
		scene->modelSlots = (Model*)MemRealloc( scene->modelSlots , sizeof(Model)*scene->modelSlotsSize );
		scene->modelFileNames = (char**)MemRealloc( scene->modelFileNames , sizeof(char*)*scene->modelSlotsSize );
	}

	Model *model = &( scene->modelSlots[ scene->modelSlotsIndex ] );

	scene->modelFileNames[ scene->modelSlotsIndex ] = NULL ;

	scene->modelSlotsIndex++;

	return model ;
}

AnimationsList *SceneGetNewAnimationsSlot( Scene3D *scene )
{
	if ( scene->animationsSlotsIndex >= scene->animationsSlotsSize )
	{
		if ( scene->numberOfNewSlotsOnResize <= 0 ) return NULL ;

		scene->animationsSlotsSize += scene->numberOfNewSlotsOnResize ;
		scene->animationsSlots = (AnimationsList*)MemRealloc( scene->animationsSlots , sizeof(AnimationsList)*scene->animationsSlotsSize );
		scene->animationsFileNames = (char**)MemRealloc( scene->animationsFileNames , sizeof(char*)*scene->animationsSlotsSize );
	}

	AnimationsList *anims = &scene->animationsSlots[ scene->animationsSlotsIndex ] ;
	
	anims->list = NULL ;
	anims->count = 0 ;

	scene->animationsFileNames[ scene->animationsSlotsIndex ] = NULL ;

	scene->animationsSlotsIndex++;

	return anims ;
}

Model *SceneLoadModel( Scene3D *scene , char *fileName )
{
	Model *model = SceneGetNewModelSlot( scene );

	if ( model != NULL )
	{
		*model = LoadModel( fileName );
		
		scene->modelFileNames[ scene->modelSlotsIndex - 1 ] = (char*)MemAlloc( TextLength( fileName ) + 1 );
		TextCopy( scene->modelFileNames[ scene->modelSlotsIndex - 1 ] , fileName );
	}

	return model ;
}

AnimationsList *SceneLoadAnimations( Scene3D *scene , char *fileName )
{
	AnimationsList *anims = SceneGetNewAnimationsSlot( scene );

	if ( anims != NULL )
	{
		anims->list = LoadModelAnimations( fileName , &anims->count );

		scene->animationsFileNames[ scene->animationsSlotsIndex - 1 ] = (char*)MemAlloc( TextLength( fileName ) + 1 );
		TextCopy( scene->animationsFileNames[ scene->animationsSlotsIndex - 1 ] , fileName );
	}

	return anims ;
}

Node3D *SceneCreateNodeAsGroup( Scene3D *scene , char *name )
{
	Node3D *node = SceneGetNewNodeSlot( scene );

	if ( node != NULL )
	{
		if ( SceneFindNode( scene , name ) != NULL )
		{
			TRACELOG( LOG_WARNING , "SCENE[%s]: Node name `%s` is a duplicate." , scene->name , name ); 
		}

		*node = NodeAsGroup( name );
	}

	return node ;
}

Node3D *SceneCreateNodeAsModel( Scene3D *scene , char *name , Model *model )
{
	Node3D *node = SceneGetNewNodeSlot( scene );

	if ( node != NULL )
	{
		if ( SceneFindNode( scene , name ) != NULL )
		{
			TRACELOG( LOG_WARNING , "SCENE[%s]: Node name `%s` is a duplicate." , scene->name , name ); 
		}

		*node = NodeAsModel( name , model );
	}

	return node ;
}

int SceneFindNodeIndex( Scene3D *scene , Node3D *node )
{
	for( int i = 0 ; i < scene->nodeSlotsIndex ; i++ )
	{
		if ( &(scene->nodeSlots[ i ]) == node ) return i ;
	}

	return -1 ;
}

int SceneFindModelIndex( Scene3D *scene , Model *model )
{
	for( int i = 0 ; i < scene->modelSlotsIndex ; i++ )
	{
		if ( &(scene->modelSlots[ i ]) == model ) return i ;
	}

	return -1 ;
}

int SceneFindAnimationsIndex( Scene3D *scene , AnimationsList *anims )
{
	for( int i = 0 ; i < scene->animationsSlotsIndex ; i++ )
	{
		if ( scene->animationsSlots[ i ].list == anims->list ) return i ;
	}

	return -1 ;
}

bool SceneSave( Scene3D *scene , char *fileName )
{
	FILE *fout = fopen( fileName , "wt" );

	if ( fout == NULL )
	{
		TRACELOG( LOG_ERROR , "SCENE: Can't save scene to `%s`." , fileName );
		return false ;
	}

	fprintf( fout , "[SCENE \"%s\"]\n" , scene->name );

	fprintf( fout , "nodes = %d\n" , scene->nodeSlotsIndex );
	fprintf( fout , "models = %d\n" , scene->modelSlotsIndex );
	fprintf( fout , "animations = %d\n" , scene->animationsSlotsIndex );

	for( int i = 0 ; i < scene->modelSlotsIndex ; i++ )
	{
		if ( scene->modelFileNames[ i ] != NULL )
		{
			fprintf( fout , "\n[MODEL %d \"%s\"]\n" , i , scene->modelFileNames[ i ] );
		}
		else
		{
			fprintf( fout , "\n[MODEL %d extern]\n" , i );
		}

		fprintf( fout , "transform = %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n" ,
			scene->modelSlots[ i ].transform.m0 ,
			scene->modelSlots[ i ].transform.m1 ,
			scene->modelSlots[ i ].transform.m2 ,
			scene->modelSlots[ i ].transform.m3 ,
			scene->modelSlots[ i ].transform.m4 ,
			scene->modelSlots[ i ].transform.m5 ,
			scene->modelSlots[ i ].transform.m6 ,
			scene->modelSlots[ i ].transform.m7 ,
			scene->modelSlots[ i ].transform.m8 ,
			scene->modelSlots[ i ].transform.m9 ,
			scene->modelSlots[ i ].transform.m10 ,
			scene->modelSlots[ i ].transform.m11 ,
			scene->modelSlots[ i ].transform.m12 ,
			scene->modelSlots[ i ].transform.m13 ,
			scene->modelSlots[ i ].transform.m14 ,
			scene->modelSlots[ i ].transform.m15 );
	}

	for( int i = 0 ; i < scene->animationsSlotsIndex ; i++ )
	{
		if ( scene->animationsFileNames[ i ] != NULL )
		{
			fprintf( fout , "\n[ANIMS %d \"%s\"]\n" , i , scene->animationsFileNames[ i ] );
		}
		else
		{
			fprintf( fout , "\n[ANIMS %d extern]\n" , i );
		}
		fprintf( fout , "count = %d\n" , scene->animationsSlots[ i ].count );
	}

	for( int i = 0 ; i < scene->nodeSlotsIndex ; i++ )
	{
		Node3D *node = &(scene->nodeSlots[ i ]);

		fprintf( fout , "\n[NODE %d \"%s\"]\n" , i , node->name );

		fprintf( fout , "parent = %d\n" , node->parent == NULL ? -1 : SceneFindNodeIndex( scene , node->parent ) );

		fprintf( fout , "toBone = %d \"%s\"\n" , node->positionRelativeToParentBoneId , node->positionRelativeToParentBoneName );

		fprintf( fout , "model = %d\n" , node->model == NULL ? -1 : SceneFindModelIndex( scene , node->model ) );

		fprintf( fout , "tint = %d %d %d %d\n" , node->tint.r , node->tint.g , node->tint.b , node->tint.a );

		fprintf( fout , "position = %f %f %f\n" , node->position.x , node->position.y , node->position.z );
		fprintf( fout , "scale = %f %f %f\n" , node->scale.x , node->scale.y , node->scale.z );

		// Rotation as a 3x3 rotation matrix :
		fprintf( fout , "rotation = %f %f %f %f %f %f %f %f %f\n" , 
				node->rotation.m0 , node->rotation.m1 , node->rotation.m2 ,
				node->rotation.m4 , node->rotation.m5 , node->rotation.m6 ,
				node->rotation.m8 , node->rotation.m9 , node->rotation.m10 );

		fprintf( fout , "nextLOD = %d %f\n" , SceneFindNodeIndex( scene , node->nextLOD ) , node->nextDistance );

		fprintf( fout , "anims = %d\n" , SceneFindAnimationsIndex( scene , &node->animations ) );
	}

	fclose( fout );

	return true ;
}




#endif //RSCENEGRAPH_IMPLEMENTATION

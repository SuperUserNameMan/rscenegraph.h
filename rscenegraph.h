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

// Missing text funcs :

RLAPI bool TextBeginsWith( const char *text , const char *with );
#define TextContains( t , s ) ( TextFindIndex( (t) , (s) ) >= 0 )

RLAPI bool _TextIsInteger( const char *text ); // Ignore white characters surrounding the value.
RLAPI int _TextToInteger( const char *text ); // Better version as it ignore white characters before and after

// Scenegraph :

RLAPI void SceneSetName( Scene3D *scene , char *name );
#define SetSceneName SceneSetName

RLAPI Scene3D *SceneCreate( char *name , int numberOfSlots , int numberOfNewSlotsOnResize );
#define CreateScene SceneCreate

//RLAPI Scene3D SceneLoad( char *name , char *fileName );
//RLAPI void SceneUnload( Scene3D *scene );
RLAPI bool SceneSave( Scene3D *scene , char *fileName );
#define SaveScene SceneSave

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

#include <stdlib.h>

#define MAX_SCENE_KEYVAL_KEY_LENGTH 64
#if MAX_TEXT_BUFFER_LENGTH < MAX_SCENE_KEYVAL_KEY_LENGTH
	#undef MAX_SCENE_KEYVAL_KEY_LENGTH
	#define MAX_SCENE_KEYVAL_KEY_LENGTH MAX_TEXT_BUFFER_LENGTH
#endif

void _SceneForceResizeAnimationsSlots( Scene3D *scene , int newSize );
void _SceneForceResizeModelSlots( Scene3D *scene , int newSize );
void _SceneForceResizeNodeSlots( Scene3D *scene , int newSize );


bool TextBeginsWith( const char *text , const char *with )
{
	int text_len = TextLength( text );
	int with_len = TextLength( with );

	if ( text_len < with_len ) return false ;

	for( int i = 0 ; i < with_len ; i++ )
	{
		if ( text[ i ] != with[ i ] ) return false ;
	}

	return true ;
}

bool _TextIsInteger( const char *text )
{
	// Left trimming :

	while( text[0] > 0 && text[0] <= ' ' ) text++;

	// Ignore sign :

	if ( text[0] == '+' || text[0] == '-' ) text++;

	// Integer part contains only digits 0-9 :

	while( text[0] >= '0' && text[0] <= '9' ) text++;

	// Right trimming

	while( text[0] > 0 && text[0] <= ' ' ) text++;

	// The last character must be 0 :

	return text[0] == 0 ;
}

int _TextToInteger( const char *text )
{
	// Left trimming :

	while( text[0] > 0 && text[0] <= ' ' ) text++;

	// Sign :

	int sign = text[0] == '-' ? -1 : 1 ;

	if ( text[0] == '+' || text[0] == '-' ) text++;

	// Integer part contains only digits 0-9 :

	int value = 0 ;

	while( text[0] >= '0' && text[0] <= '9' ) 
	{
		value = value * 10 + ( text[0] - '0' );
		text++;
	}

	value *= sign ;

	return value ;
}

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

	for( int i = 0 ; i < scene->modelSlotsIndex ; i++ )
	{
		if ( scene->modelFileNames[ i ] != NULL )
		{
			UnloadModel( scene->modelSlots[ i ] );
			MemFree( scene->modelFileNames[ i ] );
		}
	}

	MemFree( scene->modelSlots );

	for( int i = 0 ; i < scene->animationsSlotsIndex ; i++ )
	{
		if ( scene->animationsFileNames[ i ] != NULL )
		{
			UnloadModelAnimations( scene->animationsSlots[ i ].list , scene->animationsSlots[ i ].count );
			MemFree( scene->animationsFileNames[ i ] );
		}
	}

	MemFree( scene->animationsSlots );

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

void _SceneForceResizeNodeSlots( Scene3D *scene , int newSize )
{
	scene->nodeSlotsSize = newSize ;
	scene->nodeSlots = (Node3D*)MemRealloc( scene->nodeSlots , sizeof(Node3D)*newSize );
}

Node *SceneGetNewNodeSlot( Scene3D *scene )
{
	if ( scene->nodeSlotsIndex >= scene->nodeSlotsSize )
	{
		if ( scene->numberOfNewSlotsOnResize <= 0 ) return NULL ;

		_SceneForceResizeNodeSlots( scene , scene->nodeSlotsSize + scene->numberOfNewSlotsOnResize );
	}

	Node *node = &( scene->nodeSlots[ scene->nodeSlotsIndex ] );

	scene->nodeSlotsIndex++;

	return node ;
}

void _SceneForceResizeModelSlots( Scene3D *scene , int newSize )
{
	scene->modelSlotsSize = newSize ;
	scene->modelSlots = (Model*)MemRealloc( scene->modelSlots , sizeof(Model)*newSize );
	scene->modelFileNames = (char**)MemRealloc( scene->modelFileNames , sizeof(char*)*newSize );
}

Model *SceneGetNewModelSlot( Scene3D *scene )
{
	if ( scene->modelSlotsIndex >= scene->modelSlotsSize )
	{
		if ( scene->numberOfNewSlotsOnResize <= 0 ) return NULL ;

		_SceneForceResizeModelSlots( scene , scene->modelSlotsSize + scene->numberOfNewSlotsOnResize );
	}

	Model *model = &( scene->modelSlots[ scene->modelSlotsIndex ] );

	scene->modelFileNames[ scene->modelSlotsIndex ] = NULL ;

	scene->modelSlotsIndex++;

	return model ;
}

void _SceneForceResizeAnimationsSlots( Scene3D *scene , int newSize )
{
	scene->animationsSlotsSize = newSize ;
	scene->animationsSlots = (AnimationsList*)MemRealloc( scene->animationsSlots , sizeof(AnimationsList)*newSize );
	scene->animationsFileNames = (char**)MemRealloc( scene->animationsFileNames , sizeof(char*)*newSize );
}

AnimationsList *SceneGetNewAnimationsSlot( Scene3D *scene )
{
	if ( scene->animationsSlotsIndex >= scene->animationsSlotsSize )
	{
		if ( scene->numberOfNewSlotsOnResize <= 0 ) return NULL ;

		_SceneForceResizeAnimationsSlots( scene , scene->animationsSlotsSize + scene->numberOfNewSlotsOnResize );
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

int SceneLoad_getNextLineNormalized( char **buffer , size_t *bufferSize , char **line , FILE *stream )
{
	int len = getline( buffer , bufferSize , stream );

	if ( len == 0 ) return -1 ;

	*line = *buffer ;

	// Right trim :

	while( len > 0 && (*line)[len-1] >= 0 && (*line)[len-1] <= ' ' ) 
	{
		(*line)[len-1] = 0 ;
		len--;
	}

	// Left trim :

	while( (*line)[0] > 0 && (*line)[0] <= ' ' ) 
	{ 
		(*line)++ ;
		len-- ;
	}

	// Replace control characters by space :

	for( int i = 0 ; i < len ; i++ )
	{
		if ( (*line)[i] > 0 && (*line)[i] < ' ' ) (*line)[i] = ' ';
	}

	return len ;
}

bool SceneLoad_getKeyVal( char *line , char **keyBuffer , char **valBuffer , int maxLen )
{
	*keyBuffer = (char*)MemRealloc( *keyBuffer , maxLen );
	*valBuffer = (char*)MemRealloc( *valBuffer , maxLen );

	(*keyBuffer)[0] = 0 ;
	(*valBuffer)[0] = 0 ;
	
	int readPos = 0 ;
	int writePos = 0 ;

	// Copy the key name till we reach line's end, or white chars, or equal sign :
	while( ! ( line[readPos] >= 0 && line[readPos] <= ' ' ) && line[readPos] != '=' )
	{
		(*keyBuffer)[ writePos ] = line[readPos] ;
		readPos++;
		writePos++;
	}

	// Null terminate the key name string :
	(*keyBuffer)[writePos] = 0 ;

	// Skip blank chr :
	while( line[readPos] > 0 && line[readPos] <= ' ' ) readPos++;

	// Key without value :
	if ( line[readPos] != '=' ) return false ;

	readPos++;

	// Skip blank chr :
	while( line[readPos] > 0 && line[readPos] <= ' ' ) readPos++;

	// The rest of the string contains the value :
	writePos = 0 ;
	while( line[readPos] != 0 )
	{
		(*valBuffer)[ writePos ] = line[readPos] ;
		readPos++;
		writePos++;
	}

	// Null terminate the val string :
	(*valBuffer)[ writePos ] = 0 ; 

	return true ;
}

Scene3D *SceneLoad( char *fileName )
{
	FILE *fin = fopen( fileName , "rt" );
	
	if ( fin == NULL )
	{
		TRACELOG( LOG_ERROR , "SCENE: Can't open scene `%s`." , fileName );
		return NULL;
	}


	char *lineBuffer = NULL ;
	size_t lineBufferSize ;

	char *line ;
	int len ;
	int lineCounter = 0;

	char *keyBuffer = NULL ;
	char *valBuffer = NULL ;

	Scene *scene = NULL ;
	char *sceneName = NULL ;

	Model *model = NULL ;
	int modelIndex = 0 ;
	char *modelFileName = NULL ;
	
	AnimationsList *anims = NULL ;
	int animsIndex = 0 ;
	char *animsFileName = NULL ;

	Node *node = NULL ;
	int nodeIndex = 0 ;
	char *nodeName = NULL ;

	while( ( len = SceneLoad_getNextLineNormalized( &lineBuffer , &lineBufferSize , &line , fin ) ) >= 0 )
	{
		lineCounter++ ;

//		printf( "Line %d [%d] : `%s`\n" , lineCounter , len , line );

		// Parser :

		if ( line[0] == 0 ) // Empty line
		{
			continue ;
		}
		else
		if ( line[0] == ';' ) // ; Comment
		{
			continue ;
		}
		else
		if ( line[0] == '[' ) // [Section]
		{
			if ( TextBeginsWith( line , "[SCENE " ) )
			{
				if ( scene != NULL )
				{
					TRACELOG( LOG_ERROR , "SCENE: `%s`, line %d : duplicate section `[SCENE]`." , fileName , lineCounter ); 
				}

				sceneName = (char*)realloc( sceneName , len );
				sscanf( line , "[SCENE \"%[^\"]\"]" , sceneName );

				scene = SceneCreate( sceneName , 10 , 10 );
				
				node = NULL ;
				anims = NULL ;
				model = NULL ;

				continue ;
			}
			else
			if ( TextBeginsWith( line , "[MODEL " ) ) // [MODEL %d "%s"]
			{
				if ( scene == NULL )
				{
					TRACELOG( LOG_ERROR , "SCENE: `%s`, line %d : section `[SCENE]` must be the first section." , fileName , lineCounter ); 
					break;
				}

				modelFileName = (char*)realloc( modelFileName , len );
				sscanf( line , "[MODEL %d %[^]]]" , &modelIndex , modelFileName );
				
				if ( modelIndex != scene->modelSlotsIndex )
				{
					TRACELOG( LOG_ERROR , "SCENE: `%s`, line %d : invalid index order. Expecting %d." , fileName , lineCounter , scene->modelSlotsIndex );
					break;
				}

				if ( modelFileName[0] == '"' )
				{
					sscanf( line , "[MODEL %d \"%[^\"]\"]" , &modelIndex , modelFileName );
					model = SceneLoadModel( scene , modelFileName );
				}
				else
				{
					model = SceneGetNewModelSlot( scene );
				}
			}
			else
			if ( TextBeginsWith( line , "[ANIMS " ) )
			{
				if ( scene == NULL )
				{
					TRACELOG( LOG_ERROR , "SCENE: `%s`, line %d : section `[SCENE]` must be the first section." , fileName , lineCounter ); 
					break;
				}
			}
			else
			if ( TextBeginsWith( line , "[NODE " ) )
			{
				if ( scene == NULL )
				{
					TRACELOG( LOG_ERROR , "SCENE: `%s`, line %d : section `[SCENE]` must be the first section." , fileName , lineCounter ); 
					break;
				}
			}
			else
			{
				TRACELOG( LOG_WARNING , "SCENE: `%s`, line %d : Unsupported section." , fileName , lineCounter );
			}
		}
		else
		if ( TextContains( line , "=" ) ) // `key = val`
		{
			SceneLoad_getKeyVal( line , &keyBuffer , &valBuffer , len );

			char *key = keyBuffer ;
			char *val = valBuffer ;

			printf("key = `%s` ; val = `%s` ;\n" , key , val );
			
			// Are we inside the [SCENE] section ?
			if ( scene != NULL && model == NULL && anims == NULL && node == NULL )
			{
				if( TextIsEqual( key , "nodes" )
				 || TextIsEqual( key , "models" )
				 || TextIsEqual( key , "animations" )
				 || TextIsEqual( key , "anims" ) )
				{
					if ( _TextIsInteger( val ) )
					{
						int intVal = _TextToInteger( val );

						if ( intVal < 0 )
						{
							TRACELOG( LOG_WARNING , "SCENE: `%s`, line %d : a positive integer value is expected." , fileName , lineCounter );
						}

						if ( TextIsEqual( key , "nodes" ) )
						{
							_SceneForceResizeNodeSlots( scene , intVal );
						}
						else
						if ( TextIsEqual( key , "models" ) )
						{
							_SceneForceResizeModelSlots( scene , intVal );
						}
						else
						if ( TextBeginsWith( key , "anim" ) )
						{
							_SceneForceResizeAnimationsSlots( scene , intVal );
						}
					}
					else
					{
						TRACELOG( LOG_WARNING , "SCENE: `%s`, line %d : an integer value is expected." , fileName , lineCounter );
					}
				}
				else
				{
					TRACELOG( LOG_WARNING , "SCENE: `%s`, line %d : unsupported key name." , fileName , lineCounter );
				}
			}
			else // Are we inside a [MODEL] section ?
			if ( scene != NULL && model != NULL && anims == NULL && node == NULL )
			{
			}
		}
		else
		{
			TRACELOG( LOG_WARNING , "SCENE: `%s`, line %d : Unexpected error." , fileName , lineCounter );
		}

	} // wend next line

	free( keyBuffer );
	free( valBuffer );

	free( nodeName );
	free( animsFileName );
	free( modelFileName );
	free( sceneName );

	free( lineBuffer );
	fclose( fin );

	return scene ;
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

		if ( 1 )
		{ 
			Vector3 p = ExtractTranslationFromMatrix( scene->modelSlots[ i ].transform );

			fprintf( fout , "position = %f %f %f\n" , p.x , p.y , p.z );

			Vector3 s = ExtractScaleFromMatrix( scene->modelSlots[ i ].transform );

			fprintf( fout , "scale = %f %f %f\n" , s.x , s.y , s.z );
	
			Matrix r = ExtractRotationMatrixFromMatrix( scene->modelSlots[ i ].transform );

			fprintf( fout , "rotation = %f %f %f %f %f %f %f %f %f\n" ,
				r.m0 , r.m1 , r.m2 ,
				r.m4 , r.m5 , r.m6 ,
				r.m8 , r.m9 , r.m10 );
		}
		else
		{
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

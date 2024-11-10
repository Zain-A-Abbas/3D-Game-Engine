#ifndef __ENTITY_SYSTEM__
#define __ENTITY_SYSTEM__

#include "gfc_types.h"
#include "gfc_text.h"
#include "gfc_vector.h"
#include "gf3d_model.h"
#include "gfc_primitives.h"
#include "Collision.h"

typedef enum {
    NONE,
    PLAYER,
    ENEMY,
    INTERACTABLE,
    TERRAIN,
    OBJECT,
    STRUCTURE,
    PROJECTILE
} EntityType;

typedef struct EntityAnimation_S {
    char                *animFolder;
    char                currentAnimName[32];
    int                 loopAnimation;
    int                 animationFrame;
    int                 animationFrameCount;
    GFC_List            *animationList;
} EntityAnimation;

typedef struct Entity_S {
    Uint8               _in_use; // Active memory
    GFC_TextLine        name; // Entity Name
    GFC_Vector3D        position; 
    GFC_Vector3D        rotation;
    GFC_Vector3D        scale;
    Uint8               collisionLayer;
    EntityType          type;
    int                 animation;
    Model               *model; // POINTER to model
    GFC_ExtendedPrimitive      *entityCollision;
    struct Entity_S            *parent; // POINTER to parent
    struct EntityAnimation_S     *entityAnimation; // Animation manager struct
    // Behavior
    void (*think)       (struct Entity_S *self, float delta); // Called every frame on the entity
    void (*update)      (struct Entity_S *self, float delta); // Called every frame for entity state update
    void (*draw)        (struct Entity_S *self); // Custom draw code
    void (*free)        (struct Entity_S *self); // Cleans up custom data
    void                *data; // Custom entity data

} Entity;

typedef struct {
    Entity* entityList;
    Uint32 entityMax;
} EntityManager;

extern EntityManager entityManager;

/**
 * @brief Allocates blank entity for use
 * @return NULL on failure (no memory) | POINTER to entity
 */
Entity * entityNew();


void entitySystemClose();

/**
 * @brief Initialize entity manager
 * @param maxEnts Man number of entities
 */
void entitySystemInit(Uint32 maxEnts);

/**
 * @brief Draw all the entities to screen
 */
void entityDrawAll();

/**
* @brief Draws any debug info needed, such as enemy collision boxes.
*/
void entityDebugDraw(Entity* self, GFC_Matrix4 matrix);

/**
 * @brief Update all entities
 */
void entityUpdateAll(float delta);

/**
 * @brief Have all entities think
 */
void entityThinkAll(float delta);

/**
 * @brief Return memory of previously allocated entity back to pool
 * @param self POINTER to entity
 */
void _entityFree(Entity *self);

/**
* @brief Return whether or not the following raycast intersected an entity or not
*/
int entityRaycastTest(Entity * entity, GFC_Edge3D raycast, GFC_Vector3D *contact, GFC_Triangle3D * t, GFC_Box *boundingBox);

/**
* @brief Return whether or not the following capsule intersected with an entity or not
*/
Uint8 entityCapsuleTest(Entity* entity, GFC_Capsule c, GFC_Vector3D* intersectionPoint, GFC_Vector3D* penetrationNormal, float* penetrationDepth, GFC_Box* boundingBox);
/**
* @brief Check if the entity is on a collision layer or not
*/
int isOnLayer(Entity* self, int layer);

/**
* @brief Get the entity's transforms, in reference to the parent's.
*/
GFC_Vector3D entityGlobalPosition(Entity* self);
GFC_Vector3D entityGlobalRotation(Entity* self);
GFC_Vector3D entityGlobalScale(Entity* self);

/**
* @brief Assigns the entityanimation the folder to pull assets from
*/
void animationSetup(Entity* self, const char* animFolder, char* animations[], int animationCount);

/**
* @brief Frees the animation handler
*/
void animationFree(Entity* self);

/**
* @brief Sets the entity's model to the .model file in the folder that matches the name given
*/
void animationPlay(Entity* self, const char* animName);


/**
* @brief This function is called on an entity when it is attacked
*/
void entityAttacked(Entity *self, int damage);


#endif
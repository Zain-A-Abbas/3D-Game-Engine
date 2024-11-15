#include <SDL.h>            

#include "simple_json.h"
#include "simple_logger.h"

#include "gfc_input.h"
#include "gfc_config_def.h"
#include "gfc_vector.h"
#include "gfc_matrix.h"
#include "gfc_audio.h"
#include "gfc_string.h"
#include "gfc_actions.h"

#include "gf2d_sprite.h"
#include "gf2d_font.h"
#include "gf2d_draw.h"
#include "gf2d_actor.h"
#include "gf2d_mouse.h"

#include "gf3d_vgraphics.h"
#include "gf3d_pipeline.h"
#include "gf3d_swapchain.h"
#include "gf3d_model.h"
#include "gf3d_camera.h"
#include "gf3d_texture.h"
#include "gf3d_draw.h"
#include "gf3d_armature.h"
#include "Entity.h"
#include "TerrainManager.h"
#include "Player.h"
#include "Enemy.h"
#include "UI.h"
#include "Interactable.h"
#include "Structure.h"
#include "Zombie.h"

extern int __DEBUG;

static int _done = 0;
static Uint32 frame_delay = 16;
static float fps = 0;

void parse_arguments(int argc,char *argv[]);
void game_frame_delay(float* delta);

void exitGame()
{
    _done = 1;
}

void draw_origin()
{
    gf3d_draw_edge_3d(
        gfc_edge3d_from_vectors(gfc_vector3d(-100,0,0),gfc_vector3d(100,0,0)),
        gfc_vector3d(0,0,0),
        gfc_vector3d(0,0,0),gfc_vector3d(1,1,1),0.1,gfc_color(1,0,0,1));
    gf3d_draw_edge_3d(
        gfc_edge3d_from_vectors(gfc_vector3d(0,-100,0),gfc_vector3d(0,100,0)),
        gfc_vector3d(0,0,0),gfc_vector3d(0,0,0),gfc_vector3d(1,1,1),0.1,gfc_color(0,1,0,1));
    gf3d_draw_edge_3d(
        gfc_edge3d_from_vectors(gfc_vector3d(0,0,-100),gfc_vector3d(0,0,100)),
        gfc_vector3d(0,0,0),gfc_vector3d(0,0,0),gfc_vector3d(1,1,1),0.1,gfc_color(0,0,1,1));
}


int main(int argc,char *argv[])
{
    //local variables
    Model *sky;
    GFC_Matrix4 skyMat,dinoMat;
    //initializtion    
    parse_arguments(argc,argv);
    init_logger("gf3d.log",0);
    slog("gf3d begin");
    //gfc init
    gfc_input_init("config/input.cfg");
    gfc_config_def_init();
    gfc_action_init(1024);
    //gf3d init
    gf3d_vgraphics_init("config/setup.cfg");
    gf3d_materials_init();
    gf2d_font_init("config/font.cfg");
    gf2d_actor_init(1000);
    
    gf3d_draw_init();//3D
    gf2d_draw_manager_init(1000);//2D
    
    //game init
    srand(SDL_GetTicks());
    slog_sync();

    //game setup

    //armatures
    gf3d_armature_system_init(256);


    sky = gf3d_model_load("models/sky.model");
    gfc_matrix4_identity(skyMat);
    gfc_matrix4_identity(dinoMat);

    //camera
    gf3d_camera_set_scale(gfc_vector3d(1,1,1));
    gf3d_camera_set_position(gfc_vector3d(15,-15,10));
    gf3d_camera_look_at(gfc_vector3d(0,0,0),NULL);
    gf3d_camera_set_move_step(0.2);
    gf3d_camera_set_rotate_step(0.05);
    
    gf3d_camera_enable_free_look(1);
    entitySystemInit(2048);

    // Setup audio
    gfc_audio_init(128, 1, 1, 1, 0, 0);
    
    // Create player
    Entity * player = createPlayer();
    assignCamera(player, gf3dGetCamera());
    player->position.z = 0;
    
    SDL_SetRelativeMouseMode(SDL_TRUE);

    // UI setup
    initializeUI();

    // Create dummy enemies
    Entity* enemy1 = createZombie(player);
    enemy1->position = gfc_vector3d(0, -40, 0);
    //entityScalePreserveModel(enemy1, gfc_vector3d(0.08, 0.08, 0.08));
    /*Entity* enemy2 = enemyEntityNew();
    enemy2->position = gfc_vector3d(-4, 4, 0);*/
    
    // Create land
    Entity* testGround = terrainEntityNew();
    //testGround->model = gf3d_model_load("models/primitives/testground.model");
    testGround->model = gf3d_model_load("models/structures/Ground1.model");
    //testGround->scale = gfc_vector3d(4, 4, 1);
    testGround->position = gfc_vector3d(0, 0, -8);
    EntityCollision* groundCollision = (EntityCollision*)malloc(sizeof(EntityCollision));
    memset(groundCollision, 0, sizeof(EntityCollision));
    testGround->entityCollision = groundCollision;
    GFC_Box testGroundbox = gfc_box(-375, -375, -20, 750, 750, 40);
    newQuadTree(testGround, testGroundbox, 4);

    // Create house
    Entity* testHouse = structureNew(HOUSE);
    testHouse->position = gfc_vector3d(0, 64, 2);
    testHouse->scale = gfc_vector3d(1, 1, 1);


    // Create Tree
    TerrainData*treeData;
    int treeCount = 160 + gfc_random_int(40);
    for (int i = 0; i < treeCount; i++) {
        float treeX = -375 + gfc_random_int(750);
        float treeY = -375 + gfc_random_int(750);
        float treeZ = -16 + gfc_random_int(4);
        Entity* testTree = terrainEntityNew();
        testTree->model = gf3d_model_load("models/structures/Tree.model");
        testTree->position = gfc_vector3d(treeX, treeY, treeZ);
        testTree->rotation.z = gfc_random() * GFC_2PI;
        testTree->collisionLayer = 0b00000100;
        treeData = (TerrainData*)testTree->data;
        
        EntityCollision* treeCollision = (EntityCollision*)malloc(sizeof(EntityCollision));
        memset(treeCollision, 0, sizeof(EntityCollision));

        GFC_ExtendedPrimitive* primitive = (GFC_ExtendedPrimitive*)malloc(sizeof(GFC_ExtendedPrimitive));
        memset(primitive, 0, sizeof(GFC_ExtendedPrimitive));
        primitive->type = E_Capsule;
        GFC_Capsule treeCapsule = gfc_capsule(16, 4);
                primitive->s.c = treeCapsule;
        treeCollision->collisionPrimitive = primitive;

        GFC_Box boundingBox = { 0 };
        boundingBox.x = -2; boundingBox.y = -2; boundingBox.z = -2;
        boundingBox.w = 4; boundingBox.d = 4; boundingBox.h = 4;
        treeCollision->AABB = boundingBox;

        testTree->entityCollision = treeCollision;
        //printf("\nTree location: %f, %f, %f", treeX, treeY, treeZ);
    }



    // Create interactable
    //Entity* testInteractable = interactableNew(SPINNING_BOX, gfc_vector3d(0, 0, 0));
    //testInteractable->position = gfc_vector3d(0, -24, 0);
    //testInteractable->scale = gfc_vector3d(2, 2, 2);
   // testInteractable->rotation.z = 0.707;


    //Delta time
    float delta = 0.0;
    game_frame_delay(&delta);


    // main game loop
    while(!_done)
    {
        gfc_input_update();
        gf2d_mouse_update();
        gf2d_font_update();
        entityThinkAll(delta);
        entityUpdateAll(delta);
        //camera updaes
        //gf3d_camera_controls_update();
        gf3d_camera_update_view();
        gf3d_camera_get_view_mat4(gf3d_vgraphics_get_view_matrix());

        gf3d_vgraphics_render_start();

            //3D draws
        


            gf3d_model_draw_sky(sky,skyMat,GFC_COLOR_WHITE);
            entityDrawAll();
            draw_origin();

                // Draw last player raycast
              /*PlayerData* playerData = getPlayerData(player);
                if (playerData != NULL) {
                    if (playerData->raycastTests) {
                        int i = 0;
                        for (i = 0; i < playerData->raycastTests->count; i++) {
                            GFC_Edge3D drawEdge;
                            GFC_Edge3D* edgeptr = (GFC_Edge3D*)gfc_list_get_nth(playerData->raycastTests, i);
                            drawEdge.a = edgeptr->a;
                            drawEdge.b = edgeptr->b;
                            gf3d_draw_edge_3d(
                                drawEdge,
                                gfc_vector3d(0, 0, 0),
                                gfc_vector3d(0, 0, 0),
                                gfc_vector3d(1, 1, 1),
                                0.25,
                                gfc_color(1.0, 1.0, 0.0, 1.0)
                            );


                        }
                    }


                    //if (playerData->boundingBoxTest.x != 0) {
                      //  gf3d_draw_cube_solid(playerData->boundingBoxTest, gfc_vector3d(0, 0, 0), gfc_vector3d(0, 0, 0), gfc_vector3d(1, 1, 1), gfc_color(0.5, 0.2, 0.2, 0.8));
                    //}
                }*/

            for (int i = 0; i < gfc_list_get_count(testGround->entityCollision->quadTree->leaves); i++) {
                QuadtreeNode* currentLeaf = (QuadtreeNode*)gfc_list_get_nth(testGround->entityCollision->quadTree->leaves, i);
                drawBoundingBox(currentLeaf->AABB, gfc_color(0.3, 0.3, 0.3, 0.4), 1);
            }
            //GFC_Box testBox = gfc_box(-10, -5, -10, 20, 10, 20);
            //drawBoundingBox(testBox);
            drawBoundingBox(player->entityCollision->AABB, gfc_color(0.1, 0.3, 0.3, 0.5), 0);

            //if (gfc_box_overlap(testBox, player->entityCollision->AABB)) {
            //    printf("klasjsd");
            //}
            //2D draws
                //gf2d_mouse_draw();
                drawUI();

                gf2d_font_draw_line_tag("ALT+F4 to exit",FT_H1,GFC_COLOR_WHITE, gfc_vector2d(10,10));
        gf3d_vgraphics_render_end();


        if (gfc_input_command_down("exit"))_done = 1; // exit condition
        game_frame_delay(&delta);
    }    
    vkDeviceWaitIdle(gf3d_vgraphics_get_default_logical_device());    
    //cleanup
    slog("\ngf3d program end");
    exit(0);
    slog_sync();
    return 0;
}

void parse_arguments(int argc,char *argv[])
{
    int a;

    for (a = 1; a < argc;a++)
    {
        if (strcmp(argv[a],"--debug") == 0)
        {
            __DEBUG = 1;
        }
    }    
}

void game_frame_delay(float * delta)
{
    Uint32 diff;
    static Uint32 now;
    static Uint32 then;
    then = now;
    slog_sync();// make sure logs get written when we have time to write it
    now = SDL_GetTicks();
    diff = (now - then);
    *delta = (float)diff / 1000;
    if (diff < frame_delay)
    {
        SDL_Delay(frame_delay - diff);
    }
    fps = 1000.0/MAX(SDL_GetTicks() - then,0.001);
    //slog("fps: %f", fps);
    //slog("Delta: %f", *delta);
}
/*eol@eof*/

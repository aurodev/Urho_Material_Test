#include "gs_main_menu.h"

using namespace Urho3D;
using namespace std;

gs_main_menu::gs_main_menu() : game_state()
{
    Node* node_camera=globals::instance()->camera->GetNode();
    node_camera->SetPosition(Vector3(0,0,0));
    node_camera->SetDirection(Vector3::FORWARD);

    // create a transparent window with some text to display things like help and FPS
    {
        window=new Window(context_);
        gui_elements.push_back(window);
        GetSubsystem<UI>()->GetRoot()->AddChild(window);
        window->SetStyle("Window");
        window->SetSize(600,70);
        window->SetColor(Color(.0,.15,.3,.5));
        window->SetAlignment(HA_LEFT,VA_TOP);

        window_text=new Text(context_);
        window_text->SetFont(globals::instance()->cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"),14);
        window_text->SetColor(Color(.8,.85,.9));
        window_text->SetAlignment(HA_LEFT,VA_TOP);
        window->AddChild(window_text);
    }

    // a rotating flag
    {
        node_rotating_flag=globals::instance()->scene->CreateChild("Flag");
        nodes.push_back(node_rotating_flag);
        node_rotating_flag->SetPosition(Vector3(100,-0.5,6));
        StaticModel* boxObject=node_rotating_flag->CreateComponent<StaticModel>();
        boxObject->SetModel(globals::instance()->cache->GetResource<Model>("Models/flag.mdl"));
        boxObject->SetMaterial(0,globals::instance()->cache->GetResource<Material>("Materials/flag_pole.xml"));
        boxObject->SetMaterial(1,globals::instance()->cache->GetResource<Material>("Materials/flag_cloth.xml"));
        boxObject->SetCastShadows(true);

        ParticleEmitter* emitter=node_rotating_flag->CreateComponent<ParticleEmitter>();
        emitter->SetEffect(globals::instance()->cache->GetResource<ParticleEffect>("Particle/flag.xml"));
    }

    // skybox
    {
        Node* skyNode=globals::instance()->scene->CreateChild("Sky");
        nodes.push_back(skyNode);
        skyNode->SetScale(1500.0f);
        Skybox* skybox=skyNode->CreateComponent<Skybox>();
        skybox->SetModel(globals::instance()->cache->GetResource<Model>("Models/Box.mdl"));
        skybox->SetMaterial(globals::instance()->cache->GetResource<Material>("Materials/Skybox.xml"));
    }

    // a torch with a light, sound and particle effects
    {
        Node* node=globals::instance()->scene->CreateChild("Light");
        nodes.push_back(node);
        Vector3 pos(Vector3(3,-0.5,6));
        node->SetPosition(pos);

        StaticModel* boxObject=node->CreateComponent<StaticModel>();
        set_model(boxObject,globals::instance()->cache,"Data/Models/torch");
        boxObject->SetCastShadows(true);
        boxObject->SetOccludee(true);
        boxObject->SetShadowDistance(200);
        boxObject->SetDrawDistance(200);

        auto lightNode=node->CreateChild();
        lightNode->Translate(Vector3(0,2,0));
        Light* light=lightNode->CreateComponent<Light>();
        light->SetLightType(LIGHT_POINT);
        light->SetRange(50);
        light->SetBrightness(1.2);
        light->SetColor(Color(1.0,0.6,0.3,1.0));
        light->SetCastShadows(true);
        light->SetShadowDistance(200);
        light->SetDrawDistance(200);

        auto n_particle=node->CreateChild();
        n_particle->Translate(Vector3(0,1.6,0));
        ParticleEmitter* emitter=n_particle->CreateComponent<ParticleEmitter>();
        emitter->SetEffect(globals::instance()->cache->GetResource<ParticleEffect>("Particle/torch_fire.xml"));
        emitter=n_particle->CreateComponent<ParticleEmitter>();
        emitter->SetEffect(globals::instance()->cache->GetResource<ParticleEffect>("Particle/torch_smoke.xml"));

        auto sound_torch=globals::instance()->cache->GetResource<Sound>("Sounds/torch.ogg");
        sound_torch->SetLooped(true);
        auto sound_torch_source=n_particle->CreateComponent<SoundSource3D>();
        sound_torch_source->SetNearDistance(1);
        sound_torch_source->SetFarDistance(50);
        sound_torch_source->SetSoundType(SOUND_EFFECT);
        sound_torch_source->Play(sound_torch);
    }

    // grid of 400 cubes, known from the basic sample application at the Urho Wiki
    if(false)
    for(int x=-30;x<30;x+=3)
        for(int y=-30;y<30;y+=3)
        {
            Node* boxNode_=globals::instance()->scene->CreateChild("Box");
            nodes.push_back(boxNode_);
            boxNode_->SetPosition(Vector3(x,-1,y));
            boxNode_->SetScale(Vector3(3,3,3));
            StaticModel* boxObject=boxNode_->CreateComponent<StaticModel>();
            boxObject->SetModel(globals::instance()->cache->GetResource<Model>("Models/Box.mdl"));
            boxObject->SetMaterial(globals::instance()->cache->GetResource<Material>("Materials/stones_quad.xml"));
            //boxObject->SetCastShadows(true);
        }

    // sun
    {
        Node* lightNode=globals::instance()->scene->CreateChild("Light");
        nodes.push_back(lightNode);
        Light* light=lightNode->CreateComponent<Light>();
        light->SetLightType(LIGHT_DIRECTIONAL);
        light->SetCastShadows(true);
        light->SetShadowBias(BiasParameters(0.00000025f,0.1f));
        light->SetShadowCascade(CascadeParameters(20.0f,60.0f,180.0f,560.0f,100.0f,100.0f));
        light->SetShadowResolution(1.0);
        light->SetBrightness(1.0);
        light->SetColor(Color(1.0,0.6,0.3,1));
        lightNode->SetDirection(Vector3::FORWARD);
        lightNode->Yaw(-150);   // horizontal
        lightNode->Pitch(30);   // vertical
        lightNode->Translate(Vector3(0,0,-20000));

        BillboardSet* billboardObject=lightNode->CreateComponent<BillboardSet>();
        billboardObject->SetNumBillboards(1);
        billboardObject->SetMaterial(globals::instance()->cache->GetResource<Material>("Materials/sun.xml"));
        billboardObject->SetSorted(true);
        Billboard* bb=billboardObject->GetBillboard(0);
        bb->size_=Vector2(10000,10000);
        bb->rotation_=Random()*360.0f;
        bb->enabled_=true;
        billboardObject->Commit();
    }

    {
        Node* terrainNode=globals::instance()->scene->CreateChild("Terrain");
        terrainNode->SetPosition(Vector3(3.0f,-0.4f));
        terrain=terrainNode->CreateComponent<Terrain>();
        terrain->SetPatchSize(128);
        terrain->SetSpacing(Vector3(2,0.5,2));
        terrain->SetSmoothing(true);
        terrain->SetHeightMap(globals::instance()->cache->GetResource<Image>("Textures/HeightMap.png"));
        terrain->SetMaterial(globals::instance()->cache->GetResource<Material>("Materials/Terrain_quad_dsnh.xml"));
        terrain->SetCastShadows(true);
        terrain->SetOccluder(true);
    }

    SubscribeToEvent(E_UPDATE,HANDLER(gs_main_menu,update));
    SubscribeToEvent(E_KEYDOWN,HANDLER(gs_main_menu,HandleKeyDown));
}

void gs_main_menu::update(StringHash eventType,VariantMap& eventData)
{
    float timeStep=eventData[Update::P_TIMESTEP].GetFloat();




    static double last_second=0;
    static double last_second_frames=1;
    static timer this_second;
    static double this_second_frames=0;
    this_second_frames++;
    if(this_second.until_now()>=1)
    {
        last_second=this_second.until_now();
        last_second_frames=this_second_frames;
        this_second.reset();
        this_second_frames=0;
    }

    std::string str="WASD, mouse and shift to move. T to toggle fill mode,\nG to toggle GUI, Tab to toggle mouse mode, Esc to quit.\n";
    if(last_second!=0)
    {
        str.append(std::to_string(last_second_frames/last_second).substr(0,6));
        str.append(" FPS ");
    }
    //    cout<<std::to_string(last_second_frames/last_second)<<endl;
    String s(str.c_str(),str.size());
    window_text->SetText(s);




    node_rotating_flag->Rotate(Quaternion(0,64*timeStep,0));

    // Movement speed as world units per second
    float MOVE_SPEED=10.0f;
    // Mouse sensitivity as degrees per pixel
    const float MOUSE_SENSITIVITY=0.1f;

    // camera movement
    Input* input=GetSubsystem<Input>();
    Node* cameraNode_=globals::instance()->camera->GetNode();
    if(input->GetQualifierDown(1))  // 1 is shift, 2 is ctrl, 4 is alt
        MOVE_SPEED*=10;
    if(input->GetKeyDown('W'))
        cameraNode_->Translate(Vector3(0,0, 1)*MOVE_SPEED*timeStep);
    if(input->GetKeyDown('S'))
        cameraNode_->Translate(Vector3(0,0,-1)*MOVE_SPEED*timeStep);
    if(input->GetKeyDown('A'))
        cameraNode_->Translate(Vector3(-1,0,0)*MOVE_SPEED*timeStep);
    if(input->GetKeyDown('D'))
        cameraNode_->Translate(Vector3( 1,0,0)*MOVE_SPEED*timeStep);

    if(!GetSubsystem<Input>()->IsMouseVisible())
    {
        IntVector2 mouseMove=input->GetMouseMove();
        if(mouseMove.x_>-2000000000&&mouseMove.y_>-2000000000)
        {
            static float yaw_=0;
            static float pitch_=0;
            yaw_+=MOUSE_SENSITIVITY*mouseMove.x_;
            pitch_+=MOUSE_SENSITIVITY*mouseMove.y_;
            pitch_=Clamp(pitch_,-90.0f,90.0f);
            // Reset rotation and set yaw and pitch again
            cameraNode_->SetDirection(Vector3::FORWARD);
            cameraNode_->Yaw(yaw_);
            cameraNode_->Pitch(pitch_);
        }
    }
}

void gs_main_menu::HandleKeyDown(StringHash eventType,VariantMap& eventData)
{
    using namespace KeyDown;
    int key=eventData[P_KEY].GetInt();
    if(key==KEY_ESC)
        globals::instance()->engine->Exit();
    else if(key==KEY_G)
        window->SetVisible(!window->IsVisible());
    else if(key==KEY_T)
        globals::instance()->camera->SetFillMode(globals::instance()->camera->GetFillMode()==FILL_WIREFRAME?FILL_SOLID:FILL_WIREFRAME);
}
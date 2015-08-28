
#include<SDL2/SDL.h>
#include<SDL2/SDL_image.h>
#include<iostream>
#include<map>
#include<SDL2/SDL_mixer.h>
#include<vector>

#include "TinyXml/tinyxml.h"

using namespace std;

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Event Event;
SDL_Texture *background, *texture_npc, *texture_tile, *begining, *character, *character2, *star, *gameover, *winner;
SDL_Rect rect_background, rect_character, rect_npc, rect_tile, rect_tileset, rect_character2, rect_star;
bool starobtn=false, lose=false, win=false;
int pts1=0, pts2=0;

class Warp
{
public:
    int x;
    int y;
    string mapa;
    SDL_Rect rect;
    Warp(int x,int y,string mapa,SDL_Rect rect)
    {
        this->x=x;
        this->y=y;
        this->mapa=mapa;
        this->rect = rect;
    }
};

bool collision(SDL_Rect r1, SDL_Rect r2)
{
    if(r1.x+r1.w <= r2.x)
        return false;
    if(r1.x >= r2.x+r2.w)
        return false;
    if(r1.y+r1.h<=r2.y)
        return false;
    if(r1.y >= r2.y+r2.h)
        return false;

    return true;
}

vector<Warp*> getWarps(string archivo)
{
    TiXmlDocument doc(archivo.c_str());
    bool loadOkay = doc.LoadFile();
    TiXmlElement *map_node = doc.FirstChild("map")->ToElement();
    TiXmlNode*objectgroup_node = map_node->FirstChild("objectgroup");

    vector<Warp*> respuesta;

    if(objectgroup_node==NULL)
        return respuesta;

    for(TiXmlNode*object_node = objectgroup_node->FirstChild("object");
        object_node!=NULL;
        object_node=object_node->NextSibling("object"))
    {
        int x,y;
        string mapa;
        SDL_Rect rect;
        rect.x = atoi(object_node->ToElement()->Attribute("x"));
        rect.y = atoi(object_node->ToElement()->Attribute("y"));
        rect.w = atoi(object_node->ToElement()->Attribute("width"));
        rect.h = atoi(object_node->ToElement()->Attribute("height"));

        TiXmlNode*properties = object_node->FirstChild("properties");
        for(TiXmlNode*property = properties->FirstChild("property");
            property!=NULL;
            property=property->NextSibling("property"))
        {
            if(strcmp(property->ToElement()->Attribute("name"),"x")==0)
            {
                cout<<"X:"<<property->ToElement()->Attribute("value")<<endl;
                x=atoi(property->ToElement()->Attribute("value"));
            }
            if(strcmp(property->ToElement()->Attribute("name"),"y")==0)
            {
                y=atoi(property->ToElement()->Attribute("value"));
            }
            if(strcmp(property->ToElement()->Attribute("name"),"mapa")==0)
            {
                mapa=property->ToElement()->Attribute("value");
            }
        }
        Warp*warp = new Warp(x,y,mapa,rect);
        cout<<warp->x<<endl;
        cout<<warp->y<<endl;
        cout<<warp->mapa<<endl;
        cout<<warp->rect.x<<endl;
        cout<<warp->rect.y<<endl;
        cout<<warp->rect.w<<endl;
        cout<<warp->rect.h<<endl;
        respuesta.push_back(warp);
    }

    return respuesta;
}

vector<int> getMapa(string archivo,int layer)
{
    vector<int> mapa;
    TiXmlDocument doc(archivo.c_str());
    bool loadOkay = doc.LoadFile();
    TiXmlElement *map_node = doc.FirstChild("map")->ToElement();
    TiXmlNode*layer_node_temp = map_node->FirstChild("layer");
    for(int i=1;i<layer;i++)
        layer_node_temp=layer_node_temp->NextSibling("layer");

    TiXmlElement *layer_node = layer_node_temp->ToElement();

    for(TiXmlNode *tile_node = layer_node->FirstChild("data")->FirstChild("tile");
        tile_node!=NULL;
        tile_node=tile_node->NextSibling("tile"))
    {
        mapa.push_back(atoi(tile_node->ToElement()->Attribute("gid")));
    }
    return mapa;
}

void dibujarLayer(SDL_Renderer* renderer,vector<int>mapa)
{
    int x_pantalla = 0;
    int y_pantalla = 0;
    for(int i=0;i<mapa.size();i++)
    {
        int x = 0;
        int y = 0;
        for(int acum = 1;acum<mapa[i];acum++)
        {
            x+=32;
            if(acum%16==0)
            {
                y+=32;
                x=0;
            }
        }

    //            rect_tile.x = 32*(mapa[i]%16-1);
    //            rect_tile.y = 32*(mapa[i]/16);
        rect_tile.x = x;
        rect_tile.y = y;
        rect_tile.w = 32;
        rect_tile.h = 32;

        //cout<<rect_tile.x<<","<<rect_tile.y<<endl;

        rect_tileset.x = x_pantalla;
        rect_tileset.y = y_pantalla;

        if(mapa[i]!=0)
            SDL_RenderCopy(renderer, texture_tile, &rect_tile, &rect_tileset);

        x_pantalla+=32;
        if(x_pantalla>=320)
        {
            x_pantalla=0;
            y_pantalla+=32;
        }
    }
}

bool collisionLayer(vector<int>collision_map,SDL_Rect rect_personaje)
{
//    rect_personaje.x+=1;
//    rect_personaje.y+=1;
//    rect_personaje.w-=2;
//    rect_personaje.h-=2;

    int x_pantalla = 0;
    int y_pantalla = 0;
    for(int i=0;i<collision_map.size();i++)
    {
        int x = 0;
        int y = 0;
        for(int acum = 1;acum<collision_map[i];acum++)
        {
            x+=32;
            if(acum%16==0)
            {
                y+=32;
                x=0;
            }
        }

    //            rect_tile.x = 32*(mapa[i]%16-1);
    //            rect_tile.y = 32*(mapa[i]/16);
        rect_tile.x = x;
        rect_tile.y = y;
        rect_tile.w = 32;
        rect_tile.h = 32;

        //cout<<rect_tile.x<<","<<rect_tile.y<<endl;

        rect_tileset.x = x_pantalla;
        rect_tileset.y = y_pantalla;
        rect_tileset.w = 32;
        rect_tileset.h = 32;

        if(collision_map[i]!=0)
        {
            if(collision(rect_personaje,rect_tileset))
            {
                return true;
            }
        }

        x_pantalla+=32;
        if(x_pantalla>=320)
        {
            x_pantalla=0;
            y_pantalla+=32;
        }
    }
    return false;
}

int main( int argc, char* args[] )
{
    //Init SDL
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        return 10;
    }
    //Creates a SDL Window
    if((window = SDL_CreateWindow("Image Loading", 100, 100, 320/*WIDTH*/, 320/*HEIGHT*/, SDL_WINDOW_RESIZABLE | SDL_RENDERER_PRESENTVSYNC)) == NULL)
    {
        return 20;
    }
    //SDL Renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED );
    if (renderer == NULL)
    {
        std::cout << SDL_GetError() << std::endl;
        return 30;
    }

    //Init textures
    int w=0,h=0;
    background = IMG_LoadTexture(renderer,"fondo.png");
    begining = IMG_LoadTexture(renderer,"splash.png");
    gameover =IMG_LoadTexture(renderer,"go.jpg");
    winner = IMG_LoadTexture(renderer,"win.jpg");
    SDL_QueryTexture(background, NULL, NULL, &w, &h);
    rect_background.x = 0; rect_background.y = 0; rect_background.w = 320; rect_background.h = 320;

    star= IMG_LoadTexture(renderer, "star.png");
    SDL_QueryTexture(star, NULL, NULL, &w, &h);
    rect_star.x = rand()%100 +100; rect_star.y = rand()%100+100; rect_star.w = w; rect_star.h = h;

    character = IMG_LoadTexture(renderer, "personaje/down1.png");
    SDL_QueryTexture(character, NULL, NULL, &w, &h);
    rect_character.x = 0; rect_character.y = 100; rect_character.w = w; rect_character.h = h;

    character2 = IMG_LoadTexture(renderer, "charizard/cha2r.png");
    SDL_QueryTexture(character, NULL, NULL, &w, &h);
    rect_character2.x = 200; rect_character2.y = 100; rect_character2.w = w; rect_character2.h = h;

    char orientation = 'd';// d u l r
    char ori2='d';
    int current_sprite = 0;
    int animation_velocity = 15;
    int velocity = 1;
    int velo=1, cont=1;
    int frame = 0;
    map<char,vector<SDL_Texture*> >charizard;
    charizard['u'].push_back(IMG_LoadTexture(renderer, "charizard/chau.png"));
    charizard['u'].push_back(IMG_LoadTexture(renderer, "charizard/cha1u.png"));
    charizard['u'].push_back(IMG_LoadTexture(renderer, "charizard/cha2u.png"));
    charizard['d'].push_back(IMG_LoadTexture(renderer, "charizard/cha.png"));
    charizard['d'].push_back(IMG_LoadTexture(renderer, "charizard/cha1d.png"));
    charizard['d'].push_back(IMG_LoadTexture(renderer, "charizard/cha2d.png"));
    charizard['l'].push_back(IMG_LoadTexture(renderer, "charizard/chal.png"));
    charizard['l'].push_back(IMG_LoadTexture(renderer, "charizard/cha1l.png"));
    charizard['l'].push_back(IMG_LoadTexture(renderer, "charizard/cha2l.png"));
    charizard['r'].push_back(IMG_LoadTexture(renderer, "charizard/char.png"));
    charizard['r'].push_back(IMG_LoadTexture(renderer, "charizard/cha1r.png"));
    charizard['r'].push_back(IMG_LoadTexture(renderer, "charizard/cha2r.png"));

     map<char,vector<SDL_Texture*> >sprites;
    sprites['u'].push_back(IMG_LoadTexture(renderer, "personaje/up1.png"));
    sprites['u'].push_back(IMG_LoadTexture(renderer, "personaje/up2.png"));
    sprites['d'].push_back(IMG_LoadTexture(renderer, "personaje/down1.png"));
    sprites['d'].push_back(IMG_LoadTexture(renderer, "personaje/down2.png"));
    sprites['l'].push_back(IMG_LoadTexture(renderer, "personaje/left1.png"));
    sprites['l'].push_back(IMG_LoadTexture(renderer, "personaje/left2.png"));
    sprites['r'].push_back(IMG_LoadTexture(renderer, "personaje/right1.png"));
    sprites['r'].push_back(IMG_LoadTexture(renderer, "personaje/right2.png"));

    SDL_QueryTexture(sprites['u'][0], NULL, NULL, &w, &h);
    rect_character.x = 100;
    rect_character.y = 100;
    rect_character.w = w;
    rect_character.h = h;

    texture_npc = IMG_LoadTexture(renderer,"npc.png");
    SDL_QueryTexture(texture_npc, NULL, NULL, &w, &h);
    rect_npc.x = 300;
    rect_npc.y = 100;
    rect_npc.w = w;
    rect_npc.h = h;

    texture_tile = IMG_LoadTexture(renderer,"tile/crypt.png");
    rect_tile.x = 32*4;
    rect_tile.y = 32*5;
    rect_tile.w = 32;
    rect_tile.h = 32;

    SDL_QueryTexture(texture_npc, NULL, NULL, &w, &h);
    rect_tileset.x = 0;
    rect_tileset.y = 0;
    rect_tileset.w = w;
    rect_tileset.h = h;

    map<string,vector<int> >mapas_down;
    map<string,vector<int> >mapas_over;
    map<string,vector<int> >mapas_collision;
    map<string,vector<Warp*> >warps;

    mapas_down["mapa1"]=getMapa("tile/test.tmx",1);
    mapas_over["mapa1"]=getMapa("tile/test.tmx",2);
    mapas_collision["mapa1"]=getMapa("tile/test.tmx",3);
    warps["mapa1"] = getWarps("tile/test.tmx");

    mapas_down["mapa2"]=getMapa("tile/test2.tmx",1);
    mapas_over["mapa2"]=getMapa("tile/test2.tmx",2);
    mapas_collision["mapa2"]=getMapa("tile/test2.tmx",3);
    warps["mapa2"] = getWarps("tile/test2.tmx");

    mapas_down["mapa3"]=getMapa("tile/test3.tmx",1);
    mapas_over["mapa3"]=getMapa("tile/test3.tmx",2);
    mapas_collision["mapa3"]=getMapa("tile/test3.tmx",3);
    warps["mapa3"] = getWarps("tile/test3.tmx");

    mapas_down["mapa4"]=getMapa("tile/test4.tmx",1);
    mapas_over["mapa4"]=getMapa("tile/test4.tmx",2);
    mapas_collision["mapa4"]=getMapa("tile/test4.tmx",3);
    warps["mapa4"] = getWarps("tile/test4.tmx");

    mapas_down["mapa5"]=getMapa("tile/test5.tmx",1);
    mapas_over["mapa5"]=getMapa("tile/test5.tmx",2);
    mapas_collision["mapa5"]=getMapa("tile/test5.tmx",3);
    warps["mapa5"] = getWarps("tile/test5.tmx");

    mapas_down["mapa6"]=getMapa("tile/test6.tmx",1);
    mapas_over["mapa6"]=getMapa("tile/test6.tmx",2);
    mapas_collision["mapa6"]=getMapa("tile/test6.tmx",3);
    warps["mapa6"] = getWarps("tile/test6.tmx");


    string mapa_actual="mapa1";

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2,2048);

    Mix_Music *cancion= Mix_LoadMUS("The Last of Us.mp3");
    Mix_Chunk *noti= Mix_LoadWAV("noti.wav");
    Mix_Chunk *str= Mix_LoadWAV("illu.wav");
    bool ini=false;

    //Main Loop
    while(true)
    {
        while(SDL_PollEvent(&Event))
        {
            if(Event.type == SDL_QUIT)
            {
                return 0;
            }
        }

        if(!Mix_PlayingMusic())
            Mix_PlayMusic(cancion,2);

        const Uint8* currentKeyStates = SDL_GetKeyboardState( NULL );

        if(currentKeyStates[ SDL_SCANCODE_L ])
        {
            rect_character2.x+=velocity;
             while(collision(rect_character2,rect_npc))
            {
                rect_character2.x-=1;
            }
            while(collisionLayer(mapas_collision[mapa_actual],rect_character2))
            {
                rect_character2.x-=1;
            }
            orientation='r';
        }
        if(currentKeyStates[ SDL_SCANCODE_J ])
        {
            rect_character2.x-=velocity;
            while(collision(rect_character2,rect_npc))
            {
                rect_character2.x+=1;
            }
            while(collisionLayer(mapas_collision[mapa_actual],rect_character2))
            {
                rect_character2.x+=1;
            }
            orientation='l';
        }
        if(currentKeyStates[ SDL_SCANCODE_K ])
        {
            rect_character2.y+=velocity;
            while(collision(rect_character2,rect_npc))
            {
                rect_character2.y-=1;
            }

            while(collisionLayer(mapas_collision[mapa_actual],rect_character2))
            {
                rect_character2.y-=1;
            }
            orientation='d';
        }
        if(currentKeyStates[ SDL_SCANCODE_I ])
        {
            rect_character2.y-=velocity;
            while(collision(rect_character2,rect_npc))
            {
                rect_character2.y+=1;
            }
            while(collisionLayer(mapas_collision[mapa_actual],rect_character2))
            {
                rect_character2.y+=1;
            }
            orientation='u';
        }
         //segunda imagen
         if(currentKeyStates[ SDL_SCANCODE_D ])
        {
            rect_character.x+=velo;
             while(collision(rect_character,rect_npc))
            {
                rect_character.x-=1;
            }
            while(collisionLayer(mapas_collision[mapa_actual],rect_character))
            {
                rect_character.x-=1;
            }
            ori2='r';
        }
         if(currentKeyStates[ SDL_SCANCODE_A ])
        {
            rect_character.x-=velo;
            while(collision(rect_character,rect_npc))
            {
                rect_character.x+=1;
            }
            while(collisionLayer(mapas_collision[mapa_actual],rect_character))
            {
                rect_character.x+=1;
            }
            ori2='l';
        }
        if(currentKeyStates[ SDL_SCANCODE_S ])
        {
            rect_character.y+=velo;
            while(collision(rect_character,rect_npc))
            {
                rect_character.y-=1;
            }

            while(collisionLayer(mapas_collision[mapa_actual],rect_character))
            {
                rect_character.y-=1;
            }
            ori2='d';
        }
        if(currentKeyStates[ SDL_SCANCODE_W ])
        {
            rect_character.y-=velo;
            while(collision(rect_character,rect_npc))
            {
                rect_character.y+=1;
            }
            while(collisionLayer(mapas_collision[mapa_actual],rect_character))
            {
                rect_character.y+=1;
            }
            ori2='u';
        }
        if(currentKeyStates[ SDL_SCANCODE_LSHIFT ])
        {
            velo=3;
            animation_velocity=10;
        }else
        {
            velo=1;
            animation_velocity=15;
        }
        if(currentKeyStates[ SDL_SCANCODE_U])
        {
                velocity=3;
                animation_velocity=10;
        }else
        {
            velocity=1;
            animation_velocity=15;
        }


        if(frame%animation_velocity==0)
        {
            current_sprite++;
            if(current_sprite>1)
                current_sprite=0;
        }

        if(currentKeyStates[ SDL_SCANCODE_1 ])
        {
            ini=true;
        }

        for(int i=0;i<warps[mapa_actual].size();i++)
        {
            if((collision(warps[mapa_actual][i]->rect,rect_character)) or (collision(warps[mapa_actual][i]->rect,rect_character2) ))
            {
                rect_character.x=warps[mapa_actual][i]->x;
                rect_character.y=warps[mapa_actual][i]->y;
                mapa_actual=warps[mapa_actual][i]->mapa;
                 Mix_PlayChannel(-1,str,0);
                 rect_character.y=150;
                 rect_character.x=150;
                 rect_character2.x=200;
                 rect_character2.y=200;
                 rect_star.x = rand()%200 + 100;
                 rect_star.y = rand()%200 + 100-40;
                 cout<<rect_star.x<<"b"<<endl;
                 cout<<rect_star.y<<endl;
                 starobtn=false;
            }
        }

        if((rect_character.y +15 > rect_star.y && rect_character.y - 15 < rect_star.y) &&
         (rect_character.x -15 < rect_star.x  &&  rect_character.x+15 > rect_star.x)) {
            starobtn=true;
            pts1++;
            cout<<"Player 1= "<<pts1<<"  -- Player 2="<<pts2<<endl;
            rect_star.x=4000;
            Mix_PlayChannel(-1,noti,0);
            if(pts1==3){
                win=true;
            }

         }
          if((rect_character2.y +15 > rect_star.y && rect_character2.y - 15 < rect_star.y) &&
         (rect_character2.x -15 < rect_star.x  &&  rect_character2.x+15 > rect_star.x)) {
            starobtn=true;
            pts2++;
            cout<<"Player 1= "<<pts1<<"  -- Player 2="<<pts2<<endl;
            rect_star.x=4000;
             Mix_PlayChannel(-1,noti,0);
             if(pts2==3){
                lose=true;
             }
         }



        SDL_Delay(17);

        if(ini==false){
            SDL_RenderCopy(renderer, begining,NULL, &rect_background );

        }
        else if(win==true){
            SDL_RenderCopy(renderer, gameover,NULL, &rect_background );
        }
        else if(lose==true){
             SDL_RenderCopy(renderer, winner,NULL, &rect_background );

        }
        else{
        SDL_RenderCopy(renderer, background, NULL, &rect_background);

        dibujarLayer(renderer,mapas_down[mapa_actual]);
        dibujarLayer(renderer,mapas_over[mapa_actual]);

        if(starobtn==false){
            SDL_RenderCopy(renderer, star,NULL, &rect_star);

         }

        SDL_RenderCopy(renderer, sprites[ori2][current_sprite], NULL, &rect_character);
        SDL_RenderCopy(renderer, charizard[orientation][current_sprite],NULL, &rect_character2);



        dibujarLayer(renderer,mapas_over[mapa_actual]);
        SDL_RenderCopy(renderer, texture_npc, NULL, &rect_npc);

        }

        //dibujarLayer(renderer,collision_map);

        SDL_RenderPresent(renderer);
        frame++;

    }

	return 0;
}

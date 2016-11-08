#include <iostream>
#include <chrono>
#include <sys/mman.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <sstream>


#include "hotreload.h"

#include "DynamicLib.h"

using FrameTime = double;


namespace Wrapper
{
    class InitSDLException : public std::exception
    {
    public:
        InitSDLException(std::string& msg) noexcept ;
        InitSDLException() noexcept ;
        ~InitSDLException() throw() ;

        virtual const char* what() const throw() ;

    private:
        std::string m_msg;
    };

    class SDL
    {
    public:
        SDL(Uint32 flags) throw (InitSDLException);
        ~SDL();
    };
}




void* Filler(void* Code)
{
    ReloadableCode* Result = reinterpret_cast<ReloadableCode*>(Code);
//    Result->StructToStubs = reinterpret_cast<void*>(&API);
    Result->StructToAPI = reinterpret_cast<void*>(&API);

    if(!Result->isValid)
    {
        reinterpret_cast<Hotreload*>(Result->StructToAPI)->Error = reinterpret_cast<hotreload_error*>(dlsym(Result->HandleToSharedObject,"hotreload_error"));
    } else{
        reinterpret_cast<Hotreload*>(Result->StructToAPI)->Error =  stub_error;
    }

    return reinterpret_cast<void*>(Result);
}


void HandleInput(){
    TIMEBLOCK;

    SDL_Event event;
    while(SDL_PollEvent(&event) != 0) {
        switch(event.key.keysym.sym) {
            case SDL_QUIT:
            case SDLK_ESCAPE:
                exit(0);
        }
    }
}

/**
* Render the message we want to display to a texture for drawing
* @param message The message we want to display
* @param fontFile The font we want to use to render the text
* @param color The color we want the text to be
* @param fontSize The size we want the font to be
* @param renderer The renderer to load the texture in
* @return An SDL_Texture containing the rendered message, or nullptr if something went wrong
*/
SDL_Texture* renderText(const std::string &message, const std::string &fontFile,
                        SDL_Color color, int fontSize, SDL_Renderer *renderer)
{
    TIMEBLOCK;
    //Open the font
    TTF_Font *font = TTF_OpenFont(fontFile.c_str(), fontSize);
    if (font == nullptr){
        //logSDLError(std::cout, "TTF_OpenFont");
        return nullptr;
    }
    //We need to first render to a surface as that's what TTF_RenderText
    //returns, then load that surface into a texture
    SDL_Surface *surf = TTF_RenderText_Blended_Wrapped(font, message.c_str(), color,1324);
    if (surf == nullptr){
        TTF_CloseFont(font);
        //logSDLError(std::cout, "TTF_RenderText");
        return nullptr;
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
    if (texture == nullptr){
        //logSDLError(std::cout, "CreateTexture");
    }
    //Clean up the surface and font
    SDL_FreeSurface(surf);
    TTF_CloseFont(font);
    return texture;
}

auto printDebug(SDL_Renderer *renderer, double d) -> SDL_Texture*
{
    TIMEBLOCK;
    std::stringstream s;
    for(int size = 0; size < ArrayCount(records);size++)
    {

        s<<"Function: \t"<<records[size].functionname
         <<"Cycle: \t"<<(records[size].CycleCount/1000) <<" [mc]\t"
         <<"Cycle/Frame: \t"<<(records[size].CycleCount/1000)/d <<" [mc/f]\t"
         <<"Delta: \t" <<records[size].DeltaValue <<" [ns]\t"
         <<"Cy*Delta: \t"<<((records[size].CycleCount/1000)*(records[size].DeltaValue))*d<<" [mc*delta/f]"
         <<" \n";
    }

    return  renderText(s.str(),"/home/johannes/ClionProjects/HotReload/OpenSans-SemiboldItalic.ttf",{255,255,255,SDL_ALPHA_OPAQUE},18,renderer);
    /*if(Text == nullptr)
    {
        std::cerr<<SDL_GetError()<<"\n";
        exit(1);
    }
    SDL_RenderCopy(renderer,Text,nullptr,&(SDL_Rect{0,0,1124,225}));
    std::cerr<<s.str();*/


}

void pi_test_()
{
    TIMEBLOCK;

}

void pi2_test()
{
    TIMEBLOCK;

}
/*
namespace {
    DEBUG_RECORDS records[__COUNTER__ + 1];
}*/

int main(int argc, char** argv) {

    //TIMEBLOCK;

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////Create Configuartion Settings///////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    ConfigSettingString ProgramName("HotReload","","HotReload");
    //ConfigSettingString SharedObject("SharedObject","","/home/johannes/.CLion2016.2/system/cmake/generated/hotreloadSO-5eb34aa/5eb34aa/Debug/libhotreload.so");
    ConfigSettingString SharedObject("SharedObject","","libhotreload.so");

    ConfigSettingInt MemorySize("MemorySize","The amount of Memory which should be managed by the programm",Gigabytes(1));


    ///////////////////////////////////////////////////////////////////////////
    ///////////////////Declare Program Specific Variables//////////////////////
    ///////////////////////////////////////////////////////////////////////////
    //MemoryManagment
    DynamicLib Library((char*)(*(head_s->FindSetting("SharedObject"))));
    void* MemoryOrigin = nullptr;

    //Graphic
    Wrapper::SDL sdl(SDL_INIT_EVERYTHING);
    if (TTF_Init() != 0){
        //logSDLError(std::cout, "TTF_Init");
        SDL_Quit();
        return 1;
    }
    SDL_Window* win = nullptr;
    SDL_Renderer* renderer = nullptr;

    //Time Measurment
    FrameTime fps;
    FrameTime lastFt = 0.0f;
    auto currentSlice = lastFt;
    auto ftSlice = .9f;

    int high=0,low = 0;
    int high2=0,low2 = 0;

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////Initialise Variables////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    //Initiate Memory Managment
    SDL_Texture* DEBUGINFO = nullptr;

    MemoryOrigin = mmap(BaseAdress,*head->FindSetting("MemorySize"),PROT_READ|PROT_WRITE,MAP_ANONYMOUS,-1,0);
    if(MemoryOrigin == nullptr)
        throw std::bad_alloc();

    //Initiate graphic output
    SDL_CreateWindowAndRenderer(1224,786,SDL_WINDOW_SHOWN,&win,&renderer);

    if(win == nullptr || renderer == nullptr)
        std::cerr<<SDL_GetError()<<"\n";

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////Enter into the main functionalty////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    while(1)
    {
        auto timePoint1(std::chrono::high_resolution_clock::now());
        try{
            //Reload Code if necessary
            if(Library.LastWrite().tv_sec != GetLastWriteTime((char*)(*(head_s->FindSetting("SharedObject")))).tv_sec)
            {
                Library.unload();
                sleep(1);
                Library.reload();
            }
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            //EventLogic
            //
            currentSlice = lastFt;
            for(; currentSlice >= ftSlice; currentSlice -= ftSlice)
            {
                //logic
                //someupdatefunction(FixedtimeStep)
                HandleInput();

                pi_test_();
                pi2_test();

                DEBUGINFO = printDebug(renderer,lastFt);
                if(DEBUGINFO == nullptr)
                    exit(1);

                SDL_SetWindowTitle(win,("Frametime: "+
                                        std::to_string(lastFt)+
                                        "; FPS: "+std::to_string(fps)
                ).c_str());
            }

            //RenderLogic
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer,DEBUGINFO, nullptr,&(SDL_Rect{0,0,1124,225}));
            SDL_RenderPresent(renderer);


        }catch (std::runtime_error& error){
            std::cerr<<error.what();
            API.Error = stub_error;
        }

        auto timePoint2(std::chrono::high_resolution_clock::now());
        auto elapsedTime(timePoint2 - timePoint1);
        FrameTime ft{std::chrono::duration_cast<
                     std::chrono::duration<float, std::milli>>(elapsedTime).count()};

        lastFt = ft;

        auto ftSeconds(ft / 1000.f);
        fps = (1.f / ftSeconds);



    }




    ///////////////////////////////////////////////////////////////////////////
    ///////////////////Clean up////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    return 0;
}




timespec GetLastWriteTime(const std::string& filename){
    struct stat buf;
    stat(filename.c_str(),&buf);
    //std::cerr<<"----------> LastWrite Time of: "<<filename<<" is => "<<buf.st_mtim.tv_sec<<"\n";
    return buf.st_mtim;
};

namespace Wrapper
{
    InitSDLException::InitSDLException() noexcept  : m_msg(std::string(SDL_GetError()))  {}
    InitSDLException::InitSDLException(std::string &msg) noexcept  : m_msg(msg + std::string(SDL_GetError())){}
    InitSDLException::~InitSDLException() throw(){}

    const char * InitSDLException::what() const throw(){}

    SDL::SDL(Uint32 flags) throw(InitSDLException){

        if(SDL_Init(flags) == -1)
            throw InitSDLException();
    }

    SDL::~SDL() {}

}


//auto avg = (measure<std::chrono::nanoseconds>::duration(SDL_SetWindowTitle,win,("Frametime: "+std::to_string(lastFt)+"; FPS: "+std::to_string(fps)).c_str() ));

/*std::cerr<<"\nFrameTime   : "<<lastFt/1000.f<<"\n"
         <<"FPS         : "<<fps<<"\n"
         <<"CurrentSlice: "<<currentSlice<<"\n"
                                         <<"\n"<<"Duration: "<<avg.count()/1000.f<<" nanoseconds/1000\n";*/
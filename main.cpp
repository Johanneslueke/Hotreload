#include <iostream>
#include <chrono>

#include <SDL2/SDL.h>



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

int main(int argc, char** argv) {

    std::cerr<<"Anzahl der Argumente: "<<argc<<"\n";
    std::cerr<<argv[0]<<"\n\n";

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

    MemoryOrigin = mmap(BaseAdress,*head->FindSetting("MemorySize"),PROT_READ|PROT_WRITE,MAP_ANONYMOUS,-1,0);
    if(MemoryOrigin == nullptr)
        throw std::bad_alloc();

    //Initiate graphic output

    SDL_CreateWindowAndRenderer(500,500,SDL_WINDOW_SHOWN,&win,&renderer);


    std::cerr<<"CycleHigh Delta: "<<high<<"|"<<high<<" CycleLow Delta: "<<((low2-low))/1000.f/1000.f<<" MegaCycle"<<"\n";


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

            //RenderLogic

            SDL_RenderClear(renderer);



            if(true)
            {
                //API.Error(nullptr);
                ((hotreload_error*)(Library["hotreload_error"]))(nullptr);
            }
            SDL_RenderPresent(renderer);


            //EventLogic
            //
            currentSlice = lastFt;
            for(; currentSlice >= ftSlice; currentSlice -= ftSlice)
            {
                //logic
                //someupdatefunction(FixedtimeStep)


                SDL_SetWindowTitle(win,("Frametime: "+
                                        std::to_string(lastFt)+
                                        "; FPS: "+std::to_string(fps)
                ).c_str());

                auto avg = (measure<std::chrono::nanoseconds>::duration(SDL_SetWindowTitle,win,("Frametime: "+std::to_string(lastFt)+"; FPS: "+std::to_string(fps)).c_str() ));

                /*std::cerr<<"\nFrameTime   : "<<lastFt/1000.f<<"\n"
                         <<"FPS         : "<<fps<<"\n"
                         <<"CurrentSlice: "<<currentSlice<<"\n"
                                                         <<"\n"<<"Duration: "<<avg.count()/1000.f<<" nanoseconds/1000\n";*/

            }


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



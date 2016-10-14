//
// Created by johannes on 9/27/16.
//

#ifndef HOTRELOAD_HOTRELOAD_H
#define HOTRELOAD_HOTRELOAD_H

#include <map>
#include <dlfcn.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <chrono>

///////////////////////////////////////////////////////////////////////////
/////////////////////////ConfigClass///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
class ConfigSettingString {
public:
    /// Registers an integer setting
    ConfigSettingString(const char* name, const char* synopsis, const std::string& initialValue);

    /// Assigns an integer value to the setting
    ConfigSettingString& operator=(const std::string& value);

    /// Returns the setting's value as integer
    operator char*(void) const;

    /// Tries to find a setting, returns a nullptr if no setting could be found
    static ConfigSettingString* FindSetting(const char* name);

private:
    void AddToList(void);

    ConfigSettingString* m_next;
    const char* m_name;
    const char* m_synopsis;
    std::string m_value;
};

namespace {

/*
 * No dynamic memory allocation, minimal pre-main work, no singletons are required.
 * This little trick allows us to make sure that we have a simple global linked list of
 * ConfigSetting values where the memory is assured to be valid even if the actual
 * memory system isn't initialized yet. This is due the place these values are
 * stored (either the .bss or .sdata segment )!!
 */
    static ConfigSettingString* head_s = nullptr;
    static ConfigSettingString* tail_s = nullptr;
}

class ConfigSettingInt {
public:
    /// Registers an integer setting
    ConfigSettingInt(const char* name, const char* synopsis, int initialValue);

    /// Registers an integer setting, constraining it to the range [minValue, maxValue]
    ConfigSettingInt(const char* name, const char* synopsis, int initialValue, int minValue, int maxValue);

    /// Assigns an integer value to the setting
    ConfigSettingInt& operator=(int value);

    /// Returns the setting's value as integer
    operator int(void) const;

    /// Tries to find a setting, returns a nullptr if no setting could be found
    static ConfigSettingInt* FindSetting(const char* name);

private:
    void AddToList(void);

    ConfigSettingInt* m_next;
    const char* m_name;
    const char* m_synopsis;
    int m_value;
    int m_min;
    int m_max;
};

namespace {

/*
 * No dynamic memory allocation, minimal pre-main work, no singletons are required.
 * This little trick allows us to make sure that we have a simple global linked list of
 * ConfigSetting values where the memory is assured to be valid even if the actual
 * memory system isn't initialized yet. This is due the place these values are
 * stored (either the .bss or .sdata segment )!!
 */
    static ConfigSettingInt* head = nullptr;
    static ConfigSettingInt* tail = nullptr;
}

//////////////////////////////////////////////////////////////////////////
///////////////////////TYPEDEFS///////////////////////////////////////////
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef intptr_t intptr;
typedef uintptr_t uintptr;

typedef size_t memory_index;

typedef float real32;
typedef double real64;

typedef int8 s8;
typedef int8 s08;
typedef int16 s16;
typedef int32 s32;
typedef int64 s64;
typedef bool32 b32;

typedef uint8 u8;
typedef uint8 u08;
typedef uint16 u16;
typedef uint32 u32;
typedef uint64 u64;

typedef real32 r32;
typedef real64 r64;

typedef uint8* MemoryAdress;

///////////////////////////////////////////////////////////////////////////
/////////////////////Pre-Processor Macros//////////////////////////////////
///////////////////////////////////////////////////////////////////////////

#define PointerToU32(Pointer) ((u32)(memory_index)(Pointer))

#define Bytes(Value)     (Value)
#define Kilobytes(Value) (Bytes(Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define AlignPow2(Value, Alignment) ((Value + ((Alignment) - 1)) & ~((Alignment) - 1))
#define Align4(Value) ((Value + 3) & ~3)
#define Align8(Value) ((Value + 7) & ~7)
#define Align16(Value) ((Value + 15) & ~15)

#ifndef DEBUG
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression) if(!(Expression)) \
         {std::cerr<<"Error: "<<__FILE__<<"->"<<__LINE__<<" "<<"\n";}
#endif

///////////////////////////////////////////////////////////////////////////
///////////////////Structures//////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

struct ReloadableCode
{
    void* StructToAPI = nullptr;
    void* HandleToSharedObject = nullptr;

    timespec last_modifcation;
    bool isValid = false;
};
//Function to fill the ReloadableCode API which is userdefined
typedef void* (* FillAPI)(void*);

//Set Baseadress to 0x10000000
static void* BaseAdress = (void*)Gigabytes(4);

typedef struct MemoryChunk
{
    MemoryAdress StartAdress;
    memory_index ChunkSize;
    /* data */

    MemoryChunk(MemoryAdress a,memory_index s) : StartAdress(a),ChunkSize(s){};
}Chunk;

typedef struct  MemoryArena
{
    MemoryAdress StartAdress;
    memory_index Used;
    memory_index Size;
    std::map<std::string, MemoryChunk*> MemoryChunks;
    uint64 ID;
    /* data */

    MemoryArena():MemoryChunks()
    {};

}Arena;

///////////////////////////////////////////////////////////////////////////
typedef  void (*ErrorHandle)(const void*);
enum ERRORTYPE
{
    UNKOWN,
    BADALLOC,

};

typedef struct ErrorInformation
{
    std::string Error;
    ERRORTYPE   ErrorType;
    ErrorHandle HandleError;
}Error;

///////////////////////////////////////////////////////////////////////////
/////////////////////Function META Declarations////////////////////////////
///////////////////////////////////////////////////////////////////////////
#define HOTRELOAD_ERROR(name) void name(const Error*)
#define HOTRELOAD_IO_STREAM(name) const char* name(const char*, int byte)
#define HOTRELOAD_MEMORY_FUNCTION(name) MemoryArena* name(void* adress, int i)


///////////////////////////////////////////////////////////////////////////
/////////////////////Function Stub Implementation//////////////////////////
///////////////////////////////////////////////////////////////////////////
namespace{//anonymus namespace prevents symbol clashing
    HOTRELOAD_ERROR(stub_error){ /*std::cerr<<"stub\n"; */};
    HOTRELOAD_IO_STREAM(stub_readFile){};
    HOTRELOAD_IO_STREAM(stub_writeFile){};
    HOTRELOAD_MEMORY_FUNCTION(stub_allocate_arena){};
    HOTRELOAD_MEMORY_FUNCTION(stub_deallocate_arena){};
}

///////////////////////////////////////////////////////////////////////////
///////////////////Function Pointer Declaration////////////////////////////
///////////////////////////////////////////////////////////////////////////
typedef HOTRELOAD_ERROR(hotreload_error);
typedef HOTRELOAD_IO_STREAM(hotreload_readFile);
typedef HOTRELOAD_IO_STREAM(hotreload_writeFile);
typedef HOTRELOAD_MEMORY_FUNCTION(hotreload_allocate_arena);
typedef HOTRELOAD_MEMORY_FUNCTION(hotreload_deallocate_arena);


///////////////////////////////////////////////////////////////////////////
/////////////Collection of Function Pointer via Struct/////////////////////
///////////////////////////////////////////////////////////////////////////
typedef struct HOTRELOADAPI
{
    hotreload_error * Error = stub_error;
    //hotreload_readFile * readFile;
    //hotreload_writeFile * writeFile;
    //hotreload_allocate_arena * allocate_arena;
    //hotreload_deallocate_arena * deallocate_arena;
}Hotreload;

namespace {
    static void* SO = nullptr;
    static Hotreload API;
}

///////////////////////////////////////////////////////////////////////////
///////////////////Shared Object LoadInterface/////////////////////////////
///////////////////////////////////////////////////////////////////////////
#ifdef  __WIN32__
#elif __APPLE__
#else
timespec GetLastWriteTime(const std::string& filename) ;
#endif

///////////////////////////////////////////////////////////////////////////
///////////////////time measuring mechanism////////////////////////////////
///////////////////////////////////////////////////////////////////////////

#define fw(what) std::forward<decltype(what)>(what)

/**
	* @ class measure
	* @ brief Class to measure the execution time of a callable
	*/
template <
        typename TimeT = std::chrono::milliseconds, class ClockT = std::chrono::system_clock
>
struct measure
{
    /**
    * @ fn    execution
    * @ brief Returns the quantity (count) of the elapsed time as TimeT units
    */
    template<typename F, typename ...Args>
    static typename TimeT::rep execution(F&& func, Args&&... args)
    {
        auto start = ClockT::now();

        fw(func)(std::forward<Args>(args)...);

        auto duration = std::chrono::duration_cast<TimeT>(ClockT::now() - start);

        return duration.count();
    }

    /**
    * @ fn    duration
    * @ brief Returns the duration (in chrono's type system) of the elapsed time
    */
    template<typename F, typename... Args>
    static TimeT duration(F&& func, Args&&... args)
    {
        auto start = ClockT::now();

        fw(func)(std::forward<Args>(args)...);

        return std::chrono::duration_cast<TimeT>(ClockT::now() - start);
    }
};


///////////////////////////////////////////////////////////////////////////
///////////////////cpu cycle measuring mechanism///////////////////////////
///////////////////////////////////////////////////////////////////////////


#ifdef  __WIN32__

#ifdef __x86_64__
inline u32 GetThreadID(void) {
 u32 ThreadID;
    asm("mov %%fs:0x10,%0" : "=r"(ThreadID));
    return (ThreadID);
}
#elif  __i386__
inline u32 GetThreadID(void) {
 u32 ThreadID;
    asm("mov %%fs:0x10,%0" : "=r"(ThreadID));
    return (ThreadID);
}
#else
#error Unsupported architecture
#endif

#elif __APPLE__

#ifdef __x86_64__
inline u32 GetThreadID(void) {
 u32 ThreadID;
    asm("mov %%gs:0x00,%0" : "=r"(ThreadID));
    return (ThreadID);
}
#elif  __i386__
inline u32 GetThreadID(void) {
 u32 ThreadID;
    asm("mov %%fs:0x10,%0" : "=r"(ThreadID));
    return (ThreadID);
}
#else
#error Unsupported architecture
#endif

#else //Linux


#ifdef __x86_64__
inline u32 GetThreadID(void) {
    u32 ThreadID;
    asm("mov %%fs:0x10,%0" : "=r"(ThreadID));
    return (ThreadID);
}

/*
 * \name get_CPU_CYCLE_FIRST
 * \origin http://www.intel.com/content/www/us/en/embedded/training/ia-32-ia-64-benchmark-code-execution-paper.html
 * \brief
 * The "CPUID call" implements a barrier which avoids "out-of-order execution"
 * of instructions above and below the RDTSC instruction. This avoids to call a
 * CPUID instruction in between the reads of the real-time register.
 *
 * It then reads via the RDTSC instruction the timestamp register and store the
 * value in memory.
 *
 * After this function call the function which should be measured has to be executed.
 * Afterwards get_CPU_CYCLE_SECOND is expected to be called!!!
 */
inline u64 get_CPU_CYCLE_FIRST( )
{
    u64 cycles_high = 0;
    u64 cycles_low = 0;
    asm volatile (
            "CPUID\n\t" // serialize
            "RDTSC\n\t" // read TIME STAMP COUNTER
            "mov %%edx, %0\n"
            "mov %%eax, %1\n": "=r" (cycles_high), "=r" (cycles_low)::
            "%rax", "%rbx", "%rcx", "%rdx");
    return cycles_low;
}

/*
 * \name get_CPU_CYCLE_SECOND
 * \origin http://www.intel.com/content/www/us/en/embedded/training/ia-32-ia-64-benchmark-code-execution-paper.html
 * \brief
 *
 * Does the the same as get_CPU_CYLE_FIRST but implements it in a differen way
 * First "RDTSCP instruction" reads the timestamp register and now guarantees
 * that the execution of all the code we wanted to measure is >completed< .
 *
 * And the we call the CPUID instruction call again which gives us a barrier so
 * it is impossible that any instruction coming afterwards is executed before
 * CPUID itself.
 *
 */
inline u64 get_CPU_CYCLE_SECOND()
{

    u64 cycles_high = 0;
    u64 cycles_low = 0;
    asm volatile (
            "RDTSCP\n\t"/*read the clock*/
            "mov %%edx, %0\n\t"
            "mov %%eax, %1\n\t"
            "CPUID\n\t": "=r" (cycles_high), "=r"
            (cycles_low):: "%rax", "%rbx", "%rcx", "%rdx");
    return cycles_low;
}

#elif  __i386__
inline u32 GetThreadID(void) {
    asm("mov %%fs:0x10,%0" : "=r"(ThreadID));
    return (ThreadID);
#else

#error Unsupported architecture

#endif
#endif

///////////////////////////////////////////////////////////////////////////
/////////////////////////////////Debug mechanism///////////////////////////
///////////////////////////////////////////////////////////////////////////

#define TIMEBLOCK__(NUMBER, ...) timed_block TimeBlock_##NUMBER(__COUNTER__, __FILE__, __LINE__, FUNCTION__, ## __VA_ARGs__)
#define TIMEBLOCK_(NUMBER, ...) TIMEBLOCK__(NUMBER, ## __VA_ARGS__)
#define TIMEBLOCK(...) TIMEBLOCK_(__LINE__, ## __VA_ARGS__)

struct DEBUG_RECORDS{
    char* filename;
    char* functionname;

    u32   linenumber = 0;
    u64   clocks = 0;

    u32 hitcounter = 0;
};



DEBUG_RECORDS *DebugRecordArray;



class timed_block{
public:
    DEBUG_RECORDS* Record;

    timed_block(int counter,char* filename,int line,char* function, int hitcount=1) {
        this->Record = DebugRecordArray + counter;
        Record->filename = filename;
        Record->linenumber = line;
        Record->functionname = function;
        Record->clocks = get_CPU_CYCLE_FIRST();
        Record->hitcounter += hitcount;
    }

    ~timed_block(){

        Record->clocks = get_CPU_CYCLE_SECOND();

    }
};

#endif //HOTRELOAD_HOTRELOAD_H

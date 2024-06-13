#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <mutex>
#include <unistd.h>
#include <string.h>

#include <sys/mman.h>
#include <fcntl.h>
#include "state_rt.h"
#include "hash.h"

#define HASH_CONST 0xa5b35705
#define SHM_FILE_PATH 
#define SV_MAP_SIZE 2097152
using namespace std;
static mutex SV_mutex;


namespace __state_instrument 
{

int  shm_fd;
char shm_file_path[SHM_FILE_NAME_LEN];
stateINFO *MmapWriter = NULL;  


static bool is_initialized = false;

static std::map<uint32_t, uint32_t> *SV_HashMap = nullptr;

void shm_destroy(){
    SV_mutex.lock();
    if(MmapWriter != NULL){
        munmap(MmapWriter, SHM_STATE_MAP_SIZE);
        MmapWriter = NULL;
    }

    if(shm_fd != -1){
        //close(shm_fd);
        shm_fd = -1;
    }
    SV_mutex.unlock();
};


__attribute__ ((constructor, no_sanitize("address", "memory"))) void shm_init(){
    //TODO:
    // if(getenv("STATE_MMAP_FILE")){
    //     
    //     snprintf(shm_file_path, L_tmpnam, "/SV_%d_%ld", getpid(), random());
    // }
    // else{
        
    // }
    SV_HashMap = new std::map<uint32_t, uint32_t>;
    snprintf(shm_file_path, L_tmpnam, "/SVshm");
    int shm_fd = shm_open(shm_file_path, O_CREAT | O_RDWR, 0777);
    if(shm_fd < 0){
        perror("shm_open() failed!");
        exit(1);
    }
    if (ftruncate(shm_fd, SHM_STATE_MAP_SIZE)) {
        perror("ftruncate() failed");
        exit(1);
    }

    MmapWriter = (stateINFO *)mmap(NULL, SHM_STATE_MAP_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(MmapWriter == MAP_FAILED){
        shm_fd = -1;
        printf("mmap() failed\n");
        exit(1);
    }
    MmapWriter->hashstate = 0;
    MmapWriter->counter = 0;
    is_initialized = true;
    atexit(shm_destroy);
};

static uint8_t __SV_bitmap[SV_MAP_SIZE];//2^21
uint8_t *SV_bitmap_ptr = __SV_bitmap;

}//namespace  __state_instrument 

using namespace __state_instrument;


/*

*/
extern "C" __attribute__((no_sanitize("address", "memory"))) void __SV_hash_u8(int32_t SV_RND, int8_t SVvalue){   
    u_int32_t SVxorValue = SV_RND ^ SVvalue;
    if(SV_HashMap->find(SV_RND) == SV_HashMap->end()){
        SV_bitmap_ptr[SVxorValue%SV_MAP_SIZE] |= 1;
    }
    else{
        SV_bitmap_ptr[(*SV_HashMap)[SV_RND]%SV_MAP_SIZE] &= 0;
        SV_bitmap_ptr[SVxorValue%SV_MAP_SIZE] |= 1;
    }
    (*SV_HashMap)[SV_RND] = SVxorValue;
}

extern "C" __attribute__((no_sanitize("address", "memory"))) void __SV_hash_u16(int32_t SV_RND, int16_t SVvalue){
    u_int32_t SVxorValue = SV_RND ^ SVvalue;
    if(SV_HashMap->find(SV_RND) == SV_HashMap->end()){
        SV_bitmap_ptr[SVxorValue%SV_MAP_SIZE] |= 1;
    }
    else{
        SV_bitmap_ptr[(*SV_HashMap)[SV_RND]%SV_MAP_SIZE] &= 0;
        SV_bitmap_ptr[SVxorValue%SV_MAP_SIZE] |= 1;
    }
    (*SV_HashMap)[SV_RND] = SVxorValue;
}

extern "C" __attribute__((no_sanitize("address", "memory"))) void __SV_hash_u32(int32_t SV_RND, int32_t SVvalue){
    u_int32_t SVxorValue = SV_RND ^ SVvalue;
    if(SV_HashMap->find(SV_RND) == SV_HashMap->end()){
        SV_bitmap_ptr[SVxorValue%SV_MAP_SIZE] |= 1;
    }
    else{
        SV_bitmap_ptr[(*SV_HashMap)[SV_RND]%SV_MAP_SIZE] &= 0;
        SV_bitmap_ptr[SVxorValue%SV_MAP_SIZE] |= 1;
    }
    (*SV_HashMap)[SV_RND] = SVxorValue; 
}

extern "C" __attribute__((no_sanitize("address", "memory"))) void __SV_hash_u64(int32_t SV_RND, int64_t SVvalue){
    u_int32_t L_value = SVvalue & (0x00000000ffffffff);
    u_int32_t SVxorValue = SV_RND ^ L_value;
    if(SV_HashMap->find(SV_RND) == SV_HashMap->end()){
        SV_bitmap_ptr[SVxorValue%SV_MAP_SIZE] |= 1;
    }
    else{
        SV_bitmap_ptr[(*SV_HashMap)[SV_RND]%SV_MAP_SIZE] &= 0;
        SV_bitmap_ptr[SVxorValue%SV_MAP_SIZE] |= 1;
    }
    (*SV_HashMap)[SV_RND] = SVxorValue;
    
}

extern "C" __attribute__((no_sanitize("address", "memory")))  void __state_dump(){
    if(MmapWriter){

        u64 cksum = hash64(SV_bitmap_ptr, SV_MAP_SIZE, HASH_CONST);
        //printf("###SV###: state = %llx\n", cksum);
        MmapWriter->hashstate = cksum;
        MmapWriter->counter++;
    }
    else{
        printf("[ERROR]SV: state dump Failed! Check mmap!");
    }
}

/*
防数据竞争
*/
extern "C" __attribute__((no_sanitize("address", "memory"))) void __SV_lock(){
    if (__state_instrument::is_initialized == false)
        shm_init();
    SV_mutex.lock();
}

extern "C" __attribute__((no_sanitize("address", "memory"))) void __SV_unlock(){
    SV_mutex.unlock();
}

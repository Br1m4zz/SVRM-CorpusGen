#define DUMPSIZE 10

#ifndef __STATE_RT__
#define __STATE_RT__

#include <stdint.h>
#include <stdlib.h>
#include <map>

#ifdef __cplusplus
extern "C" {
#endif



void __SV_lock();
void __SV_unlock();
void __SV_hash_u8(int32_t SV_RND, int8_t SVval);
void __SV_hash_u16(int32_t SV_RND, int16_t SVval);
void __SV_hash_u32(int32_t SV_RND, int32_t SVval);
void __SV_hash_u64(int32_t SV_RND, int64_t SVval);

//插在交互输出函数前后的桩
void __state_dump();

#ifdef __cplusplus
}
#endif

namespace __state_instrument 
{

struct stateINFO{
    uint64_t hashstate;
    uint32_t counter = 0;
};

#define SHM_STATE_MAP_SIZE  sizeof(stateINFO)  
#define SHM_FILE_NAME_LEN   20
}

#endif


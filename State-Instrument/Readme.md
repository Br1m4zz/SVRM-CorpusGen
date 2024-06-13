# 功能简述
## instrument
`clang-10 -fno-discard-value-names -Xclang -load -Xclang /home/lwfdev/state_instruement/SVInstrument_Pass.so`

### example: LightFTP
`CC="clang-10 -fno-discard-value-names -g -O0 -Xclang -load -Xclang /home/ubuntu/state_instruement/SVInstrument_Pass.so" make`
`clang-10  -g -O0  -o "fftp"  ./cfgparse.o ./ftpserv.o ./main.o ./x_malloc.o   -lpthread -lgnutls -L./ -l:state_rt.a -lstdc++ -lrt`
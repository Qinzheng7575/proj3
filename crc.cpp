#include "crc.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>
#include <assert.h>

/*G=X16+X15+X2+1
  正向CRC
*/
static const uint16_t CRC16_Table[256] = {
    0x0000, 0x8005, 0x800f, 0x000a, 0x801b, 0x001e, 0x0014, 0x8011, 
    0x8033, 0x0036, 0x003c, 0x8039, 0x0028, 0x802d, 0x8027, 0x0022, 
    0x8063, 0x0066, 0x006c, 0x8069, 0x0078, 0x807d, 0x8077, 0x0072, 
    0x0050, 0x8055, 0x805f, 0x005a, 0x804b, 0x004e, 0x0044, 0x8041, 
    0x80c3, 0x00c6, 0x00cc, 0x80c9, 0x00d8, 0x80dd, 0x80d7, 0x00d2, 
    0x00f0, 0x80f5, 0x80ff, 0x00fa, 0x80eb, 0x00ee, 0x00e4, 0x80e1, 
    0x00a0, 0x80a5, 0x80af, 0x00aa, 0x80bb, 0x00be, 0x00b4, 0x80b1, 
    0x8093, 0x0096, 0x009c, 0x8099, 0x0088, 0x808d, 0x8087, 0x0082, 
    0x8183, 0x0186, 0x018c, 0x8189, 0x0198, 0x819d, 0x8197, 0x0192, 
    0x01b0, 0x81b5, 0x81bf, 0x01ba, 0x81ab, 0x01ae, 0x01a4, 0x81a1, 
    0x01e0, 0x81e5, 0x81ef, 0x01ea, 0x81fb, 0x01fe, 0x01f4, 0x81f1, 
    0x81d3, 0x01d6, 0x01dc, 0x81d9, 0x01c8, 0x81cd, 0x81c7, 0x01c2, 
    0x0140, 0x8145, 0x814f, 0x014a, 0x815b, 0x015e, 0x0154, 0x8151, 
    0x8173, 0x0176, 0x017c, 0x8179, 0x0168, 0x816d, 0x8167, 0x0162, 
    0x8123, 0x0126, 0x012c, 0x8129, 0x0138, 0x813d, 0x8137, 0x0132, 
    0x0110, 0x8115, 0x811f, 0x011a, 0x810b, 0x010e, 0x0104, 0x8101, 
    0x8303, 0x0306, 0x030c, 0x8309, 0x0318, 0x831d, 0x8317, 0x0312, 
    0x0330, 0x8335, 0x833f, 0x033a, 0x832b, 0x032e, 0x0324, 0x8321, 
    0x0360, 0x8365, 0x836f, 0x036a, 0x837b, 0x037e, 0x0374, 0x8371, 
    0x8353, 0x0356, 0x035c, 0x8359, 0x0348, 0x834d, 0x8347, 0x0342, 
    0x03c0, 0x83c5, 0x83cf, 0x03ca, 0x83db, 0x03de, 0x03d4, 0x83d1, 
    0x83f3, 0x03f6, 0x03fc, 0x83f9, 0x03e8, 0x83ed, 0x83e7, 0x03e2, 
    0x83a3, 0x03a6, 0x03ac, 0x83a9, 0x03b8, 0x83bd, 0x83b7, 0x03b2, 
    0x0390, 0x8395, 0x839f, 0x039a, 0x838b, 0x038e, 0x0384, 0x8381, 
    0x0280, 0x8285, 0x828f, 0x028a, 0x829b, 0x029e, 0x0294, 0x8291, 
    0x82b3, 0x02b6, 0x02bc, 0x82b9, 0x02a8, 0x82ad, 0x82a7, 0x02a2, 
    0x82e3, 0x02e6, 0x02ec, 0x82e9, 0x02f8, 0x82fd, 0x82f7, 0x02f2, 
    0x02d0, 0x82d5, 0x82df, 0x02da, 0x82cb, 0x02ce, 0x02c4, 0x82c1, 
    0x8243, 0x0246, 0x024c, 0x8249, 0x0258, 0x825d, 0x8257, 0x0252, 
    0x0270, 0x8275, 0x827f, 0x027a, 0x826b, 0x026e, 0x0264, 0x8261, 
    0x0220, 0x8225, 0x822f, 0x022a, 0x823b, 0x023e, 0x0234, 0x8231, 
    0x8213, 0x0216, 0x021c, 0x8219, 0x0208, 0x820d, 0x8207, 0x0202, 
};

//正向(大端字节序)
uint16_t crc16(uint8_t *frame, size_t len, int flag) {
// flag == 1 ? gen : check
    uint16_t r = 0; //初始化为零，相当于给data前面补零，但并不会影响最终结果
    
    for (size_t i = 0; i < len; i++) {
        r = ((r << 8) | frame[i]) ^ CRC16_Table[(r >> 8) & 0xff];
    }

   if (flag == 1) {
        for (size_t i = 0; i < 2; i++) {
            //r = ((r << 8) | f[i])^ CRC16_Table[(r >> 8) & 0xff];
            r = (r << 8) ^ CRC16_Table[(r >> 8) & 0xff];
        }
    }
    return r;
}//用于网络传输时，返回值需要作网络序转换

/*tester
uint8_t data_frame[1024 + 2];
int main() {
    srandom(time(0));
    int cnt = 10000;
    while (cnt--) {
        for (int i = 0; i < 1024; i++) data_frame[i] = (uint8_t)random();
        uint16_t crc = crc16(data_frame, 1024, 1);
        *(uint16_t *)(&data_frame[1024]) = htons(crc);
        assert(crc16(data_frame, 1026, 0) == 0);

           int idex = (uint16_t)random() % 1026;
           int val = (uint8_t)random();
           if (data_frame[idex] != val) {
                data_frame[idex] = val;
                assert(crc16(data_frame, 1026, 1) != 0);
           }
    }
    
//    printf("%0#.4x\n", crc);
//    printf("%0#.4x\n", crc16(data_frame, 1024, htons(crc)));
// 
//    data_frame[(uint16_t)random() % 1024] = (uint8_t)random();
//    printf("%0#.4x\n", crc16(data_frame, 1024, htons(crc)));
    
}
*/
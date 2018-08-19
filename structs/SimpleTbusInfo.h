//
// Created by capitalg on 2018/8/17.
//

#ifndef TENCENT_INTERN_SIMPLETBUSINFO_H
#define TENCENT_INTERN_SIMPLETBUSINFO_H


/**
 *  存在共享内存中的Tbus所需的所有信息
 */
struct SimpleTbusInfo {
    char name[64];
    int channel_count; // 当前tbus包含的通道数
};

#endif //TENCENT_INTERN_SIMPLETBUSINFO_H

#ifndef PUBLICDEF_H
#define PUBLICDEF_H

#define MAKE_INT(byte1, byte2) ((int)((((unsigned char)byte1)<<8) + (unsigned char)byte2))
#define MAKE_INT_4(byte1, byte2, byte3, byte4) ((int)((((unsigned char)byte1)<<24) + (((unsigned char)byte2)<<16) + (((unsigned char)byte3)<<8) + ((unsigned char)byte4)))

// context定义
#define CONTEXT_READ_SWITCH_STATUS  "read_switch_status"  // 读取充电MOS、放电MOS、均衡开关的状态
#define CONTEXT_WRITE_CHARGE_MOS_SWITCH  "write_charge_mos_switch"  // 设置充电MOS开关的状态
#define CONTEXT_WRITE_FANGDIAN_MOS_SWITCH  "write_fangdian_mos_switch"  // 设置放电MOS开关的状态
#define CONTEXT_WRITE_JUNHENG_SWITCH  "write_junheng_switch"  // 设置均衡开关的状态
#define CONTEXT_READ_BATTERY_JUNHENG_STATUS  "read_battery_junheng_status"  // 读取电池和均衡的状态
#define CONTEXT_READ_DIANYA_DIANLIU_DATA  "read_dianya_dianliu_data"  // 读取电压电流数据
#define CONTEXT_READ_TEMPERATURE_DATA  "read_temperature_data"  // 读取温度数据
#define CONTEXT_WRITE_PARAM_DATA  "write_param_data"  // 写入参数
#define CONTEXT_READ_SOFTWARE_VERSION  "read_software_version"  // 读取软件版本
#define CONTEXT_CANCEL_UPGRADE  "cancel_upgrade"  // 取消升级
#define CONTEXT_START_UPGRADE  "start_upgrade"  // 开始升级
#define CONTEXT_SEND_UPGRADE_DATA  "send_upgrade_data"  // 发送升级数据
#define CONTEXT_SEND_UPGRADE_DATA_FINISH  "send_upgrade_data_finish"  // 发送升级数据完成
#define CONTEXT_GET_UPGRADE_PROGRESS  "get_upgrade_progress"  // 获取升级进度

#endif // PUBLICDEF_H

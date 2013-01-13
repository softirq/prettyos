#ifndef     _HD_H_
#define     _HD_H_

#include "fs.h"
#define HD_TIMEOUT 	10000

#define MAKE_DEVICE_REG(lba,drive,lba_highest) ((lba << 6) | (drive << 4) | (lba_highest & 0xf) | 0xA0)


#define REG_DEV_CTL 	0x3f6

#define REG_DATA	0x1f0
#define REG_FEATURES 	0x1f1
#define REG_NSECTOR	0x1f2
#define REG_LBA_LOW	0x1f3
#define REG_LBA_MID	0X1F4
#define REG_LBA_HIGH	0x1f5
#define REG_DEVICE	0x1f6
#define REG_STATUS	0x1f7
#define REG_COMMAND 	REG_STATUS	


#define ATA_IDENTIFY 	0xec
#define ATA_READ	0x20
#define ATA_WRITE 	0x30

#define MAX_DRIVES		1 //磁盘个数
#define NR_PART_PER_DRIVE	4  //主分区个数
#define NR_PRIM_PER_DRIVE	(NR_PART_PER_DRIVE + 1)  //主分区个数加上整个硬盘

#define NR_LOG_PER_PART		16 //每个扩展分区的最大逻辑分区的个数
#define NR_LOG_PER_DRIVE 	(NR_LOG_PER_PART * NR_PART_PER_DRIVE)  //整个磁盘的最大逻辑分区的个数
#define NR_LOG_DRIVES		(NR_LOG_PER_DRIVE * MAX_DRIVES) 	//所有磁盘的最大逻辑分区的个数
//有两个硬盘的前提下，两个硬盘的最大的主分区号，第一个硬盘0-4(0是整个硬盘)，第二个硬盘5-9(5是整个硬盘)
#define MAX_PRIMARY		(MAX_DRIVES * NR_PRIM_PER_DRIVE - 1)  //计算机中主分区的最大个数

#define P_PRIMARY	0	//主分区
#define P_EXTENDED	1	//扩展分区

//分区类型 未用/引导分区/扩展分区/逻辑分区
#define UNUSED_PART	0
#define BOOT_PART	0x80
#define EXTENDED_PART 	0x5
#define LOGIC_PART	0X83

//分区表的在扇区中的偏移
#define PARTITION_TABLE_OFFSET 0x1BE

//由分区号得到磁盘设备号
//主分区和逻辑分区的情况
//每块硬盘主分区最大5个(包括硬盘本身) 
//逻辑分区最多64个(16 * 4)


//第二块硬盘的逻辑分区
#define MINOR_hd5a 	MINOR_hd1a + NR_LOG_PER_DRIVE

//1个磁盘 主分区编号0-4(包括硬盘本身)
#define DRV_OF_DEV(dev)	(dev <= MAX_PRIMARY ? \
			dev / NR_PRIM_PER_DRIVE:\
			(dev - MINOR_hd1a) / NR_LOG_PER_DRIVE)
			
#define MAJOR_SHIFT	8	
#define MAKE_DEV(a,b)	((a << MAJOR_SHIFT) | b)
#define MAJOR(x)	((x >> MAJOR_SHIFT) & 0x00ff)
#define MINOR(x)	(x & 0xff)


//逻辑分区从0x10开始算起 以便区分主分区和逻辑分区
#define MINOR_hd1a	0x10
#define MINOR_BOOT	(MINOR_hd1a)
//#define MINOR_hd2a	MINOR_hd1a + NR_LOG_PER_PART 	
#define ROOT_DEV 	(MAKE_DEV(DEV_HD,MINOR_BOOT))
						
struct partition_table
{
	u8	boot_mark; 	//引导标记
	u8	start_head;	//起始磁头号
	u8	start_sector;	//起始扇区号
	u8	start_cyl;	//起始助面号的低8位
	u8	system_id;	//分区类型 (主分区/扩展分区/逻辑分区)
	u8	end_head;	//结束磁头号
	u8	end_sector;	//结束扇区号
	u8	end_cyl;	//结束柱面号
	u32	start_sector_lba;	//起始扇区的LBA
	u32	nr_sectors;		//扇区数目
};

struct hd_cmd 
{
	u8 	features;
	u8	count;
	u8	lba_low;
	u8	lba_mid;
	u8	lba_high;
	u8 	device;
	u8	command;
	char 	buffer[SECTOR_SIZE];
};

struct partition_info
{
	u32	start_sector;
	u32	total_sectors;
};
struct hard_disk_info
{
	int 	open_cnt;		 //打开标记
	struct 	partition_info	primary[NR_PRIM_PER_DRIVE];
	struct 	partition_info	logical[NR_LOG_PER_DRIVE];
};

struct hard_disk_info hd_info[MAX_DRIVES];

extern void 	ha_handler(int irq);
extern void 	init_hd();
extern void 	hd_identify(int drive);
extern void 	hd_rw(int net_device,int start_sect,int nr_sects,int flag,struct buffer_head *bh);
extern void 	hd_open(int net_device);

#endif

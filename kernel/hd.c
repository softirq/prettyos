#include "type.h"
#include "const.h"
#include "list.h"
#include "wait.h"
#include "clock.h"
#include "traps.h"
#include "irq.h"
#include "fs.h"
#include "stdlib.h"
#include "syscall.h"
#include "hd.h"
#include "panic.h"
#include "blk_drv.h"
#include "string.h"
#include "printf.h"

/*
   there is one hard disk default
   */
u8 hd_buf[SECTOR_SIZE];
//在规定的时间内,判断主盘的状态;1为可读写，0为不可读写
int is_hd_status(int timeout)
{
    int t = get_ticks();
    int mask = 0x80;
    while(((get_ticks() -t) * 1000 / HZ) < timeout)
    {
        if((in_byte(REG_STATUS) & mask) == 0 )
            return 1;
    }
    return 0;
}

//读盘操作
int read_intr(char *buffer)
{
    if(!is_hd_status(HD_TIMEOUT))
    {
        //		printk("hd is not ready\n");
        return -1;
    }
    else	
        //		printk("hd is ready\n");
        port_read(REG_DATA,buffer,SECTOR_SIZE);
    return 0;
}

//写盘操作
int write_intr(char *buffer)
{
    if(!is_hd_status(HD_TIMEOUT))
    {
        //		printk("hd is not ready\n");
        return -1;
    }
    else	
        //		printk("hd is ready\n");
        port_write(REG_DATA,buffer,SECTOR_SIZE);
    return 0;
}

//将命令发送到各个寄存器中
void hd_cmd_out(struct hd_cmd *cmd)
{
    //active the interrupt enable bit
    out_byte(REG_DEV_CTL,0);
    out_byte(REG_FEATURES,cmd->features);
    out_byte(REG_NSECTOR,cmd->count);
    out_byte(REG_LBA_LOW,cmd->lba_low);
    out_byte(REG_LBA_MID,cmd->lba_mid);
    out_byte(REG_LBA_HIGH,cmd->lba_high);
    out_byte(REG_DEVICE,cmd->device);
    out_byte(REG_COMMAND,cmd->command);
}

//生成控制磁盘的命令(通用)
struct hd_cmd* make_cmd(struct hd_cmd *cmd,u8 features,u8 count,u8 lba_low,u8 lba_mid,u8 lba_high,u8 device,u8 command)
{
    /*	if(device> 1)
        panic("tring to write bad sector");
        */
    cmd->features = features;
    cmd->count = count;
    cmd->lba_low = lba_low;
    cmd->lba_mid = lba_mid;
    cmd->lba_high = lba_high;
    cmd->device = device;
    cmd->command = command;
    return cmd;
}

//硬盘中断程序,用来通知用户进程(添加信号机制)
void hd_handler(int irq)
{
    int mask = 0x80;
    if((in_byte(REG_STATUS) & mask) == 0 )
    {
        //	printk("hard disk done!");
    }
    else
    {
        printk("hard disk is busying\n");
    }
}
/*
//打印硬盘信息
static void print_identify_info(u16* hdinfo)
{
int i, k;
char s[64];

struct iden_info_ascii {
int idx;
int len;
char * desc;
} iinfo[] = {{10, 20, "HD SN"}, // Serial number in ASCII 
{27, 40, "HD Model"} // Model number in ASCII  
};

for (k = 0; k < sizeof(iinfo)/sizeof(iinfo[0]); k++) {
char * p = (char*)&hdinfo[iinfo[k].idx];
for (i = 0; i < iinfo[k].len/2; i++) {
s[i*2+1] = *p++;
s[i*2] = *p++;
}
s[i*2] = 0;
printk("%s: %s\n", iinfo[k].desc, s);
}

int capabilities = hdinfo[49];
printk("LBA supported: %s\n",
(capabilities & 0x0200) ? "Yes" : "No");

int cmd_set_supported = hdinfo[83];
printk("LBA48 supported: %s\n",
(cmd_set_supported & 0x0400) ? "Yes" : "No");

int sectors = ((int)hdinfo[61] << 16) + hdinfo[60];
printk("HD size: %dMB\n", sectors * 512 / 1000000);
}
*/
//识别硬盘信息
void hd_identify(int drive)
{
    struct hd_cmd cmd;
    u8 device = MAKE_DEVICE_REG(0,drive,0);
    u8 command = ATA_IDENTIFY;
    make_cmd(&cmd,0,0,0,0,0,device,command);	
    hd_cmd_out(&cmd);

    if(read_intr(cmd.buffer) == 0)
    {
        //	print_identify_info((u16*)hd_buf);
    }
    else
        printk("error\n");

    u16 *hdinfo = (u16 *)cmd.buffer;

    //设置整个磁盘的信息
    hd_info[drive].primary[0].start_sector = 0;
    //	int sectors = ((int)hdinfo[61] << 16) + hdinfo[60];
    hd_info[drive].primary[0].total_sectors = ((int)hdinfo[61] << 16) + hdinfo[60];
}

//生成读写nr_sects个扇区的命令
void make_rw_cmd(struct hd_cmd *cmd,int drive,int start_sect,int nr_sects,int flag)
{
    u8 command = flag;
    u8 device = MAKE_DEVICE_REG(1,drive,(start_sect>> 24) & 0xF);
    u8 lba_low = start_sect&0xff;
    u8 lba_mid = (start_sect>> 8)&0xff;
    u8 lba_high = (start_sect>> 16)&0xff;	
    make_cmd(cmd,0,nr_sects,lba_low,lba_mid,lba_high,device,command);

}
//读分区表所在的扇区(1个)
void get_partition_table(int drive,int start_sect,struct partition_table *entry)
{
    struct hd_cmd cmd;
    //读从start_sect开始的1个扇区
    make_rw_cmd(&cmd,drive,start_sect,1,ATA_READ);
    hd_cmd_out(&cmd);

    if(read_intr(cmd.buffer) == 0)
    {
        memcpy(entry,cmd.buffer+ PARTITION_TABLE_OFFSET, sizeof(struct partition_table) * NR_PART_PER_DRIVE);	
    }
    else
        printk("error\n");
}

//打印分区表
void print_hard_disk_partition(struct hard_disk_info *hd_info)
{
    int i;
    printk("\n\n\nhard disk partition table\n");
    printk("\n\n-----------------------------Hard Disk-----------------------------------\n");
    printk("Hard Disk\tstart sector:0x%d\t\ttotal sectors:0x%d\n",hd_info->primary[0].start_sector,hd_info->primary[0].total_sectors);
    printk("-------------------------Primary Partition-----------------------------------\n");
    for (i = 1; i < NR_PRIM_PER_DRIVE; i++) 
    {
        printk("%d\t\tstart_sector:0x%d\t\ttotal_sectors:0x%d\n",i,hd_info->primary[i].start_sector,hd_info->primary[i].total_sectors);
    }
    printk("-------------------------Logic Partition-------------------------------------\n");
    for (i = 0; i < NR_LOG_PER_DRIVE; i++) 
    {
        if (hd_info->logical[i].total_sectors == 0)
            continue;
        printk("%d\t\tstart_sector:0x%d\t\ttotal_sectors:0x%d\n",i,hd_info->logical[i].start_sector,hd_info->logical[i].total_sectors);
    }
    printk("-----------------------------------------------------------------------------\n");

}
//获取分区表
void partition(int device,int style)
{
    int i;	
    int drive = DRV_OF_DEV(device);
    struct hard_disk_info *hdi = &hd_info[drive];
    struct partition_table part_tbl[NR_LOG_PER_DRIVE];//64
    // 主分区
    if(style == P_PRIMARY)
    {
        get_partition_table(drive,drive,part_tbl);
        int nr_prim_parts = 0;
        for(i = 0;i < NR_PART_PER_DRIVE;i++)
        {
            //未用的分区表项
            if(part_tbl[i].system_id == UNUSED_PART)
            {
                continue;
            }
            nr_prim_parts++;
            int dev_nr = i + 1;
            hdi->primary[dev_nr].start_sector = part_tbl[i].start_sector_lba;
            hdi->primary[dev_nr].total_sectors = part_tbl[i].nr_sectors;
            //扩展分区
            if(part_tbl[i].system_id == EXTENDED_PART) 
            {
                partition(device + dev_nr ,P_EXTENDED);
            }
        }
    }
    else if(style == P_EXTENDED)
    {	
        int j = device % NR_PRIM_PER_DRIVE; 	 //主分区表中的第几个扩展分区
        int ext_start_sect = hdi->primary[j].start_sector; //起始扇区
        int s = ext_start_sect;
        int nr_1st_log = (j - 1) * NR_LOG_PER_PART; // 扩展分区中的首个逻辑分区
        //逻辑分区
        for(i = 0; i < NR_LOG_PER_PART;i++)
        {
            int dev_nr = nr_1st_log + i;
            get_partition_table(drive,s,part_tbl);
            hdi->logical[dev_nr].start_sector = s + part_tbl[0].start_sector_lba;
            hdi->logical[dev_nr].total_sectors = part_tbl[0].nr_sectors;
            s = ext_start_sect + part_tbl[1].start_sector_lba; 
            if(part_tbl[1].system_id == UNUSED_PART)
                break;
        }

    }
}

//打开硬盘 读取硬盘的分区表信息
void hd_open(int device)
{
    int drive = DRV_OF_DEV(device);
    assert(drive == 0);
    hd_identify(drive);	
    if(hd_info[drive].open_cnt++ == 0) 
    {
        partition(drive * (NR_PART_PER_DRIVE + 1),P_PRIMARY);
        print_hard_disk_partition(&hd_info[drive]);
    }
    //如果已经被打开了，就退出
    else
    {
        //		printk("hard disk is openning\n");
        return;
    }
}

//关闭硬盘
void hd_close(int device)
{
    int drive = DRV_OF_DEV(device);
    hd_info[drive].open_cnt--;
}

/*read and write the device from start with sects number sections*/
void hd_rw(int device,int start_sect,int nr_sects,int flag,struct buffer_head *bh)
{
    int drive = DRV_OF_DEV(MINOR(device));
    int part_index = -1;
    //	printk("device = %d\n",device);
    //逻辑分区
    if(device > MAX_PRIMARY)
    {
        //判断device是主分区还是逻辑分区,获取分区的下标
        part_index = (device - MINOR_hd1a) % NR_LOG_PER_DRIVE;
        nr_sects += hd_info[drive].logical[part_index].start_sector;
    }
    //主分区
    else
    {
        nr_sects += hd_info[drive].primary[device].start_sector;
    }
    struct hd_cmd cmd;
    //	printk("%d\t%d\t%d\t%d\n",drive,start_sect,nr_sects,flag);
    make_rw_cmd(&cmd,drive,start_sect,nr_sects,flag);
    //	printk("%d\t%d\t%d\t%d\t%d\n",cmd.count,cmd.lba_low,cmd.lba_mid,cmd.lba_high,cmd.command);
    hd_cmd_out(&cmd);
    //	bh = getblk(device,start_sect);	
    if(flag == ATA_READ)
    {	
        //	bh = getblk(device,start_sect);	
        memset(bh->b_data,0,SECTOR_SIZE);
        if(read_intr(bh->b_data) == 0)
        {
            //			disp_str("read hard disk successful\n");
        }
        else
        {
            panic("read hard disk error\n");
        }
    }
    else if(flag == ATA_WRITE)
    {
        if(write_intr(bh->b_data) == 0)
        {
            //			disp_str("write hard disk successful\n");
        }
        else
        {
            panic("write hard disk error\n");
        }
    }
    else
    {
        panic("no such operation !!!");
    }
}

//初始化硬盘
void init_hd()
{
    int i = 0;
    //初始化中断服务程序
    put_irq_handler(HD_IRQ,hd_handler);
    //enable irq 8259A从芯片
    enable_irq(CASCADE_IRQ);
    //enable irq 硬盘中断
    enable_irq(HD_IRQ);
    //打开各个磁盘 获取分区表信息
    for(;i < MAX_DRIVES;i++)
    {
        //	hd_open(i);
    }	
}

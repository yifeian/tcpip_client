/*
 * sntp.h
 *
 *  Created on: Nov 13, 2018
 *      Author: yifeifan
 */


#ifndef SNTP_H_
#define SNTP_H_
/*ntp from 1900 ,unix from 1970 */
#include "network.h"


#define JAN_1970      2208988800UL
#define	IFI_NAME	16			/* same as IFNAMSIZ in <net/if.h> */
#define	IFI_HADDR	 8			/* allow for 64-bit EUI-64 in future *

/* NTP使用的64位时间戳由秒数的32位部分和小数秒的32位部分组成 */
struct l_fixedpt            /* 64-bit fixed-point */
{
	uint32_t int_part;
	uint32_t fraction;
};

struct s_fixedpt
{
	uint16_t int_part;
	uint16_t fraction;
};

struct ntpdata               /* ntp header  */
{
	/* 跳跃指示符(2),版本号(3),模式(3)*/
	u_char  status;
	/*此字段仅在SNTP服务器消息中有意义*/
	u_char stratum;
	/* 轮询间隔仅在SNTP服务器消息中有意义*/
	u_char ppoll;
	/*精度：这是一个八位有符号整数，用作指数
      2，其结果值是系统时钟的精度，
      单位为秒。此字段仅在服务器消息中很重要，*/
	int percision:8;
	/*根延迟：这是一个32位带符号的定点数，表示
     主参考源的总往返延迟，以秒为单位
     ，分数点位于15和16之间*/
	struct s_fixedpt distance;
	/*根分散：这是一个32位无符号定点数，
   表示由时钟频率容差引起的最大误差，以
   秒为单位，分数点位于15和16之间*/
	struct s_fixedpt dispersion;
	/*参考标识符：这是一个标识
      特定参考源的32位位串。该字段仅在
      服务器消息中有意义*/
	uint32_t refid;
	/*该字段是系统时钟上次
     设置或更正的时间*/
	struct l_fixedpt reftime;
	/*This is the time at which the request departed
   the client for the server*/
	struct l_fixedpt org;
	/*this is the time at which the request arrived at
	   the server or the reply arrived at the client*/
	struct l_fixedpt rec;
	struct l_fixedpt xmt;
};



struct ifi_info {
  char    ifi_name[IFI_NAME];	/* interface name, null-terminated */
  short   ifi_index;			/* interface index */
  short   ifi_mtu;				/* interface MTU */
  u_char  ifi_haddr[IFI_HADDR];	/* hardware address */
  u_short ifi_hlen;				/* # bytes in hardware address: 0, 6, 8 */
  short   ifi_flags;			/* IFF_xxx constants from <net/if.h> */
  short   ifi_myflags;			/* our own IFI_xxx flags */
  struct sockaddr  *ifi_addr;	/* primary address */
  struct sockaddr  *ifi_brdaddr;/* broadcast address */
  struct sockaddr  *ifi_dstaddr;/* destination address */
  struct ifi_info  *ifi_next;	/* next of these structures */
};

#define VERSION_MASK  0x38
#define MODE_MASK     0x07
#define MODE_CLIENT 3
#define MODE_SERVER 4
#define MODE_BROADCAST 5
#endif /* SNTP_H_ */

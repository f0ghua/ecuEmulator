#ifndef XCOMMDEFINE_H
#define XCOMMDEFINE_H

#define PROTOCOL_ID_CONFIGURATION  	0x08
#define PROTOCOL_ID_KEYWORD_82     	0x10
#define PROTOCOL_ID_KEYWORD_71     	0x18
#define PROTOCOL_ID_IIC            	0x20
#define PROTOCOL_ID_KEYWORD_2K     	0x28
#define PROTOCOL_ID_IDB            	0x30
#define PROTOCOL_ID_ACP            	0x38
#define PROTOCOL_ID_EC             	0x40
#define PROTOCOL_ID_J1708          	0x48
#define PROTOCOL_ID_CAN1           	0x50
#define PROTOCOL_ID_CAN2           	0x58
#define PROTOCOL_ID_CLASS2         	0x60
#define PROTOCOL_ID_AOS            	0x68
#define PROTOCOL_ID_SPI            	0x70
#define PROTOCOL_ID_UPGRADE	0x78
#define PROTOCOL_ID_UART           	0x80
#define PROTOCOL_ID_J1850          	0x88
#define PROTOCOL_ID_BEAN1          	0x90
#define PROTOCOL_ID_BEAN2          	0x98
#define PROTOCOL_ID_IE_BUS         	0xB0
#define PROTOCOL_ID_LIN            	0xB8
#define PROTOCOL_ID_ISO15765_CAN1  	0xC0
#define PROTOCOL_ID_ISO15765_CAN2  	0xC8
#define PROTOCOL_ID_BLOCK_TRANSFER 	0xF8

#define BUS_CAN1                    0
#define BUS_CAN2                    1
#define BUS_LIN1                    2

#define LIN_VER_2_0                 0x20
#define LIN_VER_1_3                 0x13

#define PROTOCOL_ID_MASK        	0xF8
#define COMMAND_MASK		 		0x40
#define TXRX_MASK					0x20
#define TIMESTAMP_MASK				0x01

#define LIN_CHECKSUM_LEN            1
#define COMPLETE_CODE_LEN			1
#define TIME_STAMP_LEN			 	4
#define MSG_END_STR_LEN				2

#define DIRECTION_RX 				0
#define DIRECTION_TX 				1

#define IS_BIT_SET(data, bit)		(((data)>>(bit))&0x1)
#define SET_BIT(data, bit) 			(data) = (data)|(1<<(bit))
#define CLEAR_BIT(data, bit) 		(data) = (data)&(~(1<<(bit)))

#define IS_EXTENDED(id)             ((id & 0x1FFFF800U)?1:0)  //((id > 0x7FF)?1:0)

#define ARRAY_SIZE(x)               sizeof((x))/sizeof((x)[0])

enum {
    OPEN_SUCC   = 0,
    OPEN_ERR    = 1,
    CLOSE_SUCC  = 2,
    CLOSE_ERR   = 3,
    DEV_UNKNOWN
};

#endif // XCOMMDEFINE_H

#include "config.h"

//sfr SPSTAT          =   0xcd;                   //SPI״̬�Ĵ���
#define SPIF            0x80                    //SPSTAT.7
#define WCOL            0x40                    //SPSTAT.6
//sfr SPCTL           =   0xce;                   //SPI���ƼĴ���
#define FLASH_CSIG            0x80                    //SPCTL.7
#define SPEN            0x40                    //SPCTL.6
#define DORD            0x20                    //SPCTL.5
#define MSTR            0x10                    //SPCTL.4
#define CPOL            0x08                    //SPCTL.3
#define CPHA            0x04                    //SPCTL.2
#define SPDHH           0x00                    //CPU_CLK/4
#define SPDH            0x01                    //CPU_CLK/16
#define SPDL            0x02                    //CPU_CLK/64
#define SPDLL           0x03                    //CPU_CLK/128
//sfr SPDAT           =   0xcf;                   //SPI���ݼĴ���

//FLASH_CS
//����Flash���
#define SFC_WREN        0x06                  
#define SFC_WRDI        0x04
#define SFC_RDSR        0x05
#define SFC_WRSR        0x01
#define SFC_READ        0x03
#define SFC_FASTREAD    0x0B
#define SFC_RDID        0xAB
#define SFC_PAGEPROG    0x02
#define SFC_SECTORERASE 0x20
#define SFC_RDCR        0xA1
#define SFC_WRCR        0xF1
#define SFC_SECTORER    0xD7
#define SFC_BLOCKER     0xD8
#define SFC_CHIPER      0xC7


bit g_fFlashOK;                                //Flash״̬

bit flash_chk_id();
void flash_init()
{
  SPSTAT = SPIF | WCOL;                       //���SPI״̬
  FLASH_CS = 1;
  SPCTL = FLASH_CSIG | SPEN | MSTR;                 //����SPIΪ��ģʽ
	flash_chk_id();
}

uint8_t SpiShift(uint8_t dat)
{
    SPDAT = dat;                                //����SPI����
    while (!(SPSTAT & SPIF));                   //�ȴ�SPI���ݴ������
    SPSTAT = SPIF | WCOL;                       //���SPI״̬
    
    return SPDAT;
}

/************************************************
���Flash�Ƿ�׼������
��ڲ���: ��
���ڲ���:
    0 : û�м�⵽��ȷ��Flash
    1 : Flash׼������
************************************************/
bit flash_chk_id()
{
    uint8_t dat1;
    
    FLASH_CS = 0;
    SpiShift(SFC_RDID);                         //���Ͷ�ȡID����
    SpiShift(0x00);                             //�ն�3���ֽ�
    SpiShift(0x00);
    SpiShift(0x00);
    dat1 = SpiShift(0x00);                      //��ȡ������ID1
    FLASH_CS = 1;
                                                //����Ƿ�ΪPM25LVxxϵ�е�Flash
    g_fFlashOK = ((dat1 == 0x17));
    return g_fFlashOK;
}

/************************************************
���Flash��æ״̬
��ڲ���: ��
���ڲ���:
    0 : Flash���ڿ���״̬
    1 : Flash����æ״̬
************************************************/
bit flash_chk_busy()
{
    uint8_t dat;
    
    FLASH_CS = 0;
    SpiShift(SFC_RDSR);                         //���Ͷ�ȡ״̬����
    dat = SpiShift(0);                          //��ȡ״̬
    FLASH_CS = 1;
    
    return (dat & 0x01);                        //״ֵ̬��Bit0��Ϊæ��־
}

/************************************************
ʹ��Flashд����
��ڲ���: ��
���ڲ���: ��
************************************************/
void FlashWriteEnable()
{
    while (flash_chk_busy());                      //Flashæ���
    FLASH_CS = 0;
    SpiShift(SFC_WREN);                         //����дʹ������
    FLASH_CS = 1;
}

/************************************************
������ƬFlash
��ڲ���: ��
���ڲ���: ��
************************************************/
void flash_chip_erase()
{
    if (g_fFlashOK)
    {
        FlashWriteEnable();                     //ʹ��Flashд����
        FLASH_CS = 0;
        SpiShift(SFC_CHIPER);                   //����Ƭ��������
        FLASH_CS = 1;
    }
}

/************************************************
��Flash�ж�ȡ����
��ڲ���:
    addr   : ��ַ����
    size   : ���ݿ��С
    buffer : �����Flash�ж�ȡ������
���ڲ���:
    ��
************************************************/
void flash_read(uint32_t addr, uint32_t size, uint8_t *buffer)
{
    if (g_fFlashOK)
    {
        while (flash_chk_busy());                  //Flashæ���
        FLASH_CS = 0;
        SpiShift(SFC_FASTREAD);                 //ʹ�ÿ��ٶ�ȡ����
        SpiShift(((uint8_t *)&addr)[1]);           //������ʼ��ַ
        SpiShift(((uint8_t *)&addr)[2]);
        SpiShift(((uint8_t *)&addr)[3]);
        SpiShift(0);                            //��Ҫ�ն�һ���ֽ�
        while (size)
        {
            *buffer = SpiShift(0);              //�Զ�������ȡ������
            addr++;
            buffer++;
            size--;
        }
        FLASH_CS = 1;
    }
}

/************************************************
д���ݵ�Flash��
��ڲ���:
    addr   : ��ַ����
    size   : ���ݿ��С
    buffer : ������Ҫд��Flash������
���ڲ���: ��
************************************************/
void flash_write(uint32_t addr, uint32_t size, uint8_t *buffer)
{
    if (g_fFlashOK)
    while (size)
    {
        FlashWriteEnable();                     //ʹ��Flashд����
        FLASH_CS = 0;
        SpiShift(SFC_PAGEPROG);                 //����ҳ�������
        SpiShift(((uint8_t *)&addr)[1]);           //������ʼ��ַ
        SpiShift(((uint8_t *)&addr)[2]);
        SpiShift(((uint8_t *)&addr)[3]);
        while (size)
        {
            SpiShift(*buffer);                  //����ҳ��д
            addr++;
            buffer++;
            size--;
            if ((addr & 0xff) == 0) break;
        }
        FLASH_CS = 1;
    }
}
/************************************************
Flash ��������
��ڲ���:
    addr   : ��ַ����
���ڲ���: ��
************************************************/
void flash_erase_sector(uint32_t addr)
{
    if (g_fFlashOK)
    {
			FlashWriteEnable();                     //ʹ��Flashд����
      FLASH_CS = 0;
      SpiShift(SFC_SECTORERASE);                 //����ҳ�������
      SpiShift(((uint8_t *)&addr)[1]);           //������ʼ��ַ
      SpiShift(((uint8_t *)&addr)[2]);
      SpiShift(((uint8_t *)&addr)[3]);
      FLASH_CS = 1;
    }
}

uint8_t command_flash_program(COMMAND_ARGS)
{
	uint8_t dat[256];
	uint8_t bufferpointer = 0;
	uint32_t addr = 0;
	ES = 0;
	keep_screen_on = 1;
	terminal_add_bottom("Flash Programmer");
	terminal_add_bottom("");
	terminal_add_bottom("");
	//terminal_require_password();
	terminal_add_bottom("Erasing Flash...");
	flash_chip_erase();
	
	while (flash_chk_busy())delay10ms(10);
	terminal_add_bottom("Started");
	delay10ms(100);
	oled_stop_refresh();
	while(KEY_UP == 0);
	RI = TI = 0;
	while(KEY_UP)
	{
		if(RI)
		{
			RI = 0;
			dat[bufferpointer] = SBUF;
			++bufferpointer;
			if(bufferpointer == 0)
			{
				flash_write(addr, 256, dat);
				addr += 256;
				SBUF = 0x01;
				_nop_();
				_nop_();
				while(TI == 0);
				TI = 0;
			}
		}
	}
	keep_screen_on = 0;
	RI = TI = 0;
	ES = 1;
	terminal_add_bottom("Abort");
	sprintf(oled_tmpstr, "%lu", addr);
	terminal_add_bottom(oled_tmpstr);
	return COMMAND_OK;
}
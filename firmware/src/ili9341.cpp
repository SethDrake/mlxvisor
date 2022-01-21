#include <ili9341.h>
#include <spi.h>
#include <hardware.h>
#include <mini_fonts.h>

ILI9341::ILI9341()
{
	this->spi = NULL;
	this->color = WHITE;
	this->bgColor = BLACK;
	this->font = Consolas8x14;
	this->isLandscape = true;
}

ILI9341::~ILI9341()
{
}

void ILI9341::sendCmd(uint8_t cmd)
{
	GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, 0); //command mode
	SPIx_WriteData(spi, cmd);
}

void ILI9341::sendData(uint8_t value)
{
	GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, 1); //data mode
	SPIx_WriteData(spi, value);
}

void ILI9341::init(SPI_HandleTypeDef* spi)
{
	this->spi = spi;

	SPIx_SetPrescaler(spi, SPI1_PRESCALER_SLOW);

	GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, 0);
	GPIO_WritePin(LCD_RESET_PORT, LCD_RESET_PIN, 0);
	HAL_Delay(5);
	GPIO_WritePin(LCD_RESET_PORT, LCD_RESET_PIN, 1);

	sendCmd(ILI9341_SOFT_RESET_REG); //software reset
	HAL_Delay(5);
	sendCmd(ILI9341_DISPLAYOFF_REG); // display off

	sendCmd(ILI9341_POWERCTL1_REG); //Power control
	sendData(0x23); //VRH[5:0]

	sendCmd(ILI9341_POWERCTL2_REG); //Power control
	sendData(0x10); //SAP[2:0];BT[3:0]

	sendCmd(ILI9341_VCOMCTL1_REG); //VCM control
	sendData(0x3E); //Contrast
	sendData(0x28);

	sendCmd(ILI9341_VCOMCTL2_REG); //VCM control2
	sendData(0x86);

	sendCmd(ILI9341_MEMACCESS_REG); // Memory Access Control
	sendData(0x48); //my,mx,mv,ml,BGR,mh,0.0

	sendCmd(ILI9341_PIXFORMATSET_REG);
	sendData(0x55);

	sendCmd(ILI9341_FRAMECTL_NOR_REG);
	sendData(0x00);
	sendData(0x18);

	sendCmd(ILI9341_FUNCTONCTL_REG); // Display Function Control
	sendData(0x08);
	sendData(0x82);
	sendData(0x27);

	sendCmd(ILI9341_ENABLE_3G_REG); // 3Gamma Function Disable
	sendData(0x00);

	sendCmd(ILI9341_GAMMASET_REG); //Gamma curve selected
	sendData(0x01);

	sendCmd(ILI9341_POSGAMMACORRECTION_REG); //Set Gamma
	sendData(0x0F);
	sendData(0x31);
	sendData(0x2B);
	sendData(0x0C);
	sendData(0x0E);
	sendData(0x08);
	sendData(0x4E);
	sendData(0xF1);
	sendData(0x37);
	sendData(0x07);
	sendData(0x10);
	sendData(0x03);
	sendData(0x0E);
	sendData(0x09);
	sendData(0x00);


	sendCmd(ILI9341_NEGGAMMACORRECTION_REG); //Set Gamma
	sendData(0x00);
	sendData(0x0E);
	sendData(0x14);
	sendData(0x03);
	sendData(0x11);
	sendData(0x07);
	sendData(0x31);
	sendData(0xC1);
	sendData(0x48);
	sendData(0x08);
	sendData(0x0F);
	sendData(0x0C);
	sendData(0x31);
	sendData(0x36);
	sendData(0x0F);

	sendCmd(ILI9341_SLEEP_OUT_REG); //Exit Sleep
	HAL_Delay(100);
	sendCmd(ILI9341_DISPLAYON_REG); //Display on
	HAL_Delay(100);

	//sendCmd(ILI9341_WRITEBRIGHT_REG);   	//Change brightness
	//sendData(0x50);
	
	GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, 1);

	SPIx_SetPrescaler(spi, SPI1_PRESCALER);
}

void ILI9341::enable(uint8_t enabled)
{
	GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, 0);
	if (enabled == 0) {
		sendCmd(ILI9341_DISPLAYOFF_REG);
	}
	else {
		sendCmd(ILI9341_DISPLAYON_REG);
	}
	GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, 1);
	HAL_Delay(100);
}

void ILI9341::sleep(uint8_t enabled)
{
	GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, 0);
	if (enabled == 0) {
		sendCmd(ILI9341_SLEEP_OUT_REG);
	}
	else {
		sendCmd(ILI9341_SLEEP_ENTER_REG);
	}
	GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, 1);
	HAL_Delay(100);
}

uint32_t ILI9341::readID()
{
	uint32_t result = 0;
	GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, 0);
	sendCmd(ILI9341_READID1_REG);
	GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, 1);
	uint8_t data = (SPIx_ReadData16(spi) << 8);
	GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, 1);
	result |= data << 24;

	GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, 0);
	sendCmd(ILI9341_READID2_REG);
	GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, 1);
	data = (SPIx_ReadData16(spi) << 8);
	GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, 1);
	result |= data << 16;

	GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, 0);
	sendCmd(ILI9341_READID3_REG);
	GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, 1);
	data = (SPIx_ReadData16(spi) << 8);
	GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, 1);
	result |= data << 8;

	GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, 0);
	sendCmd(ILI9341_READID4_REG);
	GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, 1);
	data = (SPIx_ReadData16(spi) << 8);
	GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, 1);
	result |= data;
	
	return result;
}

void ILI9341::clear(uint16_t color)
{
	if (isLandscape)
	{
		fillScreen(0, 0, ILI9341_LCD_WIDTH - 1, ILI9341_LCD_HEIGHT - 1, color);
	}
	else
	{
		fillScreen(0, 0, ILI9341_LCD_HEIGHT - 1, ILI9341_LCD_WIDTH - 1, color);
	}
}

void ILI9341::setWindow(uint16_t startX, uint16_t startY, uint16_t stopX, uint16_t stopY)
{
}

void ILI9341::pixelDraw(uint16_t xpos, uint16_t ypos, uint16_t color)
{
}

void ILI9341::lineDraw(uint16_t ypos, uint16_t* line, uint32_t size)
{
}

void ILI9341::fillScreen(uint16_t xstart, uint16_t ystart, uint16_t xstop, uint16_t ystop, uint16_t color)
{
}

void ILI9341::putChar(uint16_t x, uint16_t y, uint8_t chr, uint16_t charColor, uint16_t bkgColor)
{
}

void ILI9341::printf(uint16_t x, uint16_t y, const char* format, ...)
{
}

void ILI9341::printf(uint16_t x, uint16_t y, uint16_t charColor, uint16_t bkgColor, const char* format, ...)
{
}

void ILI9341::bufferDraw(uint16_t x, uint16_t y, uint16_t xsize, uint16_t ysize, uint16_t* buf)
{
}

void ILI9341::drawBorder(uint16_t xpos, uint16_t ypos, uint16_t width, uint16_t height, uint16_t borderWidth,
                         uint16_t color)
{
}

void ILI9341::setColor(uint16_t color, uint16_t bgColor)
{
}

void ILI9341::setFont(const unsigned char font[])
{
}

void ILI9341::setLandscape()
{
}

void ILI9341::setPortrait()
{
}

void ILI9341::setCol(uint16_t StartCol, uint16_t EndCol)
{
}

void ILI9341::setPage(uint16_t StartPage, uint16_t EndPage)
{
}

void ILI9341::putString(const char str[], uint16_t x, uint16_t y, uint16_t charColor, uint16_t bkgColor)
{
}

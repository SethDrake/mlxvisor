#include <ili9341.h>
#include <spi.h>
#include <hardware.h>
#include <mini_fonts.h>
#include <stdio.h>
#include <stdarg.h>

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

void ILI9341::setCS(uint8_t val)
{
	GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, val);
}

void ILI9341::setDC(uint8_t val)
{
	GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, val);
}

// void ILI9341::setRESET(uint8_t val)
// {
// 	GPIO_WritePin(LCD_RESET_PORT, LCD_RESET_PIN, val);
// }

void ILI9341::sendCmd(uint8_t cmd)
{
	setDC(0); //command mode
	SPIx_WriteData(spi, cmd);
}

void ILI9341::sendData(uint8_t value)
{
	setDC(1); //data mode
	SPIx_WriteData(spi, value);
}

void ILI9341::init(SPI_HandleTypeDef* spi)
{
	this->spi = spi;

	//SPIx_Enable(spi);
	SPIx_SetPrescaler(spi, SPI1_PRESCALER_SLOW);

	setCS(0);
	/*setRESET(0);
	HAL_Delay(5);
	setRESET(1);*/

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
	// sendData(0x48); //my,mx,mv,ml,BGR,mh,0.0
	sendData(0x88); //my,mx,mv,ml,BGR,mh,0.0 - 10001000

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
	
	setCS(1);

	SPIx_SetPrescaler(spi, SPI1_PRESCALER);
}

void ILI9341::enable(uint8_t enabled)
{
	setCS(0);
	if (enabled == 0) {
		sendCmd(ILI9341_DISPLAYOFF_REG);
	}
	else {
		sendCmd(ILI9341_DISPLAYON_REG);
	}
	setCS(1);
	HAL_Delay(100);
}

void ILI9341::sleep(uint8_t enabled)
{
	setCS(0);
	if (enabled == 0) {
		sendCmd(ILI9341_SLEEP_OUT_REG);
	}
	else {
		sendCmd(ILI9341_SLEEP_ENTER_REG);
	}
	setCS(1);
	HAL_Delay(100);
}

uint32_t ILI9341::readID()
{
	uint32_t result = 0;

	setCS(0);
	sendCmd(ILI9341_IDENTINFO_R_REG);
	SPIx_ReadBuffer(spi, (uint8_t*)&result, 4);
	setDC(1);
	setCS(1);
	
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

void ILI9341::setCol(uint16_t StartCol, uint16_t EndCol)
{
	sendCmd(ILI9341_COLADDRSET_REG); // Column Command address
	sendData(StartCol >> 8);
	sendData(StartCol & 0xFF);
	sendData(EndCol >> 8);
	sendData(EndCol & 0xFF);
}

void ILI9341::setPage(uint16_t StartPage, uint16_t EndPage)
{
	sendCmd(ILI9341_PAGEADDRSET_REG); // Page Command address
	sendData(StartPage >> 8);
	sendData(StartPage & 0xFF);
	sendData(EndPage >> 8);
	sendData(EndPage & 0xFF);
}

void ILI9341::setWindow(uint16_t startX, uint16_t startY, uint16_t stopX, uint16_t stopY)
{
	if (isLandscape)
	{
		setPage(startX, stopX);
		setCol(startY, stopY);	
	}
	else
	{
		setPage(ILI9341_LCD_WIDTH - 1 - stopY, ILI9341_LCD_WIDTH - 1 - startY);
		setCol(startX, stopX);	
	}
}

void ILI9341::pixelDraw(uint16_t xpos, uint16_t ypos, uint16_t color)
{
	setCS(0); // CS=0;
	setWindow(xpos, ypos, xpos, ypos);
	sendCmd(ILI9341_MEMORYWRITE_REG);
	sendData(color >> 8);
	sendData(color & 0xFF);
	setCS(1); // CS=1;
}

void ILI9341::lineDraw(uint16_t ypos, uint16_t* line, uint32_t size)
{
	bufferDraw(0, ypos, size, 1, line);
}

void ILI9341::fillScreen(uint16_t xstart, uint16_t ystart, uint16_t xstop, uint16_t ystop, uint16_t color)
{
	uint32_t pixels = (xstop - xstart + 1) * (ystop - ystart + 1);
	setCS(0); // CS=0;
	setWindow(xstart, ystart, xstop, ystop);
	sendCmd(ILI9341_MEMORYWRITE_REG);

	setDC(1);

	SPIx_SetDataSize(spi, SPI_DATASIZE_16BIT);
	SPIx_Enable(spi);
	while (pixels) {
		 SPIx_WriteData16(spi, color);
		pixels--;
	}
	SPIx_SetDataSize(spi, SPI_DATASIZE_8BIT);
	setCS(1); // CS=1;
}

void ILI9341::putChar(uint16_t x, uint16_t y, uint8_t chr, uint16_t charColor, uint16_t bkgColor)
{
	uint8_t i, j;
	
	const uint8_t f_width = font[0];	
	const uint8_t f_height = font[1];
	const uint16_t f_bytes = (f_width * f_height / 8);
	
	uint16_t t = 0;
	uint16_t charbuf[(f_width + 1) * f_height];
	
	//fill charbuf
	if (isLandscape)
	{
		for (i = 0; i < f_width; i++)
		{
			for (j = 0; j < f_height; j++) {
				const uint16_t bitNumberGlobal = f_width * (f_height - j) + (f_width - i);
				const uint16_t byteNumberLocal = (bitNumberGlobal / 8);
				const uint8_t bitNumberInByte = bitNumberGlobal - byteNumberLocal * 8;
				uint8_t glyphByte = font[(chr - 0x20) * f_bytes + byteNumberLocal + 2];
				const uint8_t mask = 1 << bitNumberInByte;
				if (glyphByte & mask) {
					charbuf[t++] = charColor;
				}
				else 
				{
					charbuf[t++] = bkgColor;
				}
			}
		}
		for (j = 0; j < f_height; j++) {
			//vertical empty line right from symbol
			charbuf[t++] = bkgColor;
		}
	}
	else //portrait
	{
		for (i = 0; i < f_height; i++)
		{
			for (j = 0; j < f_width; j++) {
				const uint16_t bitNumberGlobal = f_width * i + (f_width - j);
				const uint16_t byteNumberLocal = (bitNumberGlobal / 8);
				const uint8_t bitNumberInByte = bitNumberGlobal - byteNumberLocal * 8;
				uint8_t glyphByte = font[(chr - 0x20) * f_bytes + byteNumberLocal + 2];
				const uint8_t mask = 1 << bitNumberInByte;
				if (glyphByte & mask) {
					charbuf[t++] = charColor;
				}
				else 
				{
					charbuf[t++] = bkgColor;
				}
			}
			charbuf[t++] = bkgColor; //vertical empty line right from symbol
		}
		y -= 3;
	}
	
	bufferDraw(x, y, f_width + 1, f_height, charbuf);
}

void ILI9341::putString(const char str[], uint16_t x, uint16_t y, uint16_t charColor, uint16_t bkgColor)
{
	while (*str != 0) {
		putChar(x, y, *str, charColor, bkgColor);
		x += font[0]; //increment to font width
		str++;
	}
}

void ILI9341::printf(uint16_t x, uint16_t y, const char* format, ...)
{
	char buf[40];
	va_list args;
	va_start(args, format);
	// ReSharper disable once CppLocalVariableMightNotBeInitialized
	vsprintf(buf, format, args);
	putString(buf, x, y, color, bgColor);
}

void ILI9341::printf(uint16_t x, uint16_t y, uint16_t charColor, uint16_t bkgColor, const char* format, ...)
{
	char buf[40];
	va_list args;
	va_start(args, format);
	// ReSharper disable once CppLocalVariableMightNotBeInitialized
	vsprintf(buf, format, args);
	putString(buf, x, y, charColor, bkgColor);
}

void ILI9341::bufferDraw(uint16_t x, uint16_t y, uint16_t xsize, uint16_t ysize, uint16_t* buf)
{
	setCS(0); // CS=0
	setWindow(x, y, x + xsize - 1, y + ysize - 1);
	sendCmd(ILI9341_MEMORYWRITE_REG);
	setDC(1);

	//SPIx_SetDataSize(spi, SPI_DATASIZE_16BIT);
	//SPIx_Enable(spi);
	SPIx_WriteBufferDMA(spi, buf, xsize * ysize);
	/*for (uint32_t l = 0; l < xsize * ysize; l++) {
		SPIx_WriteData16(spi, buf[l]);
	}*/
	//SPIx_SetDataSize(spi, SPI_DATASIZE_8BIT);
	setCS(1); // CS=1;
}

void ILI9341::drawBorder(uint16_t xpos, uint16_t ypos, uint16_t width, uint16_t height, uint16_t borderWidth,
                         uint16_t color)
{
	fillScreen(xpos, ypos, xpos + borderWidth, ypos + height, color);
	fillScreen(xpos + borderWidth, ypos + height - borderWidth, xpos + width, ypos + height, color);
	fillScreen(xpos + width - borderWidth, ypos, xpos + width, ypos + height - borderWidth, color);
	fillScreen(xpos + borderWidth, ypos, xpos + width - borderWidth, ypos + borderWidth, color);
}

void ILI9341::setColor(uint16_t color, uint16_t bgColor)
{
	this->color = color;
	this->bgColor = bgColor;
}

void ILI9341::setFont(const unsigned char font[])
{
	this->font = font;
}

void ILI9341::setLandscape()
{
	this->isLandscape = true;
}

void ILI9341::setPortrait()
{
	this->isLandscape = false;
}

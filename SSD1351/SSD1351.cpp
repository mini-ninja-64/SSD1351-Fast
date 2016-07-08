#include "SSD1351.h"
#include <SPI.h>
#include <avr/pgmspace.h>

#define width 128
#define height 96
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }

SPISettings spiSettings =  SPISettings(300000000000000, MSBFIRST, SPI_MODE3);

//cs = oc
//rs = dc
//rst = rst

SSD1351::SSD1351(uint8_t oc, uint8_t dc, uint8_t rst){
	_oc = oc;
	_dc = dc;
	_rst = rst;

	ocport = portOutputRegister(digitalPinToPort(oc));
	ocpinmask =  digitalPinToBitMask(oc);

	dcport = portOutputRegister(digitalPinToPort(dc));
	dcpinmask = digitalPinToBitMask(dc);
}

void SSD1351::writeCommand(uint8_t command){
    *dcport &= ~ dcpinmask;

	*ocport &= ~ ocpinmask;

	SPI.transfer(command);

	*ocport |= ocpinmask;
}

void SSD1351::writeData16(uint16_t data){
    *dcport |= dcpinmask;

    *ocport &= ~ (ocpinmask);

    SPI.transfer16(data);

    *ocport |= ocpinmask;
}

void SSD1351::writeData(uint8_t data){
    *dcport |= dcpinmask;

	*ocport &= ~ (ocpinmask);

	SPI.transfer(data);

	*ocport |= ocpinmask;
}

void SSD1351::begin(void){
    frameRecording = false;
	pinMode(_dc, OUTPUT);

	SPI.begin();
	SPI.setDataMode(SPI_MODE3);

	pinMode(_oc, OUTPUT);
	digitalWrite(_oc, LOW);

	pinMode(_rst, OUTPUT);
	digitalWrite(_rst, HIGH);
	delay(500);
	digitalWrite(_rst, LOW);
	delay(500);
	digitalWrite(_rst, HIGH);
	delay(500);

	 writeCommand(0xFD);  // set command lock
    writeData(0x12);  
    writeCommand(0xFD);  // set command lock
    writeData(0xB1);

    writeCommand(0xAE);  		// 0xAE

    writeCommand(0xB3);  		// 0xB3
    writeCommand(0xF1);  						// 7:4 = Oscillator Frequency, 3:0 = CLK Div Ratio (A[3:0]+1 = 1..16)
    
    writeCommand(0xCA);
    writeData(127);
    
    writeCommand(0xA0);
    writeData(0x74);
  
    writeCommand(0x15);
    writeData(0x00);
    writeData(0x7F);
    writeCommand(0x75);
    writeData(0x00);
    writeData(0x7F);

    writeCommand(0xA1); 		// 0xA1
    writeData(96);



    writeCommand(0xA2); 	// 0xA2
    writeData(0x0);

    writeCommand(0xB5);
    writeData(0x00);
    
    writeCommand(0xAB);
    writeData(0x01); // internal (diode drop)
    //writeData(0x01); // external bias

    //    writeCommand(SSSD1351_CMD_SETPHASELENGTH);
     //    writeData(0x32);

    writeCommand(0xB1);  		// 0xB1
    writeCommand(0x32);
 
    writeCommand(0xBE);  			// 0xBE
    writeCommand(0x05);

    writeCommand(0xA6);  	// 0xA6

    writeCommand(0xC1);
    writeData(0xC8);
    writeData(0x80);
    writeData(0xC8);

    writeCommand(0xC7);
    writeData(0x0F);

    writeCommand(0xB4);
    writeData(0xA0);
    writeData(0xB5);
    writeData(0x55);
    
    writeCommand(0xB6);
    writeData(0x01);
    
    writeCommand(0xAF);
}

void SSD1351::toggleFrameRecording(void){
    frameRecording = !frameRecording;
}

void SSD1351::removePreviousFrame(void){
    if (frameRecording){
        uint16_t currCol;
        bool bad = false;

        SPI.beginTransaction(spiSettings);
        *ocport &= ~ ocpinmask;

        *dcport &= ~ dcpinmask;
        SPI.transfer(0x15);

        *dcport |= dcpinmask;
        SPI.transfer(0);
        SPI.transfer(128-1);

        *dcport &= ~ dcpinmask;
        SPI.transfer(0x75);

        *dcport |= dcpinmask;
        SPI.transfer(0);
        SPI.transfer(96-1);
        
        for (uint16_t y=0; y < 96; y++) {
            for (uint16_t x=0; x < 128; x++) {
                //get currCol
                *dcport &= ~ dcpinmask;
                SPI.transfer(0x5D);

                *dcport |= dcpinmask;

                currCol = SPI.transfer16(0x0000);
                //delayMicroseconds(10);

                if (currCol != 0x0000){
                    /*Serial.print("#####################################ERROR##################################### ");
                    Serial.print("{");
                    Serial.print(x);
                    Serial.print(", ");
                    Serial.print(y);
                    Serial.println("}");
                    Serial.println(currCol, HEX);*/
                    bad = true;
                }
            }
        }

        *ocport |= ocpinmask;

        SPI.endTransaction();

        if (bad){
            Serial.println("BAD RAM!");
        }else{
            Serial.println("GOOD RAM!");
        }
    }
}


void SSD1351::rectFill(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t colour){
    SPI.beginTransaction(spiSettings);
    *ocport &= ~ ocpinmask;
    *dcport &= ~ dcpinmask;
    SPI.transfer(0x15);

    *dcport |= dcpinmask;
    SPI.transfer(x);
    SPI.transfer(x+w-1);

    *dcport &= ~ dcpinmask;
    SPI.transfer(0x75);

    *dcport |= dcpinmask;
    SPI.transfer(y);
    SPI.transfer(y+h-1);

    *dcport &= ~ dcpinmask;
    SPI.transfer(0x5C);

    *dcport |= dcpinmask;

    for (uint16_t i=0; i < 128*96; i++) {
        SPI.transfer16(colour);
    }

    *ocport |= ocpinmask;

    SPI.endTransaction();
}

void SSD1351::fill(uint16_t colour){
    SPI.beginTransaction(spiSettings);
    *ocport &= ~ ocpinmask;
    *dcport &= ~ dcpinmask;
    SPI.transfer(0x15);

    *dcport |= dcpinmask;
    SPI.transfer(0);
    SPI.transfer(128-1);

    *dcport &= ~ dcpinmask;
    SPI.transfer(0x75);

    *dcport |= dcpinmask;
    SPI.transfer(0);
    SPI.transfer(96-1);

    *dcport &= ~ dcpinmask;
    SPI.transfer(0x5C);

    *dcport |= dcpinmask;

    for (uint16_t i=0; i < 128*96; i++) {
        SPI.transfer16(colour);
    }

    *ocport |= ocpinmask;

    SPI.endTransaction();
}

void SSD1351::line(uint8_t x0, uint8_t y0,uint8_t x1, uint8_t y1,uint16_t colour){

    SPI.beginTransaction(spiSettings);
    *ocport &= ~ ocpinmask;

    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
      if (steep) {
        _swap_int16_t(x0, y0);
        _swap_int16_t(x1, y1);
      }

      if (x0 > x1) {
        _swap_int16_t(x0, x1);
        _swap_int16_t(y0, y1);
      }

      int16_t dx, dy;
      dx = x1 - x0;
      dy = abs(y1 - y0);

      int16_t err = dx / 2;
      int16_t ystep;

      if (y0 < y1) {
        ystep = 1;
      } else {
        ystep = -1;
      }

      for (; x0<=x1; x0++) {
        if (steep) {
          pixelCont(y0, x0, colour);
        } else {
          pixelCont(x0, y0, colour);
        }
        err -= dy;
        if (err < 0) {
          y0 += ystep;
          err += dx;
        }
      }

    *ocport |= ocpinmask;
    SPI.endTransaction();
}

void SSD1351::circle(uint8_t xc, uint8_t yc, uint8_t r, uint16_t colour){
    SPI.beginTransaction(spiSettings);
    *ocport &= ~ ocpinmask;

    unsigned int x= r, y= 0;//local coords
    int          cd2= 0;    //current distance squared - radius squared

    if (!r) return;
    pixelCont(xc-r, yc, colour);
    pixelCont(xc+r, yc, colour);
    pixelCont(xc, yc-r, colour);
    pixelCont(xc, yc+r, colour);

    while (x > y)    //only formulate 1/8 of circle
    {
        cd2-= (--x) - (++y);
        if (cd2 < 0) cd2+=x++;

        pixelCont(xc-x, yc-y, colour);//upper left left
        pixelCont(xc-y, yc-x, colour);//upper upper left
        pixelCont(xc+y, yc-x, colour);//upper upper right
        pixelCont(xc+x, yc-y, colour);//upper right right
        pixelCont(xc-x, yc+y, colour);//lower left left
        pixelCont(xc-y, yc+x, colour);//lower lower left
        pixelCont(xc+y, yc+x, colour);//lower lower right
        pixelCont(xc+x, yc+y, colour);//lower right right
    }

    *ocport |= ocpinmask;
    SPI.endTransaction();
}

void SSD1351::pixel(uint8_t x, uint8_t y, uint16_t colour,uint16_t* ram = NULL){

    if(ram == NULL){
        ram[y * 96 + y] = colour;
    }else{
        SPI.beginTransaction(spiSettings);
        *ocport &= ~ ocpinmask;
        *dcport &= ~ dcpinmask;
        SPI.transfer(0x15);

        *dcport |= dcpinmask;
        SPI.transfer(x);
        SPI.transfer(x);

        *dcport &= ~ dcpinmask;
        SPI.transfer(0x75);

        *dcport |= dcpinmask;
        SPI.transfer(y);
        SPI.transfer(y);

        *dcport &= ~ dcpinmask;
        SPI.transfer(0x5C);

        *dcport |= dcpinmask;
        SPI.transfer16(colour);

        *ocport |= ocpinmask;

        SPI.endTransaction();
    }
}

void SSD1351::pixelCont(uint8_t x, uint8_t y, uint16_t colour){

    if ((x < width) || (y < height)){
        *dcport &= ~ dcpinmask;
        SPI.transfer(0x15);

        *dcport |= dcpinmask;
        SPI.transfer(x);
        SPI.transfer(x);

        *dcport &= ~ dcpinmask;
        SPI.transfer(0x75);

        *dcport |= dcpinmask;
        SPI.transfer(y);
        SPI.transfer(y);

        *dcport &= ~ dcpinmask;
        SPI.transfer(0x5C);

        *dcport |= dcpinmask;
        SPI.transfer16(colour);
    }
}
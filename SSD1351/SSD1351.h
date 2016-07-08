#include "Arduino.h"

class SSD1351 {
	public:
		SSD1351(uint8_t oc, uint8_t dc, uint8_t rst);
		void begin(void);

		void toggleFrameRecording(void);
		void removePreviousFrame(void);

		//drawing functions
		void pixel(uint8_t x, uint8_t y, uint16_t colour,uint16_t* ram);
		void fill(uint16_t colour);
		void rectFill(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t colour);
		void line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t colour);
		void circle(uint8_t xc, uint8_t yc, uint8_t r, uint16_t colour);
	private:
		void writeCommand(uint8_t command);
		void writeData(uint8_t data);
		void writeData16(uint16_t data);
		void pixelCont(uint8_t x, uint8_t y, uint16_t colour);

		int _oc, _dc, _rst, currentFrameID;
		bool frameRecording;
		uint8_t volatile *ocport, *dcport;
		uint8_t ocpinmask, dcpinmask;
};
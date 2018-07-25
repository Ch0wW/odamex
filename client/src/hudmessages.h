#ifndef __HUDMESSAGES_H__
#define __HUDMESSAGES_H__


#include "dobject.h"
#include "m_fixed.h"
#include "../client/src/v_text.h"

class DHUDMessage : public DObject
{
public:
	DHUDMessage(const char *text, float x, float y, EColorRange textColor,
		float holdTime);
	virtual ~DHUDMessage();

protected:
	brokenlines_t * Lines;
	int Width, Height, NumLines;
	float Left, Top;
	bool CenterX;
	int HoldTics;
	int Tics;
	int State;
	EColorRange TextColor;
	//FFont *Font;

	DHUDMessage() : SourceText(NULL) {}

private:
	DHUDMessage * Next;
	DWORD SBarID;
	char *SourceText;

};

#endif
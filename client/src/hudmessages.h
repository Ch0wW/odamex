#ifndef __HUDMESSAGES_H__
#define __HUDMESSAGES_H__


#include "dobject.h"
#include "m_fixed.h"
#include "v_font.h"

class DHUDMessage : DObject
{
	DECLARE_CLASS(DHUDMessage, DObject)

public:
	DHUDMessage(const char *text, float x, float y, EColorRange textColor, float holdTime);
	virtual ~DHUDMessage();

	virtual void Serialize(FArchive &arc);

	void Draw(int bottom);
	virtual void ResetText(const char *text);
	virtual void DrawSetup();
	virtual void DoDraw(int linenum, int x, int y, int xscale, int yscale, bool clean);
	virtual bool Tick();	// Returns true to indicate time for removal

protected:
	brokenlines_t *Lines;
	int Width, Height, NumLines;
	float Left, Top;
	bool CenterX;
	int HoldTics;
	int Tics;
	int State;
	EColorRange TextColor;
	FFont *Font;

	DHUDMessage() : SourceText(NULL) {}

private:
	DHUDMessage * Next;
	DWORD SBarID;
	char *SourceText;

};

#endif
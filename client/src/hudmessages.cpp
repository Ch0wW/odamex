#include "doomdef.h"
#include "hudmessages.h"
#include "c_cvars.h"
#include "v_text.h"
#include "cmdlib.h"

#include "i_video.h"
#include "v_video.h"

int V_TextScaleXAmount();
int V_TextScaleYAmount();

EXTERN_CVAR(hud_scaletext)

IMPLEMENT_CLASS(DHUDMessage, DObject)

DHUDMessage::DHUDMessage(const char *text, float x, float y, EColorRange textColor, float holdTime)
{
	if (fabs(x) > 2.f)
	{
		CenterX = true;
		Left = 0.5f;
	}
	else
	{
		Left = x < -1.f ? x + 1.f : x > 1.f ? x - 1.f : x;
		if (fabs(x) > 1.f)
		{
			CenterX = true;
		}
		else
		{
			CenterX = false; 
		}
	}
	Top = y;
	Next = NULL;
	Lines = NULL;
	HoldTics = (int)(holdTime * TICRATE);
	Tics = 0;
	TextColor = textColor;
	State = 0;
	SourceText = copystring(text);
	Font = screen->Font;
	ResetText(SourceText);
}

DHUDMessage::~DHUDMessage()
{
	if (Lines)
	{
		V_FreeBrokenLines(Lines);
		Lines = NULL;
		//BorderNeedRefresh = screen->GetPageCount();
	}
	if (SourceText != NULL)
	{
		delete[] SourceText;
	}
}

//============================================================================
//
// DHUDMessage :: Tick
//
//============================================================================

bool DHUDMessage::Tick()
{
	Tics++;
	if (HoldTics != 0 && HoldTics <= Tics)
	{ // This message has expired
		return true;
	}
	return false;
}

//============================================================================
//
// DHUDMessage :: Serialize
//
//============================================================================

void DHUDMessage::Serialize(FArchive &arc)
{
	Super::Serialize(arc);
	arc << Left << Top << CenterX << HoldTics
		<< Tics << State << TextColor
		<< SBarID << SourceText << Font << Next;
	if (arc.IsLoading())
	{
		Lines = NULL;
		ResetText(SourceText);
	}
}

void DHUDMessage::DrawSetup()
{
}

void DHUDMessage::DoDraw(int linenum, int x, int y, int xscale, int yscale, bool clean)
{
	if (clean)
	{
		screen->DrawTextClean(TextColor, x, y, Lines[linenum].string);
	}
	else
	{
		screen->DrawText(TextColor, x, y, Lines[linenum].string);
	}
}

void DHUDMessage::ResetText(const char *text)
{
	screen->SetFont(Font);

	Lines = V_BreakLines(I_GetSurfaceWidth() / V_TextScaleXAmount(), (byte *)text);

	NumLines = 0;
	Width = 0;
	Height = 0;

	if (Lines)
	{
		for (; Lines[NumLines].width != -1; NumLines++)
		{
			Height += 16;
			Width = MAX<int>(Width, Lines[NumLines].width);
		}
	}

	screen->SetFont(SmallFont);
}

void DHUDMessage::Draw(int bottom)
{
	int xscale, yscale;
	int x, y;
	int ystep;
	int i;
	bool clean;

	screen->SetFont(Font);
	DrawSetup();

	if ((clean = hud_scaletext))
	{
		xscale = CleanXfac;
		yscale = CleanYfac;
	}
	else
	{
		xscale = yscale = 1;
	}

	if (Left > 0.f)
	{
		x = (int)((float)(I_GetSurfaceWidth() - Width * xscale) * Left);
	}
	else
	{
		x = (int)((float)I_GetSurfaceWidth() * -Left);
	}
	if (CenterX)
	{
		x += Width * xscale / 2;
	}

	if (Top > 0.f)
	{
		y = (int)((float)(bottom - Height * yscale) * Top);
	}
	else
	{
		y = (int)((float)bottom * -Top);
	}

	ystep = 16 * yscale;				// FIXME : No idea why 16 

	for (i = 0; i < NumLines; i++)
	{
		int drawx;

		drawx = CenterX ? x - Lines[i].width*xscale / 2 : x;
		DoDraw(i, drawx, y, xscale, yscale, clean);
		y += ystep;
	}

	screen->SetFont(SmallFont);
}
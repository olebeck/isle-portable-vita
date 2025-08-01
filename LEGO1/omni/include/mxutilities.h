#ifndef MXUTILITIES_H
#define MXUTILITIES_H

#include "lego1_export.h"
#include "mxtypes.h"

#include <SDL3/SDL_stdinc.h>
#include <string.h>

struct LegoSdlEvents {
	Uint32 m_windowsMessage;
	Uint32 m_presenterProgress;
	Uint32 m_hitActor;
};

LEGO1_EXPORT extern LegoSdlEvents g_legoSdlEvents;

class MxDSFile;
class MxDSObject;
class MxDSAction;
class MxCompositePresenterList;
class MxPresenter;

template <class T>
inline T Abs(T p_t)
{
	return p_t < 0 ? -p_t : p_t;
}

template <class T>
inline T Min(T p_t1, T p_t2)
{
	return p_t1 < p_t2 ? p_t1 : p_t2;
}

template <class T>
inline T Max(T p_t1, T p_t2)
{
	return p_t1 > p_t2 ? p_t1 : p_t2;
}

template <typename T>
T UnalignedRead(MxU8* p_source)
{
	T value;
	memcpy(&value, p_source, sizeof(T));
	return value;
}

template <class T>
inline void GetScalar(MxU8*& p_source, T& p_dest)
{
	p_dest = *(T*) p_source;
	p_source += sizeof(T);
}

template <class T>
inline T GetScalar(T*& p_source)
{
	T val = *p_source;
	p_source += 1;
	return val;
}

template <class T>
inline void GetDouble(MxU8*& p_source, T& p_dest)
{
	p_dest = *(double*) p_source;
	p_source += sizeof(double);
}

template <class T>
inline void GetString(MxU8*& p_source, char*& p_dest, T* p_obj, void (T::*p_setter)(const char*))
{
	(p_obj->*p_setter)((char*) p_source);
	p_source += strlen(p_dest) + 1;
}

MxBool GetRectIntersection(
	MxS32 p_rect1Width,
	MxS32 p_rect1Height,
	MxS32 p_rect2Width,
	MxS32 p_rect2Height,
	MxS32* p_rect1Left,
	MxS32* p_rect1Top,
	MxS32* p_rect2Left,
	MxS32* p_rect2Top,
	MxS32* p_width,
	MxS32* p_height
);

void MakeSourceName(char*, const char*);
void OmniError(const char* p_message, MxS32 p_status);
void SetOmniUserMessage(void (*p_omniUserMessage)(const char*, MxS32));
MxBool ContainsPresenter(MxCompositePresenterList& p_presenterList, MxPresenter* p_presenter);
void ApplyMask(MxDSAction* p_action, MxU32 p_mask, MxBool p_setFlags);
MxBool KeyValueStringParse(char*, const char*, const char*);

// TEMPLATE: BETA10 0x1012dfd0
// ?Max@@YAHHH@Z

// TEMPLATE: BETA10 0x1012dff0
// ?Min@@YAHHH@Z

#endif // MXUTILITIES_H

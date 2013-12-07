#ifndef __VLC_H__
#define __VLC_H__

struct VLC
{
	uint32_t code;
	uint8_t len;
};

struct EVENT
{
	uint8_t last;
	uint8_t run;
	int8_t level;
};

struct REVERSE_EVENT
{
	uint8_t len;
	EVENT Event;
};

struct VLC_TABLE
{
	VLC vlc;
	EVENT Event;
};

#define VLC_ERROR	(-1)

#define ESCAPE  3
#define ESCAPE1 6
#define ESCAPE2 14
#define ESCAPE3 15

#endif
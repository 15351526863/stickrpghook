#pragma once
#include "Vector.h"
#include "checksum_crc32.h"

#define MAX_SPLITSCREEN_CLIENT_BITS 2
// this should == MAX_JOYSTICKS in InputEnums.h
#define MAX_SPLITSCREEN_CLIENTS	( 1 << MAX_SPLITSCREEN_CLIENT_BITS ) // 4

#define JOYSTICK_BUTTON_INTERNAL( _joystick, _button ) ( JOYSTICK_FIRST_BUTTON + ((_joystick) * JOYSTICK_MAX_BUTTON_COUNT) + (_button) )
#define JOYSTICK_POV_BUTTON_INTERNAL( _joystick, _button ) ( JOYSTICK_FIRST_POV_BUTTON + ((_joystick) * JOYSTICK_POV_BUTTON_COUNT) + (_button) )
#define JOYSTICK_AXIS_BUTTON_INTERNAL( _joystick, _button ) ( JOYSTICK_FIRST_AXIS_BUTTON + ((_joystick) * JOYSTICK_AXIS_BUTTON_COUNT) + (_button) )

#define JOYSTICK_BUTTON( _joystick, _button ) ( (ButtonCode_t)JOYSTICK_BUTTON_INTERNAL( _joystick, _button ) )
#define JOYSTICK_POV_BUTTON( _joystick, _button ) ( (ButtonCode_t)JOYSTICK_POV_BUTTON_INTERNAL( _joystick, _button ) )
#define JOYSTICK_AXIS_BUTTON( _joystick, _button ) ( (ButtonCode_t)JOYSTICK_AXIS_BUTTON_INTERNAL( _joystick, _button ) )

#define IN_ATTACK        (1 << 0)
#define IN_JUMP          (1 << 1)
#define IN_DUCK          (1 << 2)
#define IN_FORWARD       (1 << 3)
#define IN_BACK          (1 << 4)
#define IN_USE           (1 << 5)
#define IN_CANCEL        (1 << 6)
#define IN_LEFT          (1 << 7)
#define IN_RIGHT         (1 << 8)
#define IN_MOVELEFT      (1 << 9)
#define IN_MOVERIGHT     (1 << 10)
#define IN_ATTACK2       (1 << 11)
#define IN_RUN           (1 << 12)
#define IN_RELOAD        (1 << 13)
#define IN_ALT1          (1 << 14)
#define IN_ALT2          (1 << 15)
#define IN_SCORE         (1 << 16) // Used by client.dll for when scoreboard is held down
#define IN_SPEED         (1 << 17) // Player is holding the speed key
#define IN_WALK          (1 << 18) // Player holding walk key
#define IN_ZOOM          (1 << 19) // Zoom key for HUD zoom
#define IN_WEAPON1       (1 << 20) // weapon defines these bits
#define IN_WEAPON2       (1 << 21) // weapon defines these bits
#define IN_BULLRUSH      (1 << 22)
#define IN_GRENADE1      (1 << 23) // grenade 1
#define IN_GRENADE2      (1 << 24) // grenade 2
#define IN_ATTACK3       (1 << 25)

class bf_read;
class bf_write;

class CUserCmd
{
public:
	CUserCmd()
	{
		memset(this, 0, sizeof(*this));
	};
	virtual ~CUserCmd() {};

	CRC32_t GetChecksum(void) const
	{
		CRC32_t crc;
        CRC32::Init(&crc);

		CRC32::ProcessBuffer(&crc, &command_number, sizeof(command_number));
        CRC32::ProcessBuffer(&crc, &tick_count, sizeof(tick_count));
        CRC32::ProcessBuffer(&crc, &viewangles, sizeof(viewangles));
        CRC32::ProcessBuffer(&crc, &aimdirection, sizeof(aimdirection));
        CRC32::ProcessBuffer(&crc, &forwardmove, sizeof(forwardmove));
        CRC32::ProcessBuffer(&crc, &sidemove, sizeof(sidemove));
        CRC32::ProcessBuffer(&crc, &upmove, sizeof(upmove));
        CRC32::ProcessBuffer(&crc, &buttons, sizeof(buttons));
        CRC32::ProcessBuffer(&crc, &impulse, sizeof(impulse));
        CRC32::ProcessBuffer(&crc, &weaponselect, sizeof(weaponselect));
        CRC32::ProcessBuffer(&crc, &weaponsubtype, sizeof(weaponsubtype));
        CRC32::ProcessBuffer(&crc, &random_seed, sizeof(random_seed));
        CRC32::ProcessBuffer(&crc, &mousedx, sizeof(mousedx));
        CRC32::ProcessBuffer(&crc, &mousedy, sizeof(mousedy));

		CRC32::Final(&crc);
		return crc;
	}

	int     command_number;     // 0x04 For matching server and client commands for debugging
	int     tick_count;         // 0x08 the tick the client created this command
	Vector  viewangles;         // 0x0C Player instantaneous view angles.
	Vector  aimdirection;       // 0x18
	float   forwardmove;        // 0x24
	float   sidemove;           // 0x28
	float   upmove;             // 0x2C
	int     buttons;            // 0x30 Attack button states
	char    impulse;            // 0x34
	int     weaponselect;       // 0x38 Current weapon id
	int     weaponsubtype;      // 0x3C
	int     random_seed;        // 0x40 For shared random functions
	short   mousedx;            // 0x44 mouse accum in x from create move
	short   mousedy;            // 0x46 mouse accum in y from create move
	bool    hasbeenpredicted;   // 0x48 Client only, tracks whether we've predicted this command at least once
    Vector  headangles;		// 0x4C
    Vector  headoffset;		// 0x58
};

class CVerifiedUserCmd
{
public:
    CUserCmd  m_cmd;
    CRC32_t   m_crc;
};

class CInput
{
public:
    std::byte			pad0[0xC];				// 0x00
    bool				bTrackIRAvailable;		// 0x0C
    bool				bMouseInitialized;		// 0x0D
    bool				bMouseActive;			// 0x0E
    std::byte			pad1[0x9A];				// 0x0F
    bool				bCameraInThirdPerson;	// 0xA9
    std::byte			pad2[0x2];				// 0xAA
    Vector				vecCameraOffset;		// 0xAC
    std::byte			pad3[0x38];				// 0xB8
    CUserCmd*			pCommands;				// 0xF0
    CVerifiedUserCmd*	pVerifiedCommands;		// 0xF4

    [[nodiscard]] CUserCmd* GetUserCmd(const int sequence) const
    {
        return &pCommands[sequence % 150];
    }

    [[nodiscard]] CVerifiedUserCmd* GetVerifiedCmd(const int sequence) const
    {
        return &pVerifiedCommands[sequence % 150];
    }
};

extern CInput* g_pInput;

enum
{
    MAX_JOYSTICKS = MAX_SPLITSCREEN_CLIENTS,
    MOUSE_BUTTON_COUNT = 5,
};

enum JoystickAxis_t
{
    JOY_AXIS_X = 0,
    JOY_AXIS_Y,
    JOY_AXIS_Z,
    JOY_AXIS_R,
    JOY_AXIS_U,
    JOY_AXIS_V,
    MAX_JOYSTICK_AXES,
};

enum
{
    JOYSTICK_MAX_BUTTON_COUNT = 32,
    JOYSTICK_POV_BUTTON_COUNT = 4,
    JOYSTICK_AXIS_BUTTON_COUNT = MAX_JOYSTICK_AXES * 2,
};

enum ButtonCode_t : int
{
    BUTTON_CODE_INVALID = -1,
    BUTTON_CODE_NONE = 0,

    KEY_FIRST = 0,

    KEY_NONE = KEY_FIRST,
    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
    KEY_PAD_0,
    KEY_PAD_1,
    KEY_PAD_2,
    KEY_PAD_3,
    KEY_PAD_4,
    KEY_PAD_5,
    KEY_PAD_6,
    KEY_PAD_7,
    KEY_PAD_8,
    KEY_PAD_9,
    KEY_PAD_DIVIDE,
    KEY_PAD_MULTIPLY,
    KEY_PAD_MINUS,
    KEY_PAD_PLUS,
    KEY_PAD_ENTER,
    KEY_PAD_DECIMAL,
    KEY_LBRACKET,
    KEY_RBRACKET,
    KEY_SEMICOLON,
    KEY_APOSTROPHE,
    KEY_BACKQUOTE,
    KEY_COMMA,
    KEY_PERIOD,
    KEY_SLASH,
    KEY_BACKSLASH,
    KEY_MINUS,
    KEY_EQUAL,
    KEY_ENTER,
    KEY_SPACE,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_CAPSLOCK,
    KEY_NUMLOCK,
    KEY_ESCAPE,
    KEY_SCROLLLOCK,
    KEY_INSERT,
    KEY_DELETE,
    KEY_HOME,
    KEY_END,
    KEY_PAGEUP,
    KEY_PAGEDOWN,
    KEY_BREAK,
    KEY_LSHIFT,
    KEY_RSHIFT,
    KEY_LALT,
    KEY_RALT,
    KEY_LCONTROL,
    KEY_RCONTROL,
    KEY_LWIN,
    KEY_RWIN,
    KEY_APP,
    KEY_UP,
    KEY_LEFT,
    KEY_DOWN,
    KEY_RIGHT,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,
    KEY_CAPSLOCKTOGGLE,
    KEY_NUMLOCKTOGGLE,
    KEY_SCROLLLOCKTOGGLE,

    KEY_LAST = KEY_SCROLLLOCKTOGGLE,
    KEY_COUNT = KEY_LAST - KEY_FIRST + 1,

    // Mouse
    MOUSE_FIRST = KEY_LAST + 1,

    MOUSE_LEFT = MOUSE_FIRST,
    MOUSE_RIGHT,
    MOUSE_MIDDLE,
    MOUSE_4,
    MOUSE_5,
    MOUSE_WHEEL_UP,		// A fake button which is 'pressed' and 'released' when the wheel is moved up
    MOUSE_WHEEL_DOWN,	// A fake button which is 'pressed' and 'released' when the wheel is moved down

    MOUSE_LAST = MOUSE_WHEEL_DOWN,
    MOUSE_COUNT = MOUSE_LAST - MOUSE_FIRST + 1,

    // Joystick
    JOYSTICK_FIRST = MOUSE_LAST + 1,

    JOYSTICK_FIRST_BUTTON = JOYSTICK_FIRST,
    JOYSTICK_LAST_BUTTON = JOYSTICK_BUTTON_INTERNAL(MAX_JOYSTICKS - 1, JOYSTICK_MAX_BUTTON_COUNT - 1),
    JOYSTICK_FIRST_POV_BUTTON,
    JOYSTICK_LAST_POV_BUTTON = JOYSTICK_POV_BUTTON_INTERNAL(MAX_JOYSTICKS - 1, JOYSTICK_POV_BUTTON_COUNT - 1),
    JOYSTICK_FIRST_AXIS_BUTTON,
    JOYSTICK_LAST_AXIS_BUTTON = JOYSTICK_AXIS_BUTTON_INTERNAL(MAX_JOYSTICKS - 1, JOYSTICK_AXIS_BUTTON_COUNT - 1),

    JOYSTICK_LAST = JOYSTICK_LAST_AXIS_BUTTON,

    BUTTON_CODE_LAST,
    BUTTON_CODE_COUNT = BUTTON_CODE_LAST - KEY_FIRST + 1,

    // Helpers for XBox 360
    KEY_XBUTTON_UP = JOYSTICK_FIRST_POV_BUTTON,	// POV buttons
    KEY_XBUTTON_RIGHT,
    KEY_XBUTTON_DOWN,
    KEY_XBUTTON_LEFT,

    KEY_XBUTTON_A = JOYSTICK_FIRST_BUTTON,		// Buttons
    KEY_XBUTTON_B,
    KEY_XBUTTON_X,
    KEY_XBUTTON_Y,
    KEY_XBUTTON_LEFT_SHOULDER,
    KEY_XBUTTON_RIGHT_SHOULDER,
    KEY_XBUTTON_BACK,
    KEY_XBUTTON_START,
    KEY_XBUTTON_STICK1,
    KEY_XBUTTON_STICK2,
    KEY_XBUTTON_INACTIVE_START,

    KEY_XSTICK1_RIGHT = JOYSTICK_FIRST_AXIS_BUTTON,	// XAXIS POSITIVE
    KEY_XSTICK1_LEFT,							// XAXIS NEGATIVE
    KEY_XSTICK1_DOWN,							// YAXIS POSITIVE
    KEY_XSTICK1_UP,								// YAXIS NEGATIVE
    KEY_XBUTTON_LTRIGGER,						// ZAXIS POSITIVE
    KEY_XBUTTON_RTRIGGER,						// ZAXIS NEGATIVE
    KEY_XSTICK2_RIGHT,							// UAXIS POSITIVE
    KEY_XSTICK2_LEFT,							// UAXIS NEGATIVE
    KEY_XSTICK2_DOWN,							// VAXIS POSITIVE
    KEY_XSTICK2_UP,								// VAXIS NEGATIVE
};

enum MouseCodeState_t
{
    BUTTON_RELEASED = 0,
    BUTTON_PRESSED,
    BUTTON_DOUBLECLICKED,
};
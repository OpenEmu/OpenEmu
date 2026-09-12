"""SDL2 game-controller input, independent of the desktop's window system."""
import ctypes as C
import ctypes.util


class Gamepad:
    def __init__(self):
        self.sdl = None
        self.pad = None
        try:
            self.sdl = C.CDLL(ctypes.util.find_library("SDL2-2.0") or "libSDL2-2.0.so.0")
            specs = {
                "SDL_InitSubSystem": (C.c_int, [C.c_uint]),
                "SDL_QuitSubSystem": (None, [C.c_uint]),
                "SDL_NumJoysticks": (C.c_int, []),
                "SDL_IsGameController": (C.c_int, [C.c_int]),
                "SDL_GameControllerOpen": (C.c_void_p, [C.c_int]),
                "SDL_GameControllerClose": (None, [C.c_void_p]),
                "SDL_GameControllerGetAttached": (C.c_int, [C.c_void_p]),
                "SDL_GameControllerUpdate": (None, []),
                "SDL_GameControllerGetButton": (C.c_ubyte, [C.c_void_p, C.c_int]),
                "SDL_GameControllerGetAxis": (C.c_int16, [C.c_void_p, C.c_int]),
            }
            for name, (ret, args) in specs.items():
                f = getattr(self.sdl, name)
                f.restype, f.argtypes = ret, args
            if self.sdl.SDL_InitSubSystem(0x2000) != 0:
                self.sdl = None
        except (OSError, AttributeError):
            self.sdl = None

    def poll(self):
        if not self.sdl:
            return 0
        s = self.sdl
        s.SDL_GameControllerUpdate()
        if self.pad and not s.SDL_GameControllerGetAttached(self.pad):
            s.SDL_GameControllerClose(self.pad)
            self.pad = None
        if not self.pad:
            for i in range(s.SDL_NumJoysticks()):
                if s.SDL_IsGameController(i):
                    self.pad = s.SDL_GameControllerOpen(i)
                    if self.pad:
                        break
        if not self.pad:
            return 0
        # Nintendo layout: east=A, south=B; d-pad, back, start and shoulders.
        mapping = {1: 1, 0: 2, 4: 4, 6: 8, 14: 16, 13: 32, 11: 64, 12: 128,
                   10: 256, 9: 512, 3: 1024, 2: 2048}
        keys = sum(bit for button, bit in mapping.items() if s.SDL_GameControllerGetButton(self.pad, button))
        x, y = (s.SDL_GameControllerGetAxis(self.pad, axis) for axis in (0, 1))
        if x > 16000: keys |= 16
        if x < -16000: keys |= 32
        if y < -16000: keys |= 64
        if y > 16000: keys |= 128
        return keys

    def close(self):
        if self.sdl:
            if self.pad:
                self.sdl.SDL_GameControllerClose(self.pad)
                self.pad = None
            self.sdl.SDL_QuitSubSystem(0x2000)
            self.sdl = None

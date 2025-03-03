---@diagnostic disable: missing-return
---@class Bits
Bits = {}

---Binary left shift
---@param a integer
---@param n integer
---@return integer
function Bits.lshift(a, n) end

---Binary right shift
---@param a integer
---@param n integer
---@return integer
function Bits.rshift(a, n) end

---Binary AND operation
---@param a integer
---@param b integer
---@return integer
function Bits.band(a, b) end

---Binary OR operation
---@param a integer
---@param b integer
---@return integer
function Bits.bor(a, b) end

---@class System
System = {}

---Returns the system unix time
---@return number
function System.GetTime() end

---@class Async
Async = {}
Async.scripts = {}

---Registers a function to be called internally every tick
---@param func function
function Async.create(func) end

---Yeilds an async coroutine until specified seconds has passed
---@param seconds? number
function Async.wait(seconds) end

---Executes all registered coroutines. Shouldn't be called manually
function Async.tick() end

---@class Gamepad
Gamepad = {}

---@enum KeyCode
Gamepad.KeyCode = {
    A = 1,
    B = 1 << 1,
    SELECT = 1 << 2,
    START = 1 << 3,
    DPADRIGHT = 1 << 4,
    DPADLEFT = 1 << 5,
    DPADUP = 1 << 6,
    DPADDOWN = 1 << 7,
    R = 1 << 8,
    L = 1 << 9,
    X = 1 << 10,
    Y = 1 << 11,
    ZL = 1 << 14,
    ZR = 1 << 15,
    TOUCHPAD = 1 << 20,
    CSTICKRIGHT = 1 << 24,
    CSTICKLEFT = 1 << 25,
    CSTICKUP = 1 << 26,
    CSTICKDOWN = 1 << 27,
    CPADRIGHT = 1 << 28,
    CPADLEFT = 1 << 29,
    CPADUP = 1 << 30,
    CPADDOWN = 1 << 31,
    UP = 1 << 6 | 1 << 30,
    DOWN = 1 << 7 | 1 << 31,
    LEFT = 1 << 5 | 1 << 29,
    RIGHT = 1 << 4 | 1 << 28,
    CPAD = 1 << 29 | 1 << 28 | 1 << 30 | 1 << 31,
    CSTICK = 1 << 25 | 1 << 24 | 1 << 26 | 1 << 27
}

---Returns whether the key is down or not
---@param keycode KeyCode
---@return boolean
function Gamepad.IsDown(keycode) end

---Returns whether the key was just pressed
---@param keycode KeyCode
---@return boolean
function Gamepad.IsPressed(keycode) end

---Does a virtual button press
---@param keycode KeyCode
function Gamepad.PressButton(keycode) end

Gamepad.KeyPressed = {}
Gamepad.KeyPressed.listeners = {}

---Connects a function to trigger every time is detected a key pressed
---@param func function
function Gamepad.KeyPressed:Connect(func) end

---Triggers the KeyPressed event. Shouldn't be called manually
function Gamepad.KeyPressed:Trigger() end

---@class Debug
Debug = {}

---Prints a message on screen
---@param msg string
function Debug.Message(msg) end

---@class Player
Player = {}
Player.SwimSpeed = {}

---Returns the current swimming speed of the player
function Player.SwimSpeed.get() end

---Sets the swimming speed for the player
---@param vel number
function Player.SwimSpeed.set(vel) end

---@class World
World = {}

---Sets the height at which clouds will be rendered
---@param height number
function World.SetCloudsHeight(height) end
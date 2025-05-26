---@diagnostic disable: missing-return, duplicate-set-field

Async = {}

---Adds the function to the queue that will run apart from the game until the functions ends
---@param func function
function Async.create(func) end

---Yeilds the current task until time has passed. Always returns true
---@param seconds number?
---@return boolean
function Async.wait(seconds) end

Bits = {}

---@param a integer
---@param b integer
---@return integer
function Bits.lshift(a, b) end

---@param a integer
---@param b integer
---@return integer
function Bits.rshift(a, b) end

---@param a integer
---@param b integer
---@return integer
function Bits.band(a, b) end

---@param a integer
---@param b integer
---@return integer
function Bits.bor(a, b) end

---@param a integer
---@param b integer
---@return integer
function Bits.bxor(a, b) end

---@param a integer
---@return integer
function Bits.bnot(a) end

Game = {}

Game.Debug = {}

---Displays a notification on screen
---@param msg string
function Game.Debug.message(msg) end

---Appends the message to log file. Optionally shows the message on screen
---@param msg string
---@param showOnScreen boolean
function Game.Debug.log(msg, showOnScreen) end

---Appends the error message to log file and shows it on screen
---@param msg string
function Game.Debug.logerror(msg) end

---Show error on screen
---@param msg string
function Game.Debug.error(msg) end

Game.Event = {}

---@class EventClass
local EventClass = {}

---Adds a function to call when this events fires
---@param func function
function EventClass:Connect(func) end

---Fire this event
function EventClass:Trigger() end

---@class BaseEvent: EventClass
Game.Event.BaseEvent = {}

---@class OnNewFrame: EventClass
Game.Event.OnNewFrame = {}

---@class OnKeyPressed: EventClass
Game.Event.OnKeyPressed = {}

---@class OnKeyDown: EventClass
Game.Event.OnKeyDown = {}

---@class OnKeyReleased: EventClass
Game.Event.OnKeyReleased = {}

Game.Gamepad = {}

---@class KeyCode: integer

---Returns true if the key is pressed (only counts for the first press, for button held use isDown)
---@param keycode KeyCode
---@return boolean
function Game.Gamepad.isPressed(keycode) end

---Returns true if the key is down
---@param keycode KeyCode
---@return boolean
function Game.Gamepad.isDown(keycode) end

---Returns true if the key just got released
---@param keycode KeyCode
---@return boolean
function Game.Gamepad.isReleased(keycode) end

---Performs a virtual button press
---@param keycode KeyCode
function Game.Gamepad.pressButton(keycode) end

---Returns touch x and y position
---@return number
---@return number
function Game.Gamepad.getTouch() end

Game.Gamepad.KeyCodes = {}

Game.Gamepad.KeyCodes.A = 1

Game.Gamepad.KeyCodes.B = 2

Game.Gamepad.KeyCodes.SELECT = 4

Game.Gamepad.KeyCodes.START = 8

Game.Gamepad.KeyCodes.DPADRIGHT = 16

Game.Gamepad.KeyCodes.DPADLEFT = 32

Game.Gamepad.KeyCodes.DPADUP = 64

Game.Gamepad.KeyCodes.DPADDOWN = 128

Game.Gamepad.KeyCodes.R = 256

Game.Gamepad.KeyCodes.L = 512

Game.Gamepad.KeyCodes.X = 1024

Game.Gamepad.KeyCodes.Y = 2048

Game.Gamepad.KeyCodes.ZL = 16384

Game.Gamepad.KeyCodes.ZR = 32768

Game.Gamepad.KeyCodes.TOUCHPAD = 1048576

Game.Gamepad.KeyCodes.CSTICKRIGHT = 16777216

Game.Gamepad.KeyCodes.CSTICKLEFT = 33554432

Game.Gamepad.KeyCodes.CSTICKUP = 67108864

Game.Gamepad.KeyCodes.CSTICKDOWN = 134217728

Game.Gamepad.KeyCodes.CPADRIGHT = 268435456

Game.Gamepad.KeyCodes.CPADLEFT = 536870912

Game.Gamepad.KeyCodes.CPADUP = 1073741824

Game.Gamepad.KeyCodes.CPADDOWN = 2147483648

Game.Gamepad.KeyCodes.UP = 1073741888

Game.Gamepad.KeyCodes.DOWN = 2147483776

Game.Gamepad.KeyCodes.LEFT = 536870944

Game.Gamepad.KeyCodes.RIGHT = 268435472

Game.Gamepad.KeyCodes.CPAD = 2952790016

Game.Gamepad.KeyCodes.CSTICK = 184549376

Game.Graphics = {}

---Draws a rect on screen
---@param x integer
---@param y integer
---@param width integer
---@param height integer
---@param color integer
function Game.Graphics.drawRect(x, y, width, height, color) end

---Draws a solid rect on screen
---@param x integer
---@param y integer
---@param width integer
---@param height integer
---@param color integer
function Game.Graphics.drawRectFill(x, y, width, height, color) end

Game.LocalPlayer = {}

Game.LocalPlayer.Position = {}

---Gets local player position
---@return number
---@return number
---@return number
function Game.LocalPlayer.Position.get() end

---Sets player position
---@param x number
---@param y number
---@param z number
function Game.LocalPlayer.Position.set(x, y, z) end

Game.LocalPlayer.OnGround = false

Game.LocalPlayer.Sneaking = false

Game.LocalPlayer.Jumping = false

Game.LocalPlayer.Sprinting = false

Game.LocalPlayer.Flying = false

Game.LocalPlayer.UnderWater = false

Game.LocalPlayer.TouchingWall = false

Game.LocalPlayer.Invincible = false

Game.LocalPlayer.CanFly = false

Game.LocalPlayer.CanConsumeItems = false

Game.LocalPlayer.BaseMoveSpeed = 0.0

Game.LocalPlayer.MoveSpeed = 0.0

Game.LocalPlayer.SwimSpeed = 0.02

Game.LocalPlayer.FlySpeed = 0.0

Game.LocalPlayer.CurrentHP = 0.0

Game.LocalPlayer.MaxHP = 0.0

Game.LocalPlayer.CurrentHunger = 0.0

Game.LocalPlayer.MaxHunger = 0.0

Game.LocalPlayer.CurrentLevel = 0.0

Game.LocalPlayer.LevelProgress = 0.0

Game.LocalPlayer.Gamemode = 0

Game.LocalPlayer.ReachDistance = 0.0

Game.LocalPlayer.SprintDelay = 0.0

Game.LocalPlayer.Camera = {}

Game.LocalPlayer.Camera.FOV = 0.0

Game.LocalPlayer.Camera.Yaw = 0.0

Game.LocalPlayer.Camera.Pitch = 0.0

Game.LocalPlayer.Inventory = {}

Game.LocalPlayer.Inventory.Slots = {}

---@class InventorySlot
local InventorySlot = {}

InventorySlot.Slot = 0

InventorySlot.ItemID = 0

InventorySlot.ItemCount = 0

InventorySlot.ItemData = 0

InventorySlot.ItemName = ""

System = {}

---Returns UNIX time
---@return number
function System.getTime() end

---Pauses the process
function System.pause() end

---Resume the process
function System.resume() end

Game.World = {}

Game.World.CloudsHeight = 0.0

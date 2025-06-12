---@diagnostic disable: missing-return, duplicate-set-field

Async = {}

---Adds the function to the queue that will run apart from the game until the functions ends
---@param func function
function Async.create(func) end

---Yeilds the current task until time has passed. Always returns true
---@param seconds number?
---@return boolean
function Async.wait(seconds) end

Game = {}

Core = {}

string = {}

---Returns a table with a string splitted
---@param s string
---@param separator string
---@return table
function string.split(s, separator) end

Core.Debug = {}

---Displays a notification on screen
---@param msg string
function Core.Debug.message(msg) end

---Appends the message to log file. Optionally shows the message on screen
---@param msg string
---@param showOnScreen boolean
function Core.Debug.log(msg, showOnScreen) end

---Appends the error message to log file and shows it on screen
---@param msg string
function Core.Debug.logerror(msg) end

---Show error on screen
---@param msg string
function Core.Debug.error(msg) end

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

---@class OnKeyPressed: EventClass
Game.Event.OnKeyPressed = {}

---@class OnKeyDown: EventClass
Game.Event.OnKeyDown = {}

---@class OnKeyReleased: EventClass
Game.Event.OnKeyReleased = {}

---@class OnPlayerJoinWorld: EventClass
Game.Event.OnPlayerJoinWorld = {}

---@class OnPlayerLeaveWorld: EventClass
Game.Event.OnPlayerLeaveWorld = {}

Core.Graphics = {}

---Stops the game and allows to draw on screen. Until Core.Graphics.close is called the function will be executed every frame
---Other events and async tasks will continue running
---@param func function
function Core.Graphics.open(func) end

---Resumes the game, the callback function will no longer be called and draw functions will not work
function Core.Graphics.close() end

---Returns if Graphics are open
function Core.Graphics.isOpen() end

---Draws a rect on screen. Only can be used when Core.Graphics.open was called
---@param x integer
---@param y integer
---@param width integer
---@param height integer
---@param color integer
function Core.Graphics.drawRect(x, y, width, height, color) end

---Draws a solid rect on screen. Only can be used when Core.Graphics.open was called
---@param x integer
---@param y integer
---@param width integer
---@param height integer
---@param color integer
function Core.Graphics.drawRectFill(x, y, width, height, color) end

---Draws a text on screen. Only can be used when Core.Graphics.open was called
---@param text string
---@param x integer
---@param y integer
---@param color integer
function Core.Graphics.drawText(text, x, y, color) end

---Returns a color with the r, g, b values
---@param r integer
---@param g integer
---@param b integer
---@return integer
function Core.Graphics.colorRGB(r, g, b) end

---Returns a color with the r, g, b, a values
---@param r integer
---@param g integer
---@param b integer
---@param a integer
---@return integer
function Core.Graphics.colorRGBA(r, g, b, a) end

Core.Keyboard = {}

---Opens the keyboard and returns the user input as string
---@param message string?
---@return string?
function Core.Keyboard.getString(message) end

---Opens the keyboard and returns the user input as number
---@param message string?
---@return number?
function Core.Keyboard.getNumber(message) end

---Opens the keyboard and returns the user input as unsigned integer
---@param message string?
---@return integer?
function Core.Keyboard.getInteger(message) end

---Opens the keyboard and returns the user input as hexadecimal
---@param message string?
---@return integer?
function Core.Keyboard.getHex(message) end

Core.System = {}

---Returns UNIX time
---@return number
function Core.System.getTime() end

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

Game.Items = {}

---Find the item ID using its name
---@param name string
---@return integer?
function Game.Items.findItemIDByName(name) end

---Find the item ID using its name
---@param itemID integer
---@return string?
function Game.Items.findItemNameByID(itemID) end

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

Game.LocalPlayer.Velocity = {}

---Gets local player velocity
---@return number
---@return number
---@return number
function Game.LocalPlayer.Velocity.get() end

---Sets player velocity
---@param x number
---@param y number
---@param z number
function Game.LocalPlayer.Velocity.set(x, y, z) end

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

Game.LocalPlayer.Inventory.ArmorSlots = {}

---@class InventorySlot
local InventorySlot = {}

InventorySlot.Slot = 0

InventorySlot.ItemID = 0

InventorySlot.ItemCount = 0

InventorySlot.ItemData = 0

InventorySlot.ItemName = ""

---@class InventoryArmorSlot
local InventoryArmorSlot = {}

InventoryArmorSlot.Slot = 0

InventoryArmorSlot.ItemID = 0

InventoryArmorSlot.ItemData = 0

InventoryArmorSlot.ItemName = ""

Game.World = {}

Game.World.Loaded = false

Game.World.Raining = false

Game.World.Thunderstorm = false

Game.World.CloudsHeight = 0.0

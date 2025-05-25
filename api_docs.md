You can access local player functions using 
```
Game.LocalPlayer --- Game is a global variable
```
You can store player like this
```lua
local player = Game.LocalPlayer
```
and then you can access some player data like this
```lua
player.OnGround
```
and you can also set the value like this
```lua
player.OnGround = true
```
and the same with the next fields (some are booleans and others numbers, both you can also set their value)
```lua
player.Sneaking
player.Jumping
player.Flying
player.UnderWater
player.CanFly
player.CanConsumeItems
player.MoveSpeed
player.SwimSpeed
player.FlySpeed
player.CurrentHP
player.MaxHP
player.CurrentHunger
player.CurrentLevel = 0.0
player.Gamemode = 0
```
And some others more not listed here
By this time player position is still managed the old way, using functions so it would be like this
```lua
local x, y, z = player.Position.get()
-- or
player.Position.set(0, 0, 0)
```
You can also access camera info
```lua
player.Camera.FOV
player.Camera.Yaw
player.Camera.Pitch
```
And by last but not less important, you can also access player inventory slots by using
```lua
player.Inventory.Slots
```
So if you wanna know what item is on slot 1 you need to use
```lua
player.Inventory.Slots[1].ItemName
```
You could replace 1 by others slots, it accepts 1-36. And also you can directly access the hand/held slot (the current selected slot) using
```lua
player.Inventory.Slots["hand"].ItemName
```
You can access other item info by replacing ItemName with ItemID, ItemCount or ItemData.

There is another important module called Event that allows you to execute a function when an event is triggered, for example, you can access these events
```lua
Game.Event.OnKeyPressed
Game.Event.OnKeyDown
Game.Event.OnKeyReleased
```
You can connect functions like this
```lua
Game.Event.OnKeyPressed:Connect(function()
	--Some code
end)
```
or the other way
```lua
Game.Event.OnKeyPressed:Connect(yourFunctionName)
```
So, to allow you to detect what key was pressed, or is down/released, you need the next module called Gamepad. So for the past example to execute code when button A is pressed you do this
```lua
Game.Event.OnKeyPressed:Connect(function()
	if (Game.Gamepad.isPressed(Game.Gamepad.KeyCodes.A)) then
		--Some code
	end
end)
```
You can also simplify it a bit by setting local variable Gamepad like this
```lua
local Gamepad = Game.Gamepad
Game.Event.OnKeyPressed:Connect(function()
	if (Gamepad.isPressed(Gamepad.KeyCodes.A)) then
		--Some code
	end
end)
```
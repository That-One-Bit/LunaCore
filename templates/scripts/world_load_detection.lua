local World = Game.World
local Debug = Game.Debug
local lastWorldLoaded = World.Loaded

Async.create(function ()
    while Async.wait() do
        if lastWorldLoaded ~= World.Loaded then
            lastWorldLoaded = World.Loaded
            if lastWorldLoaded == true then
                Debug.message("Player joined a world")
            else
                Debug.message("Player leaved a world")
            end
        end
    end
end)
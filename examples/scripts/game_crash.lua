local tbl = {}

Async.create(function ()
    while Async.wait() do
        local tbl2 = {}
        for i = 1, 100, 1 do
            table.insert(tbl2, i)
        end
        table.insert(tbl, tbl2)
    end
end)
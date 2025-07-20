Core.Debug.log("Init testMod", false)

local testModReg = CoreAPI.Items.newItemRegistry("testMod")

testModReg:registerItem("copper_ingot", {
    texture = "items/copper_ingot.3dst",
    group = CoreAPI.ItemGroups.newItemGroupIdentifier(CoreAPI.ItemGroups.FOOD_MINERALS, CoreAPI.ItemGroups.beforeItem("iron_ingot")),
    locales = {
        en_US = "Copper ingot",
        es_MX = "Lingote de cobre"
    }
})

testModReg:registerItems()
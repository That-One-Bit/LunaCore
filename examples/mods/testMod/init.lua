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
testModReg:registerItem("amethyst_shard", {
    texture = "items/amethyst_shard.3dst",
    group = CoreAPI.ItemGroups.newItemGroupIdentifier(CoreAPI.ItemGroups.FOOD_MINERALS, CoreAPI.ItemGroups.beforeItem("string")),
    locales = {
        en_US = "Amethyst shard",
        es_MX = "Fragmento de amatista",
        en_UK = "Amethyst shard"
    }
})
testModReg:registerItems()

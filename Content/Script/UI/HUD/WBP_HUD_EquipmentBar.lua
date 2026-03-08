--
-- DESCRIPTION
--
-- @COMPANY **
-- @AUTHOR **
-- @DATE ${date} ${ZYF_time}
--

---@type WBP_HUD_EquipmentBar_C
local M = UnLua.Class()

--function M:Initialize(Initializer)
--end

--function M:PreConstruct(IsDesignTime)
--end

function M:Construct()
    print("ZYF_WBP_HUD_EquipmentBar_Construct")
    self:Test_EquipmentBar()
end

function M:Test_EquipmentBar()
    -- 单元测试代码
end

function M:HandleScroll(wheelDelta)
    -- 补充视听效果

    self:HandleScroll(wheelDelta);
end


--function M:Tick(MyGeometry, InDeltaTime)
--end

return M

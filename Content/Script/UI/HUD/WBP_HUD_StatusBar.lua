--
-- DESCRIPTION
--
-- @COMPANY **
-- @AUTHOR **
-- @DATE ${date} ${time}
--

---@type WBP_HUD_StatusBar_C
local M = UnLua.Class()

--function M:Initialize(Initializer)
--end

--function M:PreConstruct(IsDesignTime)
--end

-- function M:Construct()
-- end

--function M:Tick(MyGeometry, InDeltaTime)
--end

function M:AddStatus(statusInfo)
    self:AddStatus(statusInfo)
    -- 调度视听效果
end

function M:RemoveStatus(iconIndex)
    self:RemoveStatus(iconIndex)
    -- 调度视听效果
end

function M:UpdateStatus(statusInfo)
    self:UpdateStatus(statusInfo)
    -- 调度视听效果
end

return M

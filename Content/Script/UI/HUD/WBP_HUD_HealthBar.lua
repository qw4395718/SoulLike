--
-- DESCRIPTION
--
-- @COMPANY **
-- @AUTHOR **
-- @DATE ${date} ${time}
--

---@type WBP_HUD_HealthBar_C
local M = UnLua.Class()

--function M:Initialize(Initializer)
--end

--function M:PreConstruct(IsDesignTime)
--end

-- function M:Construct()
-- end

--function M:Tick(MyGeometry, InDeltaTime)
--end

-- function M:UpdateProgressBar(min,max,current)
--     self:UpdateProgressBar(min,max,current)
--     if(current ~= self.m_currentProgressBarPercent)
--     then
--         if(current >= max) then
--             --血量恢复满的视觉效果
--         else
--             if(current  <= min) then
--                 --血量清空的视觉效果
--             else
--                 if(current > self.m_currentProgressBarPercent) then
--                     -- 血量恢复的视觉效果
--                 else
--                     -- 血量损失的视觉效果
--                 end
--             end
--         end
--     end
-- end

return M

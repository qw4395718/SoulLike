---@type WBP_HUD_Dialog_C
local HUD_Dialog  = UnLua.Class()

print("[Lua] HUD_Dialog.lua 文件被加载") -- 这个在文件被 require 时就会打印

function HUD_Dialog:OnInitialized()
    -- 可以在此处进行初始化
    self:TestDiaglog()
    print("ZYF_HUD_Dialog:Initialize() End")
end

function HUD_Dialog:TestDiaglog()
    local tmpName = "原神"
    local tmpContent = "《原神》是由中国游戏公司米哈游（miHoYo）自主研发的一款开放世界动作角色扮演游戏，于2020年9月正式全球上线，支持iOS、Android、PC、PlayStation等多平台，并计划登陆Nintendo Switch。游戏以精美的画面、丰富的探索内容和深度的角色养成系统闻名，在全球范围内拥有极高的人气。一、游戏核心特色/n开放世界探索/n游戏地图庞大且多样化，包含草原、雪山、沙漠、海洋、奇幻国度等区域，鼓励玩家自由探索。/n世界中隐藏着大量谜题、宝箱、秘境和随机事件，探索奖励丰富。"

    self:SetDialogText(tmpName, tmpContent)
end

return HUD_Dialog
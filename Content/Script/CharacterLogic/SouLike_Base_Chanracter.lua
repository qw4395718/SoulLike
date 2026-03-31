--
-- DESCRIPTION
--
-- @COMPANY **
-- @AUTHOR **
-- @DATE ${date} ${time}
--

---@type SoulLike_Base_CharacterBP_C
local M = UnLua.Class()

-- function M:Initialize(Initializer)
-- end

-- function M:UserConstructionScript()
-- end

-- function M:ReceiveBeginPlay()
-- end

-- function M:ReceiveEndPlay()
-- end

-- function M:ReceiveTick(DeltaSeconds)
-- end

-- function M:ReceiveAnyDamage(Damage, DamageType, InstigatedBy, DamageCauser)
-- end

-- function M:ReceiveActorBeginOverlap(OtherActor)
-- end

-- function M:ReceiveActorEndOverlap(OtherActor)
-- end

function M:ReceiveBeginPlay()
    -- 获取技能管理器
    self.Overridden.ReceiveBeginPlay(self)
    print("ZYF_SoulLike_Base_CharacterBP_C_Unlua_BeginPlay 1")
    local AbilityMgr = require("GAS.AbilityManager")
    
    -- 给予火球术技能
    local FireballHandle = AbilityMgr:GrantAbility(
        self,
        "/Game/SoulLikeDemo/GAS/GA/GA_Fireball.GA_Fireball_C",
        0,
        "GAS.GA.Fireball"
    )
    
    -- 保存技能句柄
    self.FireballHandle = FireballHandle
    print("ZYF_SoulLike_Base_CharacterBP_C_Unlua_BeginPlay")
end

-- 输入回调：按下技能键
function M:OnFireballPressed()
    local AbilityMgr = require("GAS.AbilityManager")
    AbilityMgr:ActivateAbility(self, self.FireballHandle)
end

return M

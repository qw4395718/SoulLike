-- UE4.26技能管理器
local AbilityManager = UnLua.Class()

function AbilityManager:Initialize()
    self.Abilities = {}
    self.ActiveAbilities = {}
end

-- 给予角色技能
function AbilityManager:GrantAbility(Character, AbilityClassPath, AbilityLevel, LuaFilePath)
    print("ZYF_GrantAbility_0")
    if not Character or not Character.AbilitySystemComp then
        return nil
    end
    print("ZYF_GrantAbility_1")

    local ASC = Character.AbilitySystemComp
    local AbilityClass = UE.UClass.Load(AbilityClassPath)
    
    if not AbilityClass then
        print("Failed to load ability class: " .. AbilityClassPath)
        return nil
    end
    
    print("ZYF_GrantAbility_2")

    local Handle = ASC:GiveAbilityForBP(
            AbilityClass,  -- 直接传Class
            AbilityLevel or 1,
            0,  -- InputID
            Character
        )
    
    print("ZYF_GrantAbility_2_1" .. Handle.Handle)
    --self:ActivateAbility(Character, Handle)
    return Handle
end

-- 激活技能
function AbilityManager:ActivateAbility(Character, Handle)
    if not Character or not Character.AbilitySystemComp then
        return false
    end
    
    local ASC = Character.AbilitySystemComp
    return ASC:TryActivateAbilityByHandle(Handle)
end

-- 移除技能
function AbilityManager:RemoveAbility(Character, Handle)
    if not Character or not Character.AbilitySystemComp then
        return
    end
    
    local ASC = Character.AbilitySystemComp
    ASC:ClearAbilityByHandle(Handle)
end

return AbilityManager
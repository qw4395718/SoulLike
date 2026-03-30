-- UE4.26技能管理器
local AbilityManager = UnLua.Class()

function AbilityManager:Initialize()
    self.Abilities = {}
    self.ActiveAbilities = {}
end

-- 给予角色技能
function AbilityManager:GrantAbility(Character, AbilityClassPath, AbilityLevel)
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
    -- UE4.26的GiveAbility方式
    -- local Spec = UE.FGameplayAbilitySpec()
    -- Spec.Ability = AbilityClass
    -- Spec.Level = AbilityLevel
    -- Spec.InputID = 0
    -- Spec.SourceObject = Character
    
    -- local Handle = ASC:GiveAbility(Spec)
    local Handle = ASC:GiveAbilityForBP(
            AbilityClass,  -- 直接传Class
            AbilityLevel or 1,
            0,  -- InputID
            Character
        )
    
    print("ZYF_GrantAbility_2_1" .. Handle.Handle)
    -- 绑定Lua逻辑
    if Handle.Handle ~= 0 then
        -- 获取技能实例（UE4.26方式）
        local AbilityInstances = ASC:GetActivatableAbilitiesForBP()
        print("ZYF_GrantAbility_3")
        for i = 1, AbilityInstances:Length() do
            local AbilityInstance = AbilityInstances:Get(i)
            print("ZYF_GrantAbility_4")
            --if AbilityInstance and AbilityInstance.Ability and AbilityInstance.Ability == AbilityClass then
            if AbilityInstance and AbilityInstance.Ability then
                -- 加载对应的Lua文件
                print("ZYF_GrantAbility_5")
                if AbilityInstance.LuaFilePath and AbilityInstance.LuaFilePath ~= "" then
                    local AbilityLogic = require(AbilityInstance.LuaFilePath)
                    print("ZYF_GrantAbility_6")
                    if AbilityLogic then
                        -- 绑定委托
                        print("ZYF_GrantAbility_7")
                        AbilityInstance.OnAbilityActivated:Add(
                            function(Ability, Handle, ActorInfo)
                                if AbilityLogic.OnAbilityActivated then
                                    AbilityLogic:OnAbilityActivated(Ability, Handle, ActorInfo)
                                end
                            end
                        )
                    end
                end
                break
            end
        end
    end
    
    return Handle
end

-- 激活技能
function AbilityManager:ActivateAbility(Character, Handle)
    if not Character or not Character.AbilitySystemComp then
        return false
    end
    
    local ASC = Character.AbilitySystemComp
    return ASC:TryActivateAbility(Handle)
end

-- 移除技能
function AbilityManager:RemoveAbility(Character, Handle)
    if not Character or not Character.AbilitySystemComp then
        return
    end
    
    local ASC = Character.AbilitySystemComp
    ASC:ClearAbility(Handle)
end

return AbilityManager
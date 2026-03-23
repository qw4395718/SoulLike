-- UE4.26技能管理器
local AbilityManager = UnLua.Class()

function AbilityManager:Initialize()
    self.Abilities = {}
    self.ActiveAbilities = {}
end

-- 给予角色技能
function AbilityManager:GrantAbility(Character, AbilityClassPath, AbilityLevel)
    if not Character or not Character.AbilitySystemComponent then
        return nil
    end
    
    local ASC = Character.AbilitySystemComponent
    local AbilityClass = UE.UClass.Load(AbilityClassPath)
    
    if not AbilityClass then
        print("Failed to load ability class: " .. AbilityClassPath)
        return nil
    end
    
    -- UE4.26的GiveAbility方式
    local Spec = UE.FGameplayAbilitySpec()
    Spec.Ability = AbilityClass
    Spec.Level = AbilityLevel
    Spec.InputID = 0
    
    local Handle = ASC:GiveAbility(Spec)
    
    -- 绑定Lua逻辑
    if Handle.IsValid then
        -- 获取技能实例（UE4.26方式）
        local AbilityInstances = ASC:GetActivatableAbilities()
        for i = 1, AbilityInstances:Length() do
            local AbilityInstance = AbilityInstances:Get(i)
            if AbilityInstance and AbilityInstance.GetClass and AbilityInstance:GetClass() == AbilityClass then
                -- 加载对应的Lua文件
                if AbilityInstance.LuaFilePath and AbilityInstance.LuaFilePath ~= "" then
                    local AbilityLogic = require(AbilityInstance.LuaFilePath)
                    if AbilityLogic then
                        -- 绑定委托
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
    if not Character or not Character.AbilitySystemComponent then
        return false
    end
    
    local ASC = Character.AbilitySystemComponent
    return ASC:TryActivateAbility(Handle)
end

-- 移除技能
function AbilityManager:RemoveAbility(Character, Handle)
    if not Character or not Character.AbilitySystemComponent then
        return
    end
    
    local ASC = Character.AbilitySystemComponent
    ASC:ClearAbility(Handle)
end

return AbilityManager
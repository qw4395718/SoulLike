-- UE4.26版本火球术实现
local Fireball = UnLua.Class()

-- 技能配置
Fireball.Config = {
    Damage = 100,
    ManaCost = 30,
    Cooldown = 3.0,
    CastRange = 1000,
    ProjectileSpeed = 2000,
    FireEffectPath = "/Game/Effects/Fireball/Fireball_Effect.Fireball_Effect",
    HitEffectPath = "/Game/Effects/Fireball/Explosion_Effect.Explosion_Effect",
    MontagePath = "/Game/Animations/Magic_Cast_Montage.Magic_Cast_Montage",
    SoundPath = "/Game/Sounds/Fireball_Cast.Fireball_Cast"
}

-- 技能激活时的回调
function Fireball:OnAbilityActivatedForLua(Handle, ActorInfo,ActivationInfo)
    print("Fireball ability activated in UE4.26!")
    
    -- -- 获取施法者
    -- local Caster = ActorInfo:GetAvatarActor()
    -- if not Caster then
    --     self:EndAbility(Handle, ActorInfo,ActivationInfo,true,true)
    --     return
    -- end
    
    -- -- 检查并消耗魔法值
    -- if not Ability:ConsumeManaForAbility() then
    --     print("Not enough mana!")
    --     self:EndAbility(Handle, ActorInfo,ActivationInfo,true,true)
    --     return
    -- end
    
    -- -- 播放施法动画
    -- self:PlayCastAnimation(Ability, Caster)
    
    -- -- 获取目标（简化：获取前方1000单位的目标）
    -- local Target = self:GetTargetInFront(Caster)
    
    -- -- 延迟生成火球（配合动画）
    -- self:ScheduleFireballSpawn(Ability, Caster, Target, 0.3)
end

-- 播放施法动画
function Fireball:PlayCastAnimation(Ability, Caster)
    -- 加载蒙太奇
    local Montage = UE.LoadObject(self.Config.MontagePath)
    if Montage and Ability.PlayMontageForAbility then
        Ability:PlayMontageForAbility(Montage, 1.0)
    end
    
    -- 播放施法音效
    local Sound = UE.LoadObject(self.Config.SoundPath)
    if Sound then
        UE.UGameplayStatics.PlaySoundAtLocation(
            Caster:GetWorld(), 
            Sound, 
            Caster:K2_GetActorLocation()
        )
    end
end

-- 获取前方目标
function Fireball:GetTargetInFront(Caster)
    local ForwardVector = Caster:GetActorForwardVector()
    local StartLocation = Caster:K2_GetActorLocation()
    local EndLocation = StartLocation + ForwardVector * self.Config.CastRange
    
    -- 射线检测
    local HitResult = UE.FHitResult()
    local bHit = UE.UKismetSystemLibrary.LineTraceSingle(
        Caster:GetWorld(),
        StartLocation,
        EndLocation,
        UE.ETraceTypeQuery.TraceTypeQuery1,
        false,
        {Caster},
        UE.EDrawDebugTrace.None,
        HitResult,
        true
    )
    
    if bHit then
        return HitResult.Actor
    end
    return nil
end

-- 延迟生成火球
function Fireball:ScheduleFireballSpawn(Ability, Caster, Target, Delay)
    local World = Caster:GetWorld()
    if not World then return end
    
    -- UE4.26的Timer用法
    local TimerDelegate = function()
        self:SpawnFireball(Ability, Caster, Target)
    end
    
    World:GetTimerManager():SetTimer(nil, TimerDelegate, Delay, false)
end

-- 生成火球
function Fireball:SpawnFireball(Ability, Caster, Target)
    -- 加载火球蓝图类
    local FireballClass = UE.UClass.Load("/Game/Blueprints/BP_FireballProjectile.BP_FireballProjectile_C")
    if not FireballClass then
        print("Failed to load Fireball class!")
        self:EndAbility(Ability, nil, nil)
        return
    end
    
    local SpawnLocation = Caster:K2_GetActorLocation() + Caster:GetActorForwardVector() * 100
    local SpawnRotation = Caster:GetActorRotation()
    
    -- UE4.26的SpawnActor方式
    local Projectile = Caster:GetWorld():SpawnActor(
        FireballClass,
        SpawnLocation,
        SpawnRotation,
        UE.FSpawnActorParameters()
    )
    
    if Projectile then
        -- 加载伤害效果
        local EffectClass = UE.UClass.Load("/Game/Blueprints/GE_FireDamage.GE_FireDamage_C")
        
        -- 存储技能信息到Projectile（使用Lua表）
        Projectile.Damage = self.Config.Damage
        Projectile.EffectClass = EffectClass
        Projectile.SourceCaster = Caster
        
        -- 发射火球
        if Projectile.Launch then
            local Direction = (Target:K2_GetActorLocation() - SpawnLocation):GetSafeNormal()
            Projectile:Launch(Direction, self.Config.ProjectileSpeed)
        end
        
        -- 播放火球特效
        self:PlayFireballEffect(Projectile)
        
        print(string.format("Fireball spawned in UE4.26! Damage: %.0f", self.Config.Damage))
    end
    
    -- 技能结束
    self:EndAbility(Ability, nil, nil)
end

-- 播放火球特效
function Fireball:PlayFireballEffect(Projectile)
    local EffectPath = self.Config.FireEffectPath
    local ParticleComponent = Projectile:GetComponentByClass(UE.UParticleSystemComponent)
    
    if ParticleComponent then
        local Effect = UE.LoadObject(EffectPath)
        if Effect then
            ParticleComponent:SetTemplate(Effect)
            ParticleComponent:Activate()
        end
    end
end

-- 结束技能
function Fireball:EndAbility(Ability, Handle, ActorInfo)
    if Ability and Ability.EndAbility then
        Ability:EndAbility(Handle, ActorInfo, true, false)
    end
end

return Fireball
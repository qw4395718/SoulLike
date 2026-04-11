-- UE4.26版本火球术实现
local Fireball = UnLua.Class()

-- 技能配置
Fireball.Config = {
    Damage = 100,
    ManaCost = 30,
    Cooldown = 3.0,
    CastRange = 1000,
    ProjectileSpeed = 2000,
    ProjectilePath = "/Game/SoulLikeDemo/Blueprints/Actor/BP_FireballProjectile.BP_FireballProjectile_C",
    GameAbilityEffectPath = "/Game/SoulLikeDemo/GAS/GE/GE_FireDamage.GE_FireDamage",
    FireEffectPath = "/Game/Effects/Fireball/Fireball_Effect.Fireball_Effect",
    HitEffectPath = "/Game/Effects/Fireball/Explosion_Effect.Explosion_Effect",
    MontagePath = "/Game/Animations/Magic_Cast_Montage.Magic_Cast_Montage",
    SoundPath = "/Game/Sounds/Fireball_Cast.Fireball_Cast"
}

-- 技能激活时的回调
function Fireball:OnAbilityActivatedForLua(Handle, ActorInfo,ActivationInfo)
    print("Fireball ability activated in UE4.26!")
    
    -- 获取施法者
    local Caster = ActorInfo.AvatarActor
    if not Caster then
        self:EndAbilityForBP(Handle, ActorInfo,ActivationInfo,true, false)
        return
    end
    
    -- 播放施法动画
    self:PlayCastAnimation(ActorInfo)
    
    -- 获取目标（简化：获取前方1000单位的目标）
    local Target = self:GetTargetInFront(Caster)
    
    -- 延迟生成火球（配合动画）
    self:ScheduleFireballSpawn(Handle,ActorInfo,ActivationInfo, Caster, Target, 0.3)
end

-- 播放施法动画
function Fireball:PlayCastAnimation(ActorInfo)
    -- 加载蒙太奇
    print("PlayCastAnimation And Sound in UE4.26!")
    local Montage = UE.LoadObject(self.Config.MontagePath)
    if Montage and self.PlayMontageForAbility then
        self:PlayMontageForAbility(Montage,ActorInfo, 1.0)
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
function Fireball:ScheduleFireballSpawn(Handle,ActorInfo,ActivationInfo, Caster, Target, Delay)
    
    -- UE4.26的Timer用法
    local TimerDelegate = function()
        self:SpawnFireball(Handle,ActorInfo,ActivationInfo, Caster, Target)
    end
    
    UE.UKismetSystemLibrary.K2_SetTimerDelegate({self, TimerDelegate}, Delay, false)
end

-- 生成火球
function Fireball:SpawnFireball(Handle,ActorInfo,ActivationInfo, Caster, Target)
    -- 加载火球蓝图类
    local FireballClass = UE.UClass.Load(self.Config.ProjectilePath)
    if not FireballClass then
        print("Failed to load Fireball class!")
        self:EndAbilityForBP(Handle, ActorInfo, ActivationInfo,true, false)
        return
    end
    
    

    local SpawnLocation = Caster:K2_GetActorLocation() --+ Caster:GetActorForwardVector() * 100
    local SpawnRotation = Caster:K2_GetActorRotation()
    local Transform = Caster:GetTransform()
    local AlwaysSpawn = UE.ESpawnActorCollisionHandlingMethod.AlwaysSpawn

    
    print("Spawning Fireball Info " ,"FireballClass" , FireballClass,"Caster:", Caster:GetName() , " SpawnLocation:" , SpawnLocation , " SpawnRotation:" , SpawnRotation)
    -- UE4.26的SpawnActor方式
    local Projectile = Caster:GetWorld():SpawnActor(
        FireballClass,
        Transform,
        AlwaysSpawn,
        self,
        self,
        ""
    )
    
    if Projectile then
        -- 加载伤害效果
        local EffectClass = UE.UClass.Load(self.Config.GameAbilityEffectPath)
        
        -- 存储技能信息到Projectile（使用Lua表）
        Projectile.Damage = self.Config.Damage
        Projectile.EffectClass = EffectClass
        Projectile.SourceCaster = Caster
        
        -- 发射火球
        if Projectile.Launch then
            if Target then
                local Direction = (Target:K2_GetActorLocation() - SpawnLocation):GetSafeNormal()
                Projectile:Launch(Direction, self.Config.ProjectileSpeed)
            else
                print("向前 发射火球！")
                Projectile:Launch(Caster:GetActorForwardVector(), self.Config.ProjectileSpeed)
            end
        end
        
        
        -- 播放火球特效
        self:PlayFireballEffect(Projectile)
        
        print(string.format("Fireball spawned in UE4.26! Damage: %.0f", self.Config.Damage))
    end
    
    -- 技能结束
    self:EndAbilityForBP(Handle, ActorInfo, ActivationInfo,true, false)
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

return Fireball
-- UE4.26火球弹道
local FireballProjectile = UnLua.Class()

function FireballProjectile:ReceiveBeginPlay()
    print("FireballProjectile BeginPlay in UE4.26")
    
    -- 获取组件
    self.ProjectileMovement = self:GetComponentByClass(UE.UProjectileMovementComponent)
    self.ParticleSystem = self:GetComponentByClass(UE.UParticleSystemComponent)
    self.CollisionSphere = self:GetComponentByClass(UE.USphereComponent)
    
    -- 绑定碰撞事件（UE4.26方式）
    if self.CollisionSphere then
        self.CollisionSphere.OnComponentBeginOverlap:Add(self, self.OnBeginOverlap)
    end
    
    -- 设置生命周期（5秒后自动销毁）
    self:SetLifeSpan(5.0)
end

-- 发射火球
function FireballProjectile:Launch(Direction, Speed)
    if self.ProjectileMovement then
        self.ProjectileMovement.Velocity = Direction * Speed
        self.ProjectileMovement:Activate()
    end
end

-- 碰撞处理
function FireballProjectile:OnBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult)
    -- 避免击中自己和施法者
    if OtherActor == self or OtherActor == self.SourceCaster then
        return
    end
    
    -- 应用伤害
    if OtherActor and self.EffectClass and self.SourceCaster then
        local ASC = self.SourceCaster:GetAbilitySystemComponent()
        if ASC then
            local TargetASC = OtherActor:GetAbilitySystemComponent()
            if TargetASC then
                -- 创建GameplayEffect
                local SpecHandle = ASC:MakeOutgoingSpec(self.EffectClass, 1, ASC:MakeEffectContext())
                if SpecHandle and SpecHandle.Data then
                    -- 设置伤害值
                    SpecHandle.Data:SetSetByCallerMagnitude("Damage", self.Damage)
                    ASC:ApplyGameplayEffectSpecToTarget(SpecHandle, TargetASC)
                    
                    print(string.format("Fireball hit! Damage: %.0f", self.Damage))
                end
            end
        end
        
        -- 播放命中特效
        self:PlayHitEffect(OtherActor:K2_GetActorLocation())
        
        -- 销毁火球
        self:Destroy()
    end
end

-- 播放命中特效
function FireballProjectile:PlayHitEffect(Location)
    local HitEffectPath = "/Game/Effects/Fireball/Explosion_Effect.Explosion_Effect"
    local HitEffect = UE.LoadObject(HitEffectPath)
    
    if HitEffect then
        UE.UGameplayStatics.SpawnEmitterAtLocation(
            self:GetWorld(),
            HitEffect,
            Location,
            UE.FRotator.ZeroRotator,
            UE.FVector(1, 1, 1),
            true
        )
    end
    
    -- 播放命中音效
    local HitSoundPath = "/Game/Sounds/Fireball_Impact.Fireball_Impact"
    local HitSound = UE.LoadObject(HitSoundPath)
    if HitSound then
        UE.UGameplayStatics.PlaySoundAtLocation(
            self:GetWorld(), 
            HitSound, 
            Location
        )
    end
end

return FireballProjectile
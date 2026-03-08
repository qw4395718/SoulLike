--
-- DESCRIPTION
--
-- @COMPANY **
-- @AUTHOR **
-- @DATE ${date} ${time}
--

---@type WBP_PU_InterActPanel_C
local M = UnLua.Class()

--function M:Initialize(Initializer)
--end

--function M:PreConstruct(IsDesignTime)
--end

function M:Construct()
    print("ZYF_WBP_PU_InterActPanel_Construct")
    self:Test_InterActPanel()
end

function M:Test_InterActPanel()
    self:InitializeVirtualization(10)
    local test = UE.TArray(UE.FInterActOptionInfo())
    local testItem = UE.FInterActOptionInfo();
    for i = 1,12 do
        testItem.Index = i;
        testItem.OptionIcon = nil;
        testItem.OptionText = tostring(i);
        test:Add(testItem)
    end 
   
    for i = 1, test:Num() do
        print(test[i].Index,test[i].OptionIcon,test[i].OptionText)
    end

    self:UpdateBatch(test)
end

--function M:Tick(MyGeometry, InDeltaTime)
--end

function M:HandleScroll(wheelDelta)
    -- 补充视听效果

    self:HandleScroll(wheelDelta);
end

return M

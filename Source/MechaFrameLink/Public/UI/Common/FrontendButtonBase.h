#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "FrontendButtonBase.generated.h"

class UTextBlock;

/** Common project button with instance editable display text. */
UCLASS(Abstract, Blueprintable)
class MECHAFRAMELINK_API UFrontendButtonBase : public UCommonButtonBase
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Button")
    void SetButtonText(const FText& InButtonText);

protected:
    virtual void NativePreConstruct() override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Button", meta = (ExposeOnSpawn = "true"))
    FText ButtonText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> LabelText;

private:
    void RefreshButtonText();
};

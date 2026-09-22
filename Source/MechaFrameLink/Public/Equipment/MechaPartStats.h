#pragma once

#include "CoreMinimal.h"
#include "MechaPartStats.generated.h"

/** Additive demo stats. Weight is a cost; the other values are benefits. */
USTRUCT(BlueprintType)
struct MECHAFRAMELINK_API FMechaPartStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0"))
    float AP = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0"))
    float Attack = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0"))
    float Defense = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0"))
    float Weight = 0.0f;

    bool IsValid() const
    {
        return FMath::IsFinite(AP) && AP >= 0.0f
            && FMath::IsFinite(Attack) && Attack >= 0.0f
            && FMath::IsFinite(Defense) && Defense >= 0.0f
            && FMath::IsFinite(Weight) && Weight >= 0.0f;
    }

    FMechaPartStats& operator+=(const FMechaPartStats& Other)
    {
        AP += Other.AP;
        Attack += Other.Attack;
        Defense += Other.Defense;
        Weight += Other.Weight;
        return *this;
    }

    FMechaPartStats operator-(const FMechaPartStats& Other) const
    {
        FMechaPartStats Result;
        Result.AP = AP - Other.AP;
        Result.Attack = Attack - Other.Attack;
        Result.Defense = Defense - Other.Defense;
        Result.Weight = Weight - Other.Weight;
        return Result;
    }
};

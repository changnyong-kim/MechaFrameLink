#pragma once

#include "CoreMinimal.h"
#include "Equipment/MechaPartStats.h"
#include "MechaLoadout.generated.h"

/** One complete frame plus five attachment positions. */
UENUM(BlueprintType)
enum class EMechaEquipmentSlot : uint8
{
    Frame,
    LeftWeapon,
    RightWeapon,
    LeftShoulder,
    RightShoulder,
    Back
};

/** Only stable gameplay IDs are stored; definitions and meshes are resolved locally. */
USTRUCT(BlueprintType)
struct MECHAFRAMELINK_API FMechaLoadout
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
    FName FrameId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
    FName LeftWeaponId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
    FName RightWeaponId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
    FName LeftShoulderId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
    FName RightShoulderId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
    FName BackId;

    bool operator==(const FMechaLoadout& Other) const
    {
        return FrameId == Other.FrameId && LeftWeaponId == Other.LeftWeaponId
            && RightWeaponId == Other.RightWeaponId && LeftShoulderId == Other.LeftShoulderId
            && RightShoulderId == Other.RightShoulderId && BackId == Other.BackId;
    }
};

USTRUCT(BlueprintType)
struct MECHAFRAMELINK_API FMechaEquipmentComparison
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Comparison")
    FMechaLoadout PreviewLoadout;

    UPROPERTY(BlueprintReadOnly, Category = "Comparison")
    FMechaPartStats CurrentStats;

    UPROPERTY(BlueprintReadOnly, Category = "Comparison")
    FMechaPartStats PreviewStats;

    /** Preview minus current. Negative weight means a lighter configuration. */
    UPROPERTY(BlueprintReadOnly, Category = "Comparison")
    FMechaPartStats Delta;
};

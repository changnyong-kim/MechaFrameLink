# CommonUI 메인 메뉴와 재사용 버튼

작성일: 2026-09-18  
대상 엔진: Unreal Engine 5.8.2

## 1. 구현 목표

메인 메뉴에 두 개의 선택지를 제공한다.

- `출격`: 기존 로비 화면으로 이동
- `격납고`: 이후 제작할 격납고 화면으로 이동

두 버튼은 같은 외형의 `WBP_Button`을 사용하지만 표시 문구와 클릭 결과는 서로 달라야 한다.

## 2. 최종 구조

```text
UCommonButtonBase
└─ UFrontendButtonBase
   └─ WBP_Button

UFrontendActivatableWidget
├─ UMainMenuScreen
│  └─ WBP_MainMenu
└─ UHangarScreen
   └─ WBP_Hangar (제작 예정)
```

역할은 다음과 같이 나뉜다.

| 대상 | 책임 |
|---|---|
| `UCommonButtonBase` | CommonUI의 포커스, 입력 액션, 버튼 상태 처리 |
| `UFrontendButtonBase` | 프로젝트 공통 버튼 문구와 `LabelText` 연결 |
| `WBP_Button` | 버튼 레이아웃, 색상, 폰트와 애니메이션 |
| `UMainMenuScreen` | 출격과 격납고 클릭 이벤트 및 화면 이동 |
| `WBP_MainMenu` | 버튼 배치와 인스턴스별 문구 설정 |

## 3. 재사용 버튼 기반 클래스

관련 파일:

- `Source/MechaFrameLink/Public/UI/Common/FrontendButtonBase.h`
- `Source/MechaFrameLink/Private/UI/Common/FrontendButtonBase.cpp`

핵심 선언은 다음과 같다.

```cpp
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
};
```

### `ButtonText`

`EditAnywhere`이므로 `WBP_MainMenu`에 배치된 각 버튼 인스턴스에서 다른 값을 지정할 수 있다.

```text
SortieButton.ButtonText = 출격
HangarButton.ButtonText = 격납고
```

`ExposeOnSpawn`은 Blueprint에서 버튼을 동적으로 생성할 때도 초기 문구를 전달할 수 있게 한다.

### `LabelText`와 `BindWidget`

`WBP_Button`의 디자이너 트리에 이름이 정확히 `LabelText`인 `TextBlock`이 있어야 한다. Blueprint를 컴파일할 때 Unreal이 같은 이름과 호환 타입의 위젯을 C++ 프로퍼티에 연결한다.

이름이나 타입이 다르면 Blueprint 컴파일 오류가 발생한다.

### `NativePreConstruct`

```cpp
void UFrontendButtonBase::NativePreConstruct()
{
    Super::NativePreConstruct();
    RefreshButtonText();
}
```

`NativePreConstruct`는 실행 중뿐 아니라 에디터 디자인 미리보기에서도 호출될 수 있다. 따라서 인스턴스에 입력한 `ButtonText`가 디자이너와 게임 화면에 반영된다.

### 런타임 변경

```cpp
void UFrontendButtonBase::SetButtonText(const FText& InButtonText)
{
    ButtonText = InButtonText;
    RefreshButtonText();
}
```

실행 중 문구를 바꿀 때는 프로퍼티를 직접 변경하는 대신 이 함수를 사용한다. 값을 저장한 직후 화면도 함께 갱신되기 때문이다.

## 4. 메인 메뉴 버튼 바인딩

관련 파일:

- `Source/MechaFrameLink/Public/UI/MainMenu/MainMenuScreen.h`
- `Source/MechaFrameLink/Private/UI/MainMenu/MainMenuScreen.cpp`

`WBP_MainMenu`에는 다음 이름의 버튼이 필요하다.

```cpp
UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
TObjectPtr<UCommonButtonBase> SortieButton;

UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
TObjectPtr<UCommonButtonBase> HangarButton;
```

두 위젯 모두 `Is Variable`이 활성화되어야 한다. 현재 실제 Blueprint에도 다음 이름으로 저장되어 있다.

- `SortieButton`
- `HangarButton`

초기화 시 클릭 Delegate를 한 번 연결한다.

```cpp
SortieButton->OnClicked().AddUObject(this, &ThisClass::HandleSortieClicked);
HangarButton->OnClicked().AddUObject(this, &ThisClass::HandleHangarClicked);
```

출격은 `LobbyScreenClass`, 격납고는 `HangarScreenClass`를 UI Subsystem에 전달한다.

```text
버튼 클릭
→ UMainMenuScreen 핸들러
→ UFrontendUISubsystem::PushMainScreen
→ UFrontendRootWidget::PushMainScreen
→ MainStack에 새 CommonActivatableWidget 추가
```

## 5. C++과 Blueprint의 책임 구분

### C++에서 처리할 항목

- 버튼이 가진 데이터와 동작
- `BindWidget` 계약
- 클릭 이벤트 연결
- 화면 이동 요청
- 재사용 가능한 공통 로직

### Blueprint에서 처리할 항목

- 버튼 배치
- 여백과 정렬
- 색상, 폰트와 애니메이션
- 인스턴스별 `ButtonText`
- 이동할 화면 클래스 지정

이 구분을 지키면 디자인을 변경할 때 C++을 다시 컴파일할 필요가 줄어든다.

## 6. 이번 작업의 시행착오

처음에는 기존 버튼 클래스를 런타임에 복제하고 부모 패널 슬롯의 설정을 C++로 복사했다. 이 방식에서는 새 버튼이 기존 버튼보다 크게 늘어나는 문제가 발생했다.

원인은 위젯 클래스와 패널 슬롯이 서로 다른 객체이기 때문이다. 같은 `WBP_Button` 클래스를 생성해도 `VerticalBoxSlot`의 정렬, Padding과 Size Rule은 자동 복제되지 않는다.

다음과 같은 슬롯 타입별 복사 코드는 최종 구현에서 제거했다.

- `VerticalBoxSlot` 복사
- `HorizontalBoxSlot` 복사
- `OverlaySlot` 복사
- 버튼 내부 `TextBlock` 순회
- 런타임 버튼 강제 생성

최종적으로 버튼은 Blueprint 디자이너에 배치하고, C++은 동작과 데이터 계약만 제공하도록 변경했다.

## 7. 에디터에서 확인할 항목

### `WBP_Button`

1. 부모 클래스가 `FrontendButtonBase`인지 확인한다.
2. 내부 TextBlock 이름이 `LabelText`인지 확인한다.
3. Compile 결과에 오류가 없는지 확인한다.

### `WBP_MainMenu`

1. `SortieButton`과 `HangarButton`이 `VerticalBox_0` 아래에 있는지 확인한다.
2. 두 버튼의 `Is Variable`이 활성화되어 있는지 확인한다.
3. `ButtonText`가 각각 `출격`, `격납고`인지 확인한다.
4. 두 버튼의 정렬과 Padding이 같은지 확인한다.
5. Compile 후 Save한다.

## 8. 실행 확인

1. `L_Frontend`에서 PIE를 실행한다.
2. 첫 포커스가 `SortieButton`에 있는지 확인한다.
3. 출격 버튼에 `출격`이 표시되는지 확인한다.
4. 격납고 버튼에 `격납고`가 표시되는지 확인한다.
5. 출격을 누르면 로비 화면으로 이동하는지 확인한다.
6. Back 입력으로 메인 메뉴에 복귀하는지 확인한다.
7. 격납고 화면 클래스가 지정된 뒤에는 격납고 Push와 Back도 확인한다.

## 9. 다음 구현

다음 단계에서는 `UHangarScreen`을 부모로 하는 `WBP_Hangar`를 만들고 `WBP_MainMenu`의 `HangarScreenClass`에 지정한다. 초기 목업은 제목, 장비 슬롯 영역, 기체 프리뷰 영역과 상세 정보 영역만 배치하면 된다.


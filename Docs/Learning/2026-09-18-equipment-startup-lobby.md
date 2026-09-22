# 게임 시작 시 장비 초기화와 로비 직접 진입

## 문제와 목적

이전 구현은 `UHangarScreen::NativeOnActivated`에서만 `InitializeHangar`를 호출했다. 그래서 격납고에 먼저 들어가면 정상이지만, 최초 실행에서 메인 메뉴 → 로비로 바로 이동하면 Equipment Subsystem 안에 카탈로그와 프레임이 준비되지 않았다.

Subsystem 객체가 자동 생성되는 것과 그 객체의 도메인 데이터가 준비되는 것은 별개다. 초기화 책임을 GameInstance 수명에 맞추고, 격납고와 로비가 공통으로 준비된 데이터를 사용하도록 수정했다.

## 변경 구조

| 코드 | 책임 |
| --- | --- |
| `UMechaEquipmentSubsystem::Initialize` | GameInstance 시작 시 부품 검색 완료 콜백 등록 |
| `HandleInitialAssetScanCompleted` | 화면 방문과 무관하게 기본 장비 초기화 |
| `EnsureEquipmentReady` | 준비 여부 확인, 실패한 준비 재시도, 중복 호출 시 현재 구성 보존 |
| `CompleteSetup` | 준비 보장과 전체 Loadout 검증, 완료 플래그 갱신 |
| `UMainMenuScreen::HandleSortieClicked` | 검증 성공 시에만 로비 화면 열기 |
| `UHangarScreen::NativeOnActivated` | 준비 확인과 화면 구독·표시 |
| `Deinitialize` | 종료 상태 표시, 데이터·Delegate 정리 |

`InitializeHangar`는 화면과 무관한 이름인 `EnsureEquipmentReady`로 변경했다. 기존 이름으로 노드를 직접 연결한 사용자 Blueprint가 있다면 새 함수로 교체해야 한다. 프로젝트 C++ 호출부는 모두 수정했다.

## Unreal 초기화 순서와 중요한 코드

`UGameInstance::Init()`는 내부 Subsystem Collection을 초기화한다. 이때 `UGameInstanceSubsystem::Initialize(FSubsystemCollectionBase&)`가 호출된다. 생성자에서 콘텐츠를 읽는 대신 이 수명 주기 함수를 사용한다.

다만 에디터의 최초 Asset Manager 검색은 늦게 끝날 수 있다. 카탈로그가 아직 검색되지 않은 상태를 파츠가 하나도 없는 상태로 오인하지 않도록 다음 API를 사용한다.

```cpp
UAssetManager::CallOrRegister_OnCompletedInitialScan(
    FSimpleMulticastDelegate::FDelegate::CreateUObject(
        this, &ThisClass::HandleInitialAssetScanCompleted));
```

이미 검색이 끝났으면 즉시 호출하며, 아직 진행 중이면 완료 후 호출한다. `EnsureEquipmentReady`도 `HasInitialScanCompleted`를 검사해 검색 중에는 false와 설명을 반환한다. 로비 버튼을 일찍 눌러도 빈 Loadout으로 진입하지 않는다.

`CreateUObject`는 약한 객체 참조를 사용하는 Delegate지만, PIE 종료 후 GC 전까지 객체가 살아 있을 수 있다. 따라서 `bSubsystemActive`도 검사해 종료된 GameInstance의 뒤늦은 콜백이 데이터를 다시 만들지 않도록 했다.

## 실행 흐름

1. 게임 시작 → Equipment Subsystem 생성과 `Initialize` 호출.
2. 부품 검색 완료 → 카탈로그 로드 → 기본 프레임과 능력치 준비.
3. 메인 메뉴에서 곧바로 로비 선택 → `CompleteSetup` 호출.
4. 준비가 안 됐으면 다시 준비를 시도한다. 준비 또는 검증 실패 시 메뉴에 머물고 `OnLobbyEntryFailed(Error)`를 발행한다.
5. 성공하면 현재 Loadout을 완료 상태로 표시하고 로비 화면을 연다.
6. 이후 격납고에 들어가도 초기 장비로 되돌리지 않고 같은 구성을 편집한다.

`IsEquipmentReady`는 데이터 준비 여부다. `IsSetupComplete`는 현재 구성이 로비 진입용 검증을 통과했는지다. 두 상태를 구분하므로 게임 시작 시 기본 장비를 준비했다고 해서 사용자 세팅 완료까지 자동으로 처리하지 않는다. 직접 로비 선택을 누르면 기본 구성으로 검증과 완료를 수행한다.

기본 구성 정책은 그대로 유지한다. 첫 정렬 프레임 1개가 장착되고 장비 5개는 비어 있다. 프레임만 필수라는 현재 규칙에서는 이 구성으로 로비에 갈 수 있다. 초기 무기 지급 정책을 새로 추가한 것은 아니다.

## 실패 처리

- 검색 중: 아직 준비되지 않았다는 오류를 반환하고 진입 차단.
- 카탈로그 오류 또는 프레임 없음: 준비 실패, 다음 호출에서 재시도 가능.
- 준비 이후 ID 해석 또는 전체 구성 검증 실패: 진입 차단, `bSetupComplete`를 false로 변경.
- 실패해도 사용자가 편집한 Loadout ID는 덮어쓰지 않는다.
- 메인 메뉴는 `OnLobbyEntryFailed`를 Blueprint에 제공한다. 실제 오류 팝업 디자인은 연결하지 않았으며 로그에도 이유를 남긴다.

## 회귀 검증

`MechaFrameLink.Hangar.DirectLobbyStartup`은 실제 `UGameInstance::InitializeStandalone`을 실행한다. 격납고 Widget을 만들거나 격납고 초기화 함수를 호출하지 않은 상태에서 다음을 검사한다.

- GameInstance가 Subsystem을 생성하고 기본 프레임·능력치를 준비했는가.
- 격납고 방문 없이 `CompleteSetup`에 성공하는가.
- 이후 무기를 바꿔도 다른 화면의 준비 확인이 해당 구성을 보존하는가.
- 이 테스트 인스턴스의 카탈로그를 비우면 완료 검증을 거부하고 기존 완료 상태를 취소하는가.
- 명시적인 준비 없이 완료 함수만 호출해도 준비 경로를 거치는가.

테스트 종료 시 GameInstance와 생성한 World Context를 정리한다. 기존 4개 테스트도 함께 실행해 장착·해제·비교와 카탈로그 동작의 회귀를 확인한다. 실제 메인 메뉴 버튼 클릭과 로비 화면 렌더링의 PIE 검증을 대체하지는 않는다.

검증 결과: `MechaFrameLinkEditor Win64 Development` 빌드 성공. `MechaFrameLink.Hangar` 테스트는 새 `DirectLobbyStartup`을 포함해 5개 모두 성공했고 실행 종료 코드는 0이다. 보고서는 `Saved/Automation/Hangar/index.json`에 있다. 최초 샌드박스 빌드 실행은 새 컴파일 로그를 남기지 않고 종료되어, 승인된 외부 빌드로 검증했다. 기존 ModelViewViewModel 의존성 선언 경고와 GameFeatureData 검색 규칙 관련 시작 로그는 별도 문제로 남아 있다.

## 배운 점

이전 테스트도 `InitializeHangar`를 먼저 호출했기 때문에 실제 사용자가 격납고를 건너뛰는 경로를 확인하지 못했다. 테스트 준비 단계에서 항상 초기화해 버리면 초기화 누락 버그를 가릴 수 있다. 이제 화면 방문 없이 GameInstance 수명 주기에서 시작하는 테스트를 별도로 유지한다.

세션 전체에서 필요한 데이터는 세션 시작에 준비하고, 화면은 그 데이터를 조회·편집해야 한다. 진입 지점에서도 유효성을 확인하면 초기 준비 실패를 정상 흐름으로 처리할 수 있다.

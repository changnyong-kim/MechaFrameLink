# 격납고 장착과 비교 데이터 흐름

> 후속 UI 구현: 기존 데이터 로직에 연결한 `WBP_HangarMock`이 추가됐다. 실행 방법은 [격납고 텍스트 목업](./2026-09-18-hangar-mock-ui.md)을 참고한다. 아래 Blueprint 연결 절은 원래 데이터 기반을 설명하는 내용이다.

## 목적과 확정 범위

통메시도 하나의 프레임 파츠다. MechaFrameLink는 프레임 1개와 장비 5개, 총 6개 장착 위치를 사용한다. 프리셋 없이 현재 기체 한 대를 편집하고 세팅 완료 후 로비에서 매칭하는 흐름의 데이터 기반을 구현했다.

| 장착 위치 | 부품 종류 | 해제 |
| --- | --- | --- |
| Frame | Frame | 불가, 다른 프레임으로 교체 |
| LeftWeapon | Weapon | 가능 |
| RightWeapon | Weapon | 가능 |
| LeftShoulder | Shoulder | 가능 |
| RightShoulder | Shoulder | 가능 |
| Back | Back | 가능 |

좌우 무기는 같은 목록, 좌우 어깨도 같은 목록을 사용한다. 같은 부품을 양쪽에 장착할 수 있고, 양쪽 각각의 능력치를 합산한다. 보유 수량 제한은 없다. 현재는 프레임만 필수이며 무기 필수 조건, 적재 한도, EN 제한은 추가하지 않았다.

## 구현 구조

| 클래스·구조체 | 책임 |
| --- | --- |
| `EMechaEquipmentSlot` | 6개 장착 위치 |
| `FMechaLoadout` | 각 위치의 PartId만 보관 |
| `FMechaEquipmentComparison` | 현재 수치, 후보 적용 수치, 차이, 후보 구성 |
| `UMechaEquipmentLibrary` | 호환 검사, 전체 능력치 계산, 부작용 없는 후보 비교 |
| `UMechaEquipmentSubsystem` | 실행 중 현재 구성 보관, 장착·해제, 변경 알림, 세팅 완료 상태 |
| `UHangarScreen` | 활성화 시 데이터 준비 확인, 변경 구독, Blueprint 갱신 이벤트, 로비 화면 전환 진입점 |

파일은 `Source/MechaFrameLink/Public/Equipment/`, `Private/Equipment/`, `Public/UI/Hangar/`, `Private/UI/Hangar/`에 있다.

별도 ScreenModel은 아직 만들지 않았다. 확정 구성은 Subsystem이 소유하고, 선택 슬롯·포커스·스크롤·후보 표시 상태는 후속 Widget 구현이 소유하면 된다. 지금 규모에서는 이 정도 경계로 충분하다.

## Unreal 개념

`UGameInstanceSubsystem`은 GameInstance 수명 동안 유지된다. 격납고 Widget이 비활성화되거나 로비로 전환되어도 현재 Loadout은 사라지지 않는다. 게임을 종료하면 초기화된다. 파일 저장, 계정 데이터, 서버 복제는 구현하지 않았다. 같은 GameInstance 안의 분할 화면 사용자별 분리도 이번 범위가 아니다.

Subsystem의 `Catalog`는 `UPROPERTY(Transient)`로 보관해 GC에 의해 사라지지 않게 한다. 카탈로그가 부품 정의를 보관하며, 메시와 아이콘은 계속 Soft Reference 상태다. 화면 진입만으로 3D 리소스를 로드하지 않는다.

`UBlueprintFunctionLibrary`의 계산 함수는 현재 장비를 수정하지 않는다. 후속 서버 검증에서도 이 규칙을 재사용할 수 있지만, 현재 구현 자체가 RPC나 서버 권한 검증을 제공하는 것은 아니다.

CommonUI 화면은 `NativeOnActivated`에서 변경 Delegate를 구독하고 `NativeOnDeactivated`에서 해제한다. 다시 활성화할 때 현재 데이터를 즉시 한 번 전달하므로 비활성화 중 놓친 이벤트를 재생할 필요가 없다.

## 실행 흐름

### 첫 진입과 재진입

1. GameInstance 시작 시 `UMechaEquipmentSubsystem::Initialize()`가 자동 호출된다. 격납고 방문은 필요 없다.
2. Asset Manager의 최초 검색이 완료되면 `EnsureEquipmentReady`가 카탈로그와 기본 구성을 준비한다. 이미 검색이 끝났으면 즉시 실행한다.
3. `SortOrder`, PartId 순으로 가장 앞선 프레임을 기본 장착하고 5개 장비 위치는 비워 둔다. 현재 샘플에서는 `frame.standard`다.
4. 프레임이 하나도 없거나 카탈로그 검증에 실패하면 오류를 반환하고 초기화를 완료하지 않는다. 검색 중인 경우도 로비 진입을 허용하지 않는다.
5. `UHangarScreen::NativeOnActivated()`는 이미 준비된 Subsystem을 얻어 준비 상태를 확인하고, 변경 이벤트 구독 후 현재 데이터를 표시한다.
6. 재진입의 `EnsureEquipmentReady` 호출은 장비를 초기값으로 덮어쓰지 않는다. 메인 메뉴에서 바로 로비로 가는 경로도 `CompleteSetup`으로 준비와 검증을 보장한다.

초기 구현은 1번 준비를 격납고 활성화에 의존해 메인 메뉴 → 로비 경로를 놓쳤다. 수정 근거와 회귀 테스트는 [게임 시작 시 장비 초기화와 로비 직접 진입](./2026-09-18-equipment-startup-lobby.md)을 참고한다.

### 후보 비교와 취소

```cpp
FMechaEquipmentComparison Comparison;
FText Error;
const bool bCanEquip = Equipment->CompareCandidate(
    EMechaEquipmentSlot::LeftWeapon,
    TEXT("weapon.cannon"),
    Comparison,
    Error);
```

성공하면 `Comparison.CurrentStats`, `PreviewStats`, `Delta`를 화면에 표시한다. `Delta = PreviewStats - CurrentStats`다. 중량 차이가 음수면 더 가벼워진다는 뜻이다.

비교는 현재 Loadout을 복사하고 후보 위치만 바꾼다. 기존 부품 위에 새 부품 수치를 더하지 않는다. 후보 선택을 취소하면 화면의 비교 표시만 지우면 된다. 확정 구성을 수정하지 않았기 때문에 데이터 롤백은 필요 없다. 향후 3D 후보를 표시한다면 취소 시 현재 Loadout 외형을 다시 적용해야 한다.

`PartId = None`은 해제 미리보기다. 프레임 해제는 실패한다. 실패한 비교 결과는 빈 값으로 초기화되므로, 호출 결과가 false일 때 이전 비교값을 계속 표시하지 않는다.

### 장착과 해제

- `EquipPart(Slot, PartId, Error)`는 같은 비교·검증 경로를 통과한 결과만 현재 구성에 반영한다. 빈 ID는 거부한다.
- `UnequipPart(Slot, Error)`는 해당 위치를 None으로 바꾸는 검증 경로를 사용한다.
- 알 수 없는 ID, 맞지 않는 종류, 잘못된 슬롯, 프레임 해제는 실패하며 기존 상태를 유지한다.
- 실제 변경이 있으면 캐시된 능력치를 갱신하고 `bSetupComplete = false`로 바꾼 뒤 `OnLoadoutChanged`를 한 번 발행한다.
- 같은 부품을 다시 장착하거나 이미 빈 위치를 해제하는 경우 성공하되 상태 변경과 이벤트는 발생하지 않는다.

`CalculateStats`는 후보 하나뿐 아니라 전체 Loadout의 ID와 종류를 확인한다. 합산 결과가 비유한 값으로 넘치는 경우도 실패로 처리한다. 검증 실패 시 부분 합계를 출력하지 않는다.

### 세팅 완료와 로비

`CompleteSetup`는 `EnsureEquipmentReady`로 준비를 보장한 뒤 현재 구성을 다시 검증하고 완료 플래그를 켠다. 실패하면 기존 완료 플래그도 해제한다. `UHangarScreen::CompleteSetupAndOpenLobby`는 활성화된 격납고, Equipment, UI Subsystem, `LobbyScreenClass`가 있는지 확인하고 세팅 완료 후 기존 `PushMainScreen`으로 로비를 연다. 메인 메뉴의 `HandleSortieClicked`도 동일한 `CompleteSetup` 검증에 성공해야 로비를 연다.

로비 화면은 같은 GameInstance의 `MechaEquipmentSubsystem`에서 `GetCurrentLoadout`, `GetCurrentStats`, `IsSetupComplete`를 읽을 수 있다. 실제 매칭 시스템은 아직 연결하지 않았다. 완료 플래그는 데이터 검증 완료를 뜻하며, 로비 화면 생성 성공이나 서버 준비 상태를 뜻하지 않는다.

## Blueprint에서 붙이는 순서

현재 `WBP_Hangar` 레이아웃은 생성하지 않았고, C++ 화면 베이스와 호출 계약을 준비했다.

1. `HangarScreen`을 부모로 Widget Blueprint를 만들고 메인 메뉴의 `HangarScreenClass`에 지정한다.
2. 격납고의 `LobbyScreenClass`에 로비 Widget 클래스를 지정한다.
3. `OnEquipmentUpdated`에서 Equipment의 현재 Loadout과 능력치를 읽어 슬롯 목록과 수치를 갱신한다.
4. 슬롯 선택 시 `Equipment.GetPartsForSlot`을 목록에 넣는다.
5. 후보 포커스 시 `CompareCandidate`, 장착 버튼에서 `EquipPart`, 해제 버튼에서 `UnequipPart`를 호출한다. 각 반환값과 Error를 처리한다.
6. 세팅 완료 버튼에서 `CompleteSetupAndOpenLobby`를 호출한다.
7. `OnHangarInitializationFailed`에서는 오류를 표시하고 장착·완료 입력을 막는다.

아직 메시를 교체하거나 소켓에 장비를 부착하는 코드는 없다. 프레임별 실제 리소스의 소켓 이름과 좌우 배치가 정해지면 외형 적용 컴포넌트를 연결한다.

## 검증

`MechaFrameLinkEditor Win64 Development` 빌드 성공. `MechaFrameLink.Hangar` 자동화 테스트 4개 모두 성공, 프로세스 종료 코드 0을 확인했다.

| 테스트 | 확인 내용 |
| --- | --- |
| PartCatalog | ID 조회, 종류별 목록, 중복·음수 데이터 거부 |
| AssetDiscovery | 실제 샘플 DataAsset 검색 |
| EquipmentRules | 6개 위치 합산, 동일 부품 중복 장착, 교체 차이, 해제 차이, 프레임 교체·해제 금지, 잘못된 ID·종류·슬롯 거부 |
| EquipmentSession | 샘플 초기화, 비교 후 원본 유지, 세팅 완료, 재진입 유지, 실패·동일 장착 시 상태 보존, 해제 후 갱신, GameInstance 간 분리 |

결과는 `Saved/Automation/Hangar/index.json`, 로그는 `Saved/Logs/HangarAutomation.log`다. UI 전환과 3D 외형은 PIE로 검증하지 않았다. GameFeatureData 검색 규칙 누락과 기존 ModelViewViewModel 의존성 경고는 별도 프로젝트 설정 문제로 남아 있다.

## 설계에서 피한 문제

실패한 새 접근을 반복한 것은 없다. 이번 구현은 첫 빌드와 테스트에서 통과했다. 대신 다음 오류가 생기지 않도록 계약과 테스트를 명확히 했다.

- 좌우를 부품 종류로 나누면 같은 리소스에 데이터가 중복된다. 종류 4개와 위치 6개를 분리했다.
- PartId를 중복 제거한 뒤 합산하면 양쪽 동일 무기의 수치가 한 번만 계산된다. 장착 위치 단위로 합산했다.
- 후보 선택마다 실제 Loadout을 바꾸면 취소 복구가 복잡해진다. 비교용 사본을 사용했다.
- 화면이 장착 구성을 소유하면 닫았다가 열 때 구성이 사라진다. GameInstance Subsystem으로 수명을 옮겼다.
- 전체 프레임도 일반 파츠로 교체하지만 프레임 미장착은 허용하지 않아 유효한 기체의 기준을 단순하게 유지했다.

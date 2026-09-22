# 격납고 부품 데이터 기반

> 후속 구현: 사용자 확인으로 통메시도 파츠이며, 프레임 1개 + 장비 5개인 총 6개 장착 위치로 확정했다. 아래는 1차 데이터 구현 시점의 기록이다. 현재 장착·해제·비교 구현은 [격납고 장착과 비교 데이터 흐름](./2026-09-18-hangar-equipment-flow.md)을 참고한다.

## 목적과 현재 범위

MechaFrameLink의 격납고는 현재 기체 한 대의 장착, 해제, 능력치 비교를 제공하고 세팅 완료 후 로비로 이동하는 방향이다. 프리셋과 파일 저장은 이번 범위에서 제외한다. 기존 문서의 제너레이터, FCS, 부스터 슬롯도 새 요구에 포함되지 않는다.

이번 구현은 부품의 고정 정보, 카탈로그 조회, 비교용 기본 능력치, 샘플 DataAsset까지다. 장착 상태인 Loadout, 화면 모델, 메시 교체, 로비 전환은 아직 구현하지 않았다.

슬롯 수는 확인 중이다. 사용자 설명은 통메시 1개, 좌우 무기 2개, 좌우 어깨 2개, 등 장비 1개여서 합계 6개지만 마지막에 5슬롯으로 표현됐다. 통메시를 별도 선택으로 보고 장비 5슬롯으로 셀 것인지, 모두 포함한 6슬롯인지 확인하기 전에는 슬롯 enum과 Loadout을 확정하지 않는다. 이와 무관하게 부품 종류는 Frame, Weapon, Shoulder, Back의 네 가지로 구성할 수 있다.

## 구조와 책임

| 코드 | 책임 |
| --- | --- |
| `Public/Data/MechaPartDefinition.h` | 부품 ID, 종류, 표시 정보, 능력치, 외형 참조 |
| `Private/Data/MechaPartDefinition.cpp` | Primary Asset ID와 에디터 데이터 검증 |
| `Public/Equipment/MechaPartStats.h` | AP, 공격, 방어, 중량 및 합산·차이 계산 |
| `Public/Data/MechaPartCatalog.h` | ID 조회 및 종류별 정렬 목록 API |
| `Private/Data/MechaPartCatalog.cpp` | Asset Manager 검색, 정의 로드, 카탈로그 검증 |
| `Scripts/create_hangar_sample_parts.py` | 기존 에셋을 덮어쓰지 않는 샘플 생성 |

부품 종류와 장착 위치는 다른 개념이다. Weapon 부품 하나를 좌우 무기 위치에서 공통으로 선택하고, Shoulder 부품도 좌우에서 공통으로 선택한다. 부품 정의에 왼쪽용·오른쪽용 ID나 장착 여부를 넣지 않는다. 실제 위치와 소켓은 후속 외형 적용 단계의 책임이다.

## Unreal 개념과 중요한 코드

`UMechaPartDefinition`은 `UPrimaryDataAsset`이다. 콘텐츠 브라우저에서 Data Asset을 생성할 때 `MechaPartDefinition`을 선택한다. 일반 Blueprint 클래스가 아니라 이 클래스의 데이터 인스턴스를 만든다.

`GetPrimaryAssetId()`는 `MechaPart:에셋이름`을 반환한다. 게임용 `PartId`는 `weapon.rifle`처럼 별도로 지정한다. 에셋 이름을 바꾸더라도 게임용 ID는 유지할 수 있다. 같은 Primary Asset 타입 안에서는 에셋 이름도 고유하게 유지해야 한다.

`Config/DefaultGame.ini`의 Asset Manager 설정은 `/Game/MechaFrameLink/Data/Parts`를 검색한다. 이 폴더의 부품 에셋은 AlwaysCook 대상으로 지정했다. 패키징 실행 검증은 아직 하지 않았다.

카탈로그 사용 예시는 다음과 같다.

```cpp
UMechaPartCatalog* Catalog = NewObject<UMechaPartCatalog>(Owner);
FText Error;
if (Catalog->LoadFromAssetManager(Error))
{
    TArray<UMechaPartDefinition*> Weapons = Catalog->GetParts(EMechaPartCategory::Weapon);
    UMechaPartDefinition* Rifle = Catalog->FindPart(TEXT("weapon.rifle"));
}
```

이 예시의 카탈로그를 여러 프레임에 걸쳐 사용하려면 소유 객체의 `UPROPERTY`로 보관해야 한다. `Outer`만 지정해서는 GC 보호를 보장하지 않는다. 현재는 테스트에서 직접 생성하며 실제 GameInstance/화면 연결은 후속 구현이다.

부품 정의만 동기 로드하며, 아이콘과 메시의 `TSoftObjectPtr`는 이 시점에 로드하지 않는다. 수십 개의 작은 정의 데이터를 다루는 단순한 출발점이다. 실제 외형 적용 때는 별도로 비동기 로드하고 오래된 요청의 완료 결과를 무시해야 한다.

프레임 외형은 `FrameMesh`의 Skeletal Mesh, 장비 외형은 `AttachmentMesh`의 Static Mesh를 사용한다. 반대 타입 필드에 참조를 넣으면 에디터 검증에서 오류를 낸다. 현재 샘플에는 아트가 없으므로 누락 아이콘·메시는 경고로 처리하며 데이터 계산은 허용한다. 실제 리소스의 소켓, 리깅, 좌우 배치 호환성은 아직 검증하지 않았다.

## 실행 흐름

1. Asset Manager가 설정된 폴더의 `MechaPart` 에셋을 검색한다.
2. `LoadFromAssetManager`가 정의를 읽고 `Build`로 카탈로그를 구성한다.
3. 빈 ID·표시 이름, 잘못된 종류, 음수·비유한 능력치, 중복 ID를 거부한다.
4. 전체 검증 성공 후에만 기존 맵을 새 맵으로 교체한다. 실패하면 기존 조회 결과를 유지하고 오류 설명을 반환한다.
5. 화면은 `GetParts(Category)`로 정렬된 목록을 얻고 `FindPart(PartId)`로 상세 정보를 조회하게 된다. 정렬은 `SortOrder` 우선, 동일하면 ID 순이다.

알 수 없는 ID를 기본 부품으로 조용히 대체하지 않는다. 조회 실패는 `nullptr`로 반환하므로 후속 장착 처리에서 명시적으로 거부할 수 있다.

## 샘플 데이터와 능력치

에셋 위치는 `Content/MechaFrameLink/Data/Parts/`다. 프레임 2개, 무기 2개, 어깨 2개, 등 장비 2개를 생성했다.

| PartId | AP | 공격 | 방어 | 중량 |
| --- | ---: | ---: | ---: | ---: |
| frame.standard | 1000 | 0 | 100 | 1000 |
| frame.heavy | 1500 | 0 | 180 | 1600 |
| weapon.rifle | 0 | 100 | 0 | 120 |
| weapon.cannon | 0 | 180 | 0 | 240 |
| shoulder.missile | 0 | 80 | 0 | 150 |
| shoulder.shield | 0 | 0 | 80 | 180 |
| back.wing | 0 | 0 | 20 | 100 |
| back.pack | 100 | 0 | 40 | 200 |

모든 숫자는 UI 비교용 임시 값이다. 네 값은 현재 단순 합산용이며 공격력은 전투 DPS가 아니다. 중량 상한, 에너지, 이동속도 등의 계산 규칙은 아직 추가하지 않았다.

`FMechaPartStats::IsValid()`는 **부품 원본 값**에 적용한다. 교체 후보 총합에서 현재 총합을 뺀 비교 결과는 음수일 수 있으므로 이 검증을 적용하면 안 된다. AP·공격·방어는 증가가 이득, 중량은 감소가 이득이라는 표시 규칙은 후속 UI에서 반영해야 한다.

## 검증 결과와 재현 방법

- `MechaFrameLinkEditor Win64 Development` 빌드 성공.
- `MechaFrameLink.Hangar.PartCatalog` 성공: ID 조회, 종류 필터링, 알 수 없는 ID, 중복 ID 거부, 실패 시 기존 카탈로그 보존, 음수 능력치 거부.
- `MechaFrameLink.Hangar.AssetDiscovery` 성공: 실제 Asset Manager 설정으로 샘플 프레임·무기 검색.
- 자동화 보고서: `Saved/Automation/Hangar/index.json`. 두 테스트 모두 성공, 테스트 프로세스 종료 코드 0.
- 에디터에서 샘플을 열어 이름·수치를 수정하고 저장한 뒤 같은 테스트를 실행할 수 있다. 샘플 ID를 바꾸거나 삭제하면 AssetDiscovery의 고정 샘플 확인도 함께 갱신해야 한다.
- 슬롯별 장착·해제·비교 및 PIE 화면 검증은 아직 수행 대상이 아니다.

테스트 실행 명령:

```powershell
& 'G:/1_Unreal/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe' 'G:/1_Unreal/2_Project/MechaFrameLink/MechaFrameLink.uproject' '-ExecCmds=Automation RunTests MechaFrameLink.Hangar; Quit' '-TestExit=Automation Test Queue Empty' '-ReportExportPath=G:/1_Unreal/2_Project/MechaFrameLink/Saved/Automation/Hangar' -DisablePlugins=ModelContextProtocol -unattended -nop4 -nosplash -nullrhi -nosound
```

## 시행착오와 남은 작업

- 샌드박스 실행은 Zen 캐시 경로 접근에 실패했다. 승인된 외부 실행으로 샘플을 생성했다.
- 생성 스크립트는 8개 에셋 저장을 마쳤지만 GameFeatureData 검색 규칙 누락과 MCP HTTP 포트 충돌 때문에 커맨드렛 전체 종료 코드는 1이었다. 저장된 에셋은 별도 자동화 테스트에서 실제 로드해 확인했다. 해당 기존 플러그인 설정 문제를 격납고 코드 오류와 구분한다.
- 테스트 코드 추가 후 실행 중인 에디터가 DLL을 잠가 링크가 실패했다. 에디터를 정상 종료하고 다시 빌드해 해결했다. 첫 테스트 실행은 이전 DLL을 읽어 테스트를 찾지 못했으며, 최종 빌드 이후 재실행에서는 두 테스트를 찾고 통과했다.
- 빌드에는 기존 ModelViewViewModel 플러그인 의존성 선언 경고가 남아 있다.
- 다음 단계는 슬롯 수 확인, Loadout과 현재 구성의 실행 중 보관, 후보 비교와 장착·해제, 세팅 완료 검증 순서다. 프리셋은 추가하지 않는다.

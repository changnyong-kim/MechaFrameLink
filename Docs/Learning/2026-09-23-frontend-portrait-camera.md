# 프런트엔드 캐릭터와 고정 카메라 구도

## 목적과 구조

참조 이미지처럼 화면 왼쪽을 비우고 오른쪽에 캐릭터 상반신을 배치한다. 기존 `L_Frontend`의 `BP_LevelVisuals` 조명·안개와 `MI_Floor` 바닥을 활용한다. 텍스트와 메뉴 Widget Blueprint는 변경하지 않는다.

모든 레벨 변경과 저장은 Unreal MCP를 통해 Unreal Editor에서 수행했다. C++ 변경은 없으며 `Content/Frontend/Maps/L_Frontend.umap`은 Diversion 관리 대상이다.

## 최종 배치

| 항목 | 설정 |
| --- | --- |
| 레벨 | `/Game/Frontend/Maps/L_Frontend` |
| 캐릭터 액터 라벨 | `Frontend_PreviewCharacter` |
| 메시 | `/Game/Assets/LevelVisuals/Characters/Meshes/SKM_Manny` |
| 캐릭터 위치 / 회전 / 스케일 | `(0, 0, 50)` / Pitch `0`, Yaw `80`, Roll `0` / `(1, 1, 1)` |
| 애니메이션 | `AnimationSingleNode`, `/Game/Assets/LevelVisuals/Characters/Mannequins/Animations/Manny/MM_Idle` |
| 자세 | Saved Position `0`, Saved Playing `false`, Saved Looping `true`, Saved Play Rate `1` |
| 카메라 액터 라벨 | `Frontend_PortraitCamera` |
| 카메라 위치 / 회전 | `(-225, -40, 196)` / Pitch `0.3`, Yaw `0`, Roll `0` |
| 카메라 화각 / 종횡비 | 수평 FOV `40`, Aspect Ratio `1.833333`, Constrain Aspect Ratio `true` |
| 자동 카메라 활성화 | Auto Activate For Player `Player0` |

위치는 Unreal 단위 cm, 회전은 도 단위다. 바닥 Cube의 상면이 Z=50이므로 캐릭터 발 위치를 이에 맞췄다. 애니메이션은 참조와 비교하기 쉽도록 첫 자세에서 정지한다.

## 실행 흐름과 Unreal 개념

1. 레벨의 SkeletalMeshActor가 Manny 메시와 같은 스켈레톤을 사용하는 Idle 시퀀스를 읽는다.
2. CameraActor의 CameraComponent가 구도와 화각을 결정한다. 에디터 자유 시점과 저장된 CameraActor는 별개다.
3. Player0 자동 활성화 설정으로 게임 시작 시 카메라 사용을 요청한다. 기존 프런트엔드 PlayerController는 CommonUI 화면을 생성한다.
4. 기존 메뉴의 불투명 배경은 3D 장면을 가린다. 이번 작업은 레벨 구성 범위이며 UI 배경 투명화는 포함하지 않는다.

## 검증

- MCP로 최종 액터·카메라 프로퍼티를 조회하고 레벨 저장 후 `is_dirty=false`를 확인한다.
- 에디터에서 `Frontend_PortraitCamera`를 Pilot하여 구도를 확인한다. 검은 여백은 뷰포트와 카메라의 종횡비 차이에 따른 것이다.
- PIE 시작·종료가 가능함을 확인했다. 기존 UI 배경 때문에 PIE의 전체 캐릭터 구도는 직접 시각 검증하지 못했으며, 비교 이미지는 에디터 카메라 뷰다.
- 게임 실행 상태에서 `PlayerCameraManager_0`와 `CameraActor_0`의 실제 위치가 모두 `(-225, -40, 196)`, 회전이 `(Pitch=0.3, Yaw=0, Roll=0)`임을 MCP로 확인했다. 런타임 카메라 컴포넌트의 FOV `40`, 종횡비 `1.833333`, 종횡비 제한 설정도 확인했다. 에디터 시점만 맞춘 것이 아니라 게임의 카메라에 적용된 상태다.
- MCP 캡처: `Saved/Screenshots/FrontendPortrait_Review.png`. 캡처 원본을 그대로 저장했다.
- 비교 시 캡처의 유효 카메라 영역만 기준으로 좌표를 정규화한다. 수동 좌표 오차는 이미지 전체의 픽셀 유사도와 다르다.

## 시행착오

- `SK_Mannequin`이라는 이름만으로 메시를 선택하면 스켈레톤 에셋과 혼동할 수 있다. 최종 메시 `SKM_Manny`와 `MM_Idle`의 스켈레톤 호환성을 기준으로 선택했다. UE4 메시를 확인하던 중의 설정은 최종 결과에 사용하지 않는다.
- 이 환경의 액터 트랜스폼 도구는 일부 항목만 전달하면 생략한 위치가 초기화되는 동작을 보였다. 최종 설정에는 위치·회전·스케일을 모두 전달했다.
- `CaptureViewport`에 빈 트랜스폼 객체를 전달하면 원점 시점이 적용됐다. 현재 카메라를 캡처할 때는 `captureTransform=null`을 사용했다.
- Pilot 중 액터 위치만 갱신하면 에디터 시점이 이전 위치에 남아 카메라 프록시가 보일 수 있었다. `SetCameraTransform`으로 뷰포트 시점도 함께 맞췄다.
- Pilot 상태에서 뷰포트를 이동하면 실제 CameraActor의 배치도 바뀔 수 있다. 게임 검증 중 구도 이탈을 발견해 저장된 목표 위치로 복원했다. 에디터 자유 탐색 전에 Pilot을 해제해야 한다.
- `bUpdateAnimationInEditor` 프로퍼티 쓰기는 도구에서 거부됐다. 정지 자세 검증에 해당 설정을 의존하지 않았다.

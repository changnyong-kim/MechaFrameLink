# 실행 가능한 격납고 텍스트 목업

## 목적

디자인과 3D 외형 없이 실제 장비 데이터를 조작하는 최소 UI를 만들었다. 왼쪽에는 6개 장착 위치와 현재 부품 ID·이름, 가운데에는 선택한 위치에 호환되는 부품 목록, 오른쪽에는 현재 수치와 후보 적용 수치·차이를 표시한다.

메인 메뉴의 격납고 버튼은 새 `WBP_HangarMock`에 연결했다. 세팅 완료는 기존 `WBP_Lobby`로 이동한다. 로비 자체 디자인과 매칭 시스템은 이번에 추가하지 않았다.

## 구조

| 파일·에셋 | 역할 |
| --- | --- |
| `Public/UI/Hangar/HangarMockScreen.h`, `Private/UI/Hangar/HangarMockScreen.cpp` | C++로 UMG 레이아웃 생성 및 입력 처리 |
| `UHangarMockButton` | 동적으로 생성한 UButton 클릭을 native Delegate로 전달 |
| `UHangarScreen::HandleLoadoutChanged` | 기존 Blueprint 알림과 native 목업 갱신을 함께 지원하도록 virtual로 변경 |
| `Content/Frontend/UI/Hangar/WBP_HangarMock.uasset` | HangarMockScreen을 부모로 하는 실행용 Widget Blueprint |
| `Content/Frontend/UI/MainMenu/WBP_MainMenu.uasset` | HangarScreenClass를 목업으로 지정 |
| `Scripts/setup_hangar_mock.py` | Widget Blueprint 생성 및 화면 클래스 연결 |
| `Private/Tests/HangarMockUITests.cpp` | 실제 게임 창에서 Widget 버튼 이벤트와 화면 전환 검증 |

목업의 시각 트리는 Blueprint Designer에 수작업으로 저장한 배치가 아니라, C++ `RebuildWidget`에서 생성한다. 디자이너용 완성 UI를 만들 때에는 이 목업과 별도 화면을 만들고 기존 Equipment Subsystem API를 재사용하면 된다.

## Unreal 개념과 실행 순서

1. 메인 메뉴의 기존 `HandleHangarClicked`가 설정된 `WBP_HangarMock`을 CommonUI 스택에 추가한다.
2. `RebuildWidget`에서 WidgetTree가 비어 있으면 Border, VerticalBox, HorizontalBox, ScrollBox, TextBlock과 Button을 만든다.
3. `Super::RebuildWidget`를 호출해 CommonUI와 UUserWidget의 정상 생성 흐름을 유지한다.
4. `NativeOnActivated`에서 기존 장비 준비 확인을 실행하고, 현재 구성을 화면에 표시한다.
5. 슬롯이나 후보 클릭은 로컬 선택 상태만 바꾼다. `Refresh`가 카탈로그에서 목록을 읽고 비교 결과를 갱신한다.
6. 장착·해제는 Equipment Subsystem으로 전달한다. 변경 Delegate를 받으면 다시 화면을 갱신한다.

UMG `UButton::OnClicked`는 Dynamic Multicast Delegate다. 슬롯·부품 ID를 캡처하는 C++ 람다를 직접 붙이는 대신, 작은 `UHangarMockButton`이 Dynamic Delegate를 수신하고 `FSimpleDelegate`로 전달한다. 화면 객체를 캡처할 때 `CreateWeakLambda`를 사용한다.

`UPROPERTY(Transient)`는 런타임 위젯 참조를 GC로부터 보호한다. 장착 데이터는 여전히 GameInstance Subsystem에 있고 화면에는 선택 위치, 후보 ID, 상태 메시지만 있다. 화면을 닫거나 다시 열어도 실제 장비 구성이 유지된다.

이 목업은 변경 시 슬롯·부품 목록을 다시 만드는 단순한 방식이다. 목록이 커지면 CommonListView 같은 가상화 목록으로 교체할 수 있다. 현재는 6개 위치와 종류별 샘플 2개라 구현 단순성을 우선했다.

## 조작 방법

프로젝트를 열어 `L_Frontend`에서 Play하고 메인 메뉴의 격납고 버튼을 누른다.

- 왼쪽 슬롯 버튼: 장착 위치 선택. `>`가 현재 선택을 뜻한다.
- 가운데 부품 버튼: 후보 선택. `[Equipped]`는 현재 장착 부품이다.
- `Equip candidate`: 후보를 실제 장착한다. 미선택 또는 이미 장착된 부품이면 비활성화된다.
- `None / Preview unequip`: 해제했을 때의 수치를 미리 본다.
- `Unequip slot`: 선택 위치의 부품을 해제한다. Frame에서는 비활성화된다.
- `Clear comparison`: 비교 표시만 지우고 현재 장비를 유지한다.
- `Complete setup > Lobby`: 현재 장비를 검증하고 기존 로비를 연다.
- `Back to menu`: 격납고를 닫는다. 장착 변경은 실행 중 유지된다.

현재 및 후보 수치는 AP, Attack, Defense, Weight다. 차이는 후보 적용 후 − 현재이며, Weight는 감소가 유리하다. 파츠명은 샘플 DataAsset에 저장된 한국어를 그대로 표시한다.

## 검증 방법

실제 게임 창에서 `MechaFrameLink.Hangar.MockUI` 자동화 테스트를 실행한다. 이 테스트는 ClientContext 전용이며 에디터용 데이터 테스트와 실행 환경이 다르다.

테스트는 메인 메뉴에 연결된 클래스를 읽어 격납고를 열고, 생성된 UMG 버튼을 찾아 `OnClicked` 이벤트를 호출한다. 후보 선택 시 원본 유지, 무기 장착·해제, 비교 취소, 프레임 교체, 세팅 완료 후 로비 클래스, 뒤로 가기, 재진입 시 구성 유지를 검사한다. OS 마우스 클릭 주입 검증은 아니다.

마지막에는 격납고 비교 화면을 열어 두고 Unreal의 `FScreenshotRequest`로 UI를 포함한 `Saved/Screenshots/HangarMock.png`를 요청한다. 보고서는 `Saved/Automation/HangarMockUI/`, 실행 로그는 `Saved/Logs/HangarMockPlay.log`다.

검증 결과: Editor Development 빌드 성공. 실제 `-game` 실행에서 `MechaFrameLink.Hangar.MockUI` 테스트가 성공했다. 1280×800 엔진 스크린샷을 직접 확인해 6개 슬롯, ID·한국어 이름, 후보 수치와 차이, 하단 조작 버튼이 잘리지 않고 표시되는 것을 확인했다. 최종 확인용 게임 창은 중장 프레임 상태에서 왼쪽 무기 라이플 후보를 비교하는 화면으로 남겨 두었다. 테스트가 바꾼 구성은 해당 실행 세션 안에만 존재하며 파일에 저장하지 않는다.

## 시행착오

- UWidget에 이미 `Slot` 멤버가 있어 같은 이름의 함수 인자·지역 변수가 C4458 경고를 오류로 발생시켰다. 장비 위치 이름을 `EquipmentSlot`으로 변경했다.
- 새 테스트 파일이 첫 증분 빌드에서 수집되지 않아 `-NoUBTMakefiles`로 다시 수집하고 실제 컴파일 여부를 확인했다.
- Windows computer-use 캡처가 `SetIsBorderRequired` 인터페이스 오류로 실패했다. 재시도도 실패했고, 접근성 텍스트에는 창 테두리만 노출됐다. 임의 좌표 클릭으로 성공을 추정하지 않고 Unreal 내부 UI 테스트와 엔진 스크린샷으로 확인하도록 전환했다.
- 기존 프로젝트의 GameFeatureData 등록 오류로 Python 커맨드렛의 종료 코드가 1일 수 있다. 생성 완료 로그와 실제 게임에서 로드한 화면 클래스·동작을 별도로 확인한다.

## 범위

프리셋, 파일 저장, 실제 메시 교체, 장비 소켓 부착, 로비 매칭은 추가하지 않았다. 이 목업은 현재 장비 데이터와 화면 연결을 바로 실행하며 확인하기 위한 도구다.

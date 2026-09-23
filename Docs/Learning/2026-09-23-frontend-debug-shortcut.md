# 프런트엔드 UI 표시 테스트 단축키

## 목적과 구조

MechaFrameLink 게임 화면에서 `Ctrl+F1`으로 프런트엔드 루트 표시를 전환한다.
숨겨진 WBP 자체의 키 이벤트에 의존하지 않고, 살아 있는 PlayerController가
게임 뷰포트의 입력을 받아 루트의 명시적인 C++/Blueprint 계약을 호출한다.

실행 흐름은 다음과 같다.

1. `AFrontendPlayerController::BeginPlay`가 루트를 생성해 화면에 추가한다.
2. 기본 로컬 플레이어가 게임 뷰포트의 `OnOverrideInputKey`에 바인딩한다.
3. CommonUI 라우팅 전에 전달된 `Ctrl+F1` 눌림을 처리한다.
4. C++에 `BlueprintImplementableEvent`로 선언한 `DebugToggleFrontendVisibility`를 호출한다.
5. WBP_FrontendRoot의 이벤트가 기존 `ToggleVisibilityForTest` 함수를 실행한다.
6. `Visible`이면 `Collapsed`, 그 외 상태이면 `Visible`로 설정한다.

`FindFunction`과 문자열 이름에 의존하는 수동 `ProcessEvent` 호출은 사용하지 않는다.
Blueprint 이벤트 호출 자체에는 엔진의 리플렉션 경로가 있지만, C++ 호출부의
함수 이름과 시그니처는 컴파일 시 확인할 수 있다.

## 입력과 수명 관리

- 단축키 등록 및 C++ 처리 코드는 `!UE_BUILD_SHIPPING`에서만 포함한다.
- 이벤트 선언과 BP 그래프는 남지만 Shipping에서는 이 키로 호출하지 않는다.
- Shift, Alt, Command가 추가로 눌린 조합은 받지 않는다.
- `IE_Pressed`에서만 전환하며 반복 이벤트로 재전환하지 않는다.
- Ctrl을 먼저 떼더라도 처리했던 F1의 해제 이벤트를 소비한다.
- 뷰포트 델리게이트는 단일 바인딩이므로 기존 핸들러를 복사해 보관하고 먼저 호출한다.
  기존 핸들러가 처리하지 않은 입력만 테스트 단축키 처리로 넘긴다.
- 기본 로컬 플레이어만 등록한다. 분할 화면의 모든 플레이어를 지원하는 기능은 아니다.
- `EndPlay`에서는 현재 바인딩을 자신이 소유할 때만 기존 핸들러를 복원한다.

## 범위와 주의점

UE 5.8의 `ContentBrowserCommands.cpp`에는 Ctrl+F1이 즐겨찾기 단축키로 등록되어 있다.
사용자는 이 중복을 알고 게임 화면에 포커스가 있을 때 사용하는 원안을 선택했다.
에디터 전체 또는 운영체제 전역 단축키가 아니다. Slate 위젯이 먼저 소비한 입력은
게임 뷰포트까지 도달하지 않을 수 있다.

`Collapsed`는 CommonUI 화면의 비활성화를 뜻하지 않는다. 화면 표시와 히트 테스트를
끄더라도 활성 화면 스택과 Menu 입력 모드는 유지된다. 또한 이 테스트 함수는 이전
Visibility 값을 보관하지 않으므로 SelfHitTestInvisible 같은 상태를 원복하지 않는다.
현재 루트 CDO의 기본값은 MCP 조회 결과 `SelfHitTestInvisible`이다. 따라서 최초 입력은
`Visible`로 전환하고, 다음 입력부터 `Collapsed`와 `Visible`을 왕복한다.

## 검증

- Development Editor 전체 빌드 성공(UHT 포함).
- 에디터 재실행 후 명시적 BP 이벤트 노출 확인, 기존 토글 함수에 실행 핀 연결.
- BP 컴파일/저장 성공 및 `is_dirty=false` 확인.
- Git diff 공백 검사 통과. Content는 Git에 추가하지 않았다.
- Claude는 코드와 빌드 순서를 검토했다. Claude 세션의 MCP 연결 실패로 실제 키 입력은
  검증하지 못했다. 기본 표시 상태 복원 우려는 부분수용하되 기존 테스트 의미는 유지한다.
- 최종 바인딩 수정 후 MAIN에서 PIE 시작/종료를 두 차례 확인했다. 이는 초기화와 종료의
  확인이며, Ctrl+F1 실제 왕복이나 반복키/포커스 처리 통과를 뜻하지 않는다.
- 최종 수정안도 Claude에게 전달했으나 MCP 연결 진단 대화로 전환되어 추가 리뷰의
  완료 결과는 회수하지 못했다. 최종 수정안은 MAIN이 엔진 구현과 대조 검토했다.
- 사용자가 구현 후 Ctrl+F1 기능이 정상 동작한다고 확인했다(2026-09-23).
  포커스/반복키 등 세부 조건별 검증 결과는 별도로 확인되지 않았다. MAIN의 PIE는 종료해 두었다.

## 시행착오

처음에는 `OnOverrideInputKey().IsBound()`인 경우 등록을 건너뛰었다. MAIN의 PIE 실행에서
`Frontend debug shortcut not bound` 경고를 확인했다. 엔진 `SLevelViewport.cpp:4955`가
PIE의 에디터 명령 처리를 위해 이미 이 델리게이트를 사용하기 때문이다. 기존 핸들러를
복사해 먼저 실행하는 체인 방식으로 수정했다. 같은 파일의 엔진 구현도 이전 핸들러를
우선 호출하는 방식을 사용한다. 수정 후 Development Editor 재빌드에 성공했다.

MCP `SlateInspectorToolset.PressKey`는 FKeyEvent에 수정키 정보를 담지만 OS의 키 상태는
변경하지 않는다. 반면 `FSlateApplication::GetModifierKeys()`는 플랫폼의 상태를 조회한다.
따라서 합성 Ctrl+F1 호출 성공만으로 실제 Ctrl+F1 왕복 동작을 검증했다고 볼 수 없다.

검증할 항목은 UI가 보이는 상태와 접힌 상태의 왕복 전환, 버튼 포커스가 있는 상태,
F1 단독 및 추가 수정키의 무시, 길게 누를 때 한 번만 전환, PIE 재시작 후 재바인딩이다.

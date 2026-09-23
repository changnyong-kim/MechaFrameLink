# 프런트엔드 루트 표시 상태 테스트 함수

## 목적과 구조

`WBP_FrontendRoot`에 `ToggleVisibilityForTest` Blueprint 함수를 추가했다. C++ 변경 없이 Unreal MCP로 노드를 생성·연결하고 컴파일·저장했다.

## 실행 흐름

1. 함수 호출 시 Self의 `Get Visibility`를 읽는다.
2. `Equal (Enum)`으로 정확히 `Visible`인지 비교한다.
3. Branch의 True는 `Set Visibility(Collapsed)`, False는 `Set Visibility(Visible)`을 실행한다.

`Hidden`, `Collapsed`, 두 Hit Test Invisible 상태는 모두 Visible로 전환된다. `Is Visible` 대신 열거형 비교를 사용하여 요청한 정확한 상태 판정을 유지한다. Collapsed는 화면 표시와 레이아웃 공간을 없앤다.

## 호출 방법과 검증

- 기존 Event Tick → Set Visibility(Visible) 연결을 끊었다. 매 프레임 Visible을 강제하면 토글 결과가 유지되지 않는다.
- 함수는 자동 실행되지 않는다. 테스트 버튼이나 PlayerController처럼 숨겨진 루트 바깥에서 호출할 수 있는 경로에 연결한다. 숨겨진 위젯 내부 버튼만으로 다시 표시할 수는 없다.
- MCP로 비교 핀의 Visible 값, 두 Set Visibility의 Collapsed/Visible 값, Branch 실행 연결을 재조회했다. Blueprint 컴파일 호출이 오류 없이 완료됐고 저장 후 dirty=false를 확인했다.
- 실제 입력 트리거 연결과 런타임 왕복 호출 검증은 이번 작업에 포함하지 않았다.

## 시행착오

한국어 에디터에서 DSL 생성 도구가 영문 Self 노드 타입을 찾지 못했다. 노드를 개별 생성하고 실제 핀 정보를 조회해 연결하는 방식으로 구성했다. DSL 읽기 결과가 빈 문자열이어서 검증에는 노드·핀 연결 조회를 사용했다.

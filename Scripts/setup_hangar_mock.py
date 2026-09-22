"""Create the native-layout hangar Blueprint and wire existing frontend screens."""
import unreal

ROOT = "/Game/Frontend/UI/Hangar"
PATH = ROOT + "/WBP_HangarMock"
unreal.EditorAssetLibrary.make_directory(ROOT)
hangar = unreal.EditorAssetLibrary.load_asset(PATH) if unreal.EditorAssetLibrary.does_asset_exist(PATH) else None
if not hangar:
    factory = unreal.WidgetBlueprintFactory()
    factory.set_editor_property("parent_class", unreal.HangarMockScreen)
    hangar = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "WBP_HangarMock", ROOT, None, factory)
if not hangar:
    raise RuntimeError("Could not create hangar Widget Blueprint")

unreal.BlueprintEditorLibrary.compile_blueprint(hangar)
hangar_class = unreal.EditorAssetLibrary.load_blueprint_class(PATH)
lobby_class = unreal.EditorAssetLibrary.load_blueprint_class("/Game/Frontend/UI/Lobby/WBP_Lobby")
main = unreal.EditorAssetLibrary.load_asset("/Game/Frontend/UI/MainMenu/WBP_MainMenu")
main_class = unreal.EditorAssetLibrary.load_blueprint_class("/Game/Frontend/UI/MainMenu/WBP_MainMenu")
if not (hangar_class and lobby_class and main and main_class):
    raise RuntimeError("Frontend screen classes are missing")

unreal.get_default_object(hangar_class).set_editor_property("lobby_screen_class", lobby_class)
unreal.get_default_object(main_class).set_editor_property("hangar_screen_class", hangar_class)
for asset in (hangar, main):
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError("Could not save " + asset.get_path_name())
unreal.log("HANGAR_MOCK_READY: MainMenu -> WBP_HangarMock -> " + lobby_class.get_path_name())

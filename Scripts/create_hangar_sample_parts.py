"""Run with UnrealEditor-Cmd -run=pythonscript; existing assets are never overwritten."""
import unreal

ROOT = "/Game/MechaFrameLink/Data/Parts"
# These are demo values for UI comparison, not combat balance.
SAMPLES = [
    ("DA_Frame_Standard", "frame.standard", unreal.MechaPartCategory.FRAME, "표준 프레임", 1000, 0, 100, 1000),
    ("DA_Frame_Heavy", "frame.heavy", unreal.MechaPartCategory.FRAME, "중장 프레임", 1500, 0, 180, 1600),
    ("DA_Weapon_Rifle", "weapon.rifle", unreal.MechaPartCategory.WEAPON, "라이플", 0, 100, 0, 120),
    ("DA_Weapon_Cannon", "weapon.cannon", unreal.MechaPartCategory.WEAPON, "캐논", 0, 180, 0, 240),
    ("DA_Shoulder_Missile", "shoulder.missile", unreal.MechaPartCategory.SHOULDER, "미사일 유닛", 0, 80, 0, 150),
    ("DA_Shoulder_Shield", "shoulder.shield", unreal.MechaPartCategory.SHOULDER, "실드 유닛", 0, 0, 80, 180),
    ("DA_Back_Wing", "back.wing", unreal.MechaPartCategory.BACK, "윙 유닛", 0, 0, 20, 100),
    ("DA_Back_Pack", "back.pack", unreal.MechaPartCategory.BACK, "백팩", 100, 0, 40, 200),
]

unreal.EditorAssetLibrary.make_directory(ROOT)
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
for order, (name, part_id, category, label, ap, attack, defense, weight) in enumerate(SAMPLES):
    path = ROOT + "/" + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        unreal.log("Keep existing sample: " + path)
        continue
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", unreal.MechaPartDefinition)
    part = asset_tools.create_asset(name, ROOT, unreal.MechaPartDefinition, factory)
    if not part:
        raise RuntimeError("Could not create " + path)
    part.set_editor_property("part_id", part_id)
    part.set_editor_property("category", category)
    part.set_editor_property("display_name", label)
    part.set_editor_property("description", "장착 및 능력치 비교용 샘플입니다. 외형 리소스를 지정해 주세요.")
    part.set_editor_property("sort_order", order)
    stats = unreal.MechaPartStats()
    for key, value in [("ap", ap), ("attack", attack), ("defense", defense), ("weight", weight)]:
        stats.set_editor_property(key, float(value))
    part.set_editor_property("stats", stats)
    if not unreal.EditorAssetLibrary.save_loaded_asset(part):
        raise RuntimeError("Could not save " + path)
unreal.log("MechaFrameLink hangar samples are ready.")

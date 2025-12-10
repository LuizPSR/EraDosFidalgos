#include "Army.hpp"

#include "Characters.hpp"
#include "imgui.h"
#include "MapGenerator.hpp"
#include "Components/Province.hpp"

ArmyModule::ArmyModule(const flecs::world& ecs)
{
    const auto qTopmostRealm = ecs.query_builder<>("TopmostRealm")
        .without<InRealm>()
        .with<InRealm>("$this").src("$province")
        .build();
    const auto qPlayerRealm = ecs.query_builder<>("PlayerRealm")
        .with<RulerOf>("$this").src<Player>()
        .build();

    void(ecs.component<MovingArmies>().add(flecs::Singleton));

    ecs.system<MovingArmies, const TileMap>("DrawArmyMovement")
        .each([=](MovingArmies &movement, const TileMap &map)
        {
            flecs::entity e = movement.mProvince;
            Province &province = e.get_mut<Province>();
            ProvinceArmy &army = e.get_mut<ProvinceArmy>();

            bool open = true;
            std::string title("Mover Tropas##" + std::to_string(e.id()));
            const auto &playerRealm = qPlayerRealm.first();
            if (ImGui::Begin(title.c_str(), &open, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Província %s tem %d tropas", province.name.c_str(), army.mAmount);

                ImGui::InputInt("Quantidade selecionada", &movement.mAmount);
                movement.mAmount = std::clamp<int>(movement.mAmount, 0, army.mAmount);

                const auto x = province.mPosX, y = province.mPosY;
                for (int xd = x-1; xd <= x+1; xd += 1)
                for (int yd = y-1; yd <= y+1; yd += 1)
                {
                    if (xd < 0 || xd >= map.width) continue;
                    if (yd < 0 || yd >= map.height) continue;

                    flecs::entity pe = map.tiles[xd][yd];
                    auto &p = pe.get_mut<Province>();
                    auto &a = pe.get_mut<ProvinceArmy>();
                    const auto &top = qTopmostRealm.set_var("province", pe).first();

                    bool isMove = playerRealm == top;
                    std::string text = isMove ?
                        "Mover para " + p.name + "##" + std::to_string(pe.id()) :
                        "Atacar " + p.name + "##" + std::to_string(pe.id());
                    if (ImGui::Button(text.c_str()))
                    {
                        if (isMove)
                        {
                            a.mAmount += movement.mAmount;
                            army.mAmount -= movement.mAmount;
                        } else
                        {
                            army.mAmount -= movement.mAmount;
                            if (a.mAmount < movement.mAmount)
                            {
                                a.mAmount = movement.mAmount - a.mAmount;
                                void(pe.remove(ecs.component<InRealm>(), top));
                                void(pe.add<InRealm>(playerRealm));
                            } else
                            {
                                a.mAmount -= movement.mAmount;
                                // TODO: reduce relations
                            }
                        }
                    }
                }
            }
            ImGui::End();
            if (!open) ecs.remove<MovingArmies>();
        });
}

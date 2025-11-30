#include "EstatePower.hpp"

#include <algorithm>
#include <filesystem>
#include <SDL3/SDL.h>

#include "Characters.hpp"
#include "imgui.h"
#include "toml++/toml.hpp"

std::vector<EstatePowerEvent> readEstatePowerEventsFromFile()
{
    const auto &path = std::filesystem::path(SDL_GetBasePath()) / "Assets/PowerEvents.toml";
    toml::table tbl = toml::parse_file(path.string());
    std::vector<EstatePowerEvent> result;

    tbl["event"].as_array()->for_each([&](const toml::table &element)
    {
        EstatePowerEvent event;
        event.mMessage = element["message"].as_string()->get();
        element["choices"].as_array()->for_each([&](const toml::table &choiceElement)
        {
            EstateEventChoice choice;
            choice.mText = choiceElement["text"].as_string()->get();
            choice.mCost = uint64_t(choiceElement["cost"].as_floating_point()->get() * 100);
            choiceElement["power_changes"].as_array()->for_each([&](const toml::array &array)
            {
                std::string estateString = array.get(0)->as_string()->get();
                SocialEstate estate;
                int powerChange = array.get(1)->as_integer()->get();
                if (estateString == "Commoners") estate = SocialEstate::Commoners;
                else if (estateString == "Nobility") estate = SocialEstate::Nobility;
                else if (estateString == "Clergy") estate = SocialEstate::Clergy;
                else exit(1);
                choice.mPowerChanges.emplace_back(estate, powerChange);
            });
            event.mChoices.push_back(choice);
        });
        result.push_back(event);
    });

    return result;
}

void DoEstatePowerSystems(const flecs::world& ecs, const GameTime& timers)
{
    void(ecs.component<EstatePowers>()
        .add(flecs::Singleton)
        .add<EstatePowers>());

    const auto powerEvents = readEstatePowerEventsFromFile();

    ecs.system<>("PowerEventsSpawner")
        .tick_source(timers.mMonthTimer)
        .run([=](const flecs::iter &it)
        {
            const auto &ecs = it.world();
            if (Random::GetFloat() < 0.2f)
            {
                auto event = powerEvents[Random::GetIntRange(0, powerEvents.size() - 1)];
                void(ecs.entity()
                    .set<EstatePowerEvent>(event)
                    .add<FiredEvent>());
            }
        });

    ecs.system<const EstatePowerEvent, EstatePowers, Character>("PowerEvents")
        .term_at(2).src<Player>()
        .tick_source(timers.mTickTimer)
        .each([](flecs::entity entity, const EstatePowerEvent &event, EstatePowers &powers, Character &player)
        {
            void(entity.add<PausesGame>());

            std::string title = "Estate Event##" + std::to_string(entity.id());
            if (ImGui::Begin(title.data()))
            {
                ImGui::TextWrapped("%s", event.mMessage.data());
                for (const auto &choice: event.mChoices)
                {
                    bool isDisabled = choice.mCost > player.mMoney;
                    if (isDisabled) ImGui::BeginDisabled();
                    if (ImGui::Button(choice.mText.data()))
                    {
                        for (const auto &[estate, change]: choice.mPowerChanges)
                        {
                            switch (estate)
                            {
                            case SocialEstate::Commoners:
                                powers.mCommonersPower = std::clamp((int)powers.mCommonersPower + change, -128, 127);
                                break;
                            case SocialEstate::Nobility:
                                powers.mNobilityPower = std::clamp((int)powers.mNobilityPower + change, -128, 127);
                                break;
                            case SocialEstate::Clergy:
                                powers.mClergyPower = std::clamp((int)powers.mClergyPower + change, -128, 127);
                                break;
                            }
                        }
                        player.mMoney -= choice.mCost;
                        entity.destruct();
                    }
                    if (isDisabled) ImGui::EndDisabled();

                    if (ImGui::BeginItemTooltip())
                    {
                        ImGui::Text("Cost: %.2f", choice.FloatCost());
                        for (const auto &[estate, change]: choice.mPowerChanges)
                        {
                            std::string estateString;
                            switch (estate)
                            {
                            case SocialEstate::Commoners: estateString = "Commoners"; break;
                            case SocialEstate::Nobility: estateString = "Nobility"; break;
                            case SocialEstate::Clergy: estateString = "Clergy"; break;
                            }
                            ImGui::Text("%s: %d", estateString.data(), change);
                        }
                        ImGui::EndTooltip();
                    }
                }
            }
            ImGui::End();
        });
}

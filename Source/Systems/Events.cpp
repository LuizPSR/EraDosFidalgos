#include <cmath>
#include <imgui.h>

#include "Events.hpp"
#include "Characters.hpp"
#include "Game.hpp"

#include "EstatePower.hpp"

void CenterNextImGuiWindow()
{
    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2{0.5f, 0.5f});
}

void DoCharacterBirthSystems(const flecs::world& ecs, const flecs::timer &tickTimer)
{
    // Starts pregnancy events for married couples
    ecs.observer<const Character, const GameTime>()
        .with(ecs.component<MarriedTo>(), flecs::Wildcard)
        .with(Gender::Female)
        .event(flecs::Monitor)
        .each([](flecs::iter &it, size_t i, const Character &c, const GameTime &gameTime)
        {
            flecs::entity characterEntity = it.entity(i);
            const auto &ecs = it.world();
            if (it.event() == flecs::OnAdd)
            {
                flecs::entity spouseEntity = characterEntity.target(ecs.component<MarriedTo>());
                void(characterEntity.child()
                    .add<FiredEvent>()
                    .set<PregnancySaga>({
                        .father = spouseEntity,
                        .mother = characterEntity,
                        .dynasty = characterEntity.target<DynastyMember>(),
                    }));
            } else if (it.event() == flecs::OnRemove)
            {
                characterEntity.children([](flecs::entity child)
                {
                    if (child.has<PregnancySaga>()) return child.destruct();
                });
            }
        });

    ecs.system<PregnancySaga, const GameTime>()
        .with<FiredEvent>()
        .tick_source(tickTimer)
        .each([](flecs::iter &it, size_t i, PregnancySaga &saga, const GameTime &gameTime)
        {
            const auto &entity = it.entity(i);
            auto *father = saga.father.try_get<Character>();
            auto *mother = saga.mother.try_get<Character>();
            if (father == nullptr || mother == nullptr) return entity.destruct();

            switch (saga.stage)
            {
            case PregnancySaga::Attempt:
                {
                    // TODO: make random deterministic
                    if (Random::GetFloat() < 0.2f)
                    {
                        saga.stage = PregnancySaga::Announce;
                    }
                    void(entity.remove<PausesGame>()
                        .remove<FiredEvent>()
                        .set<EventSchedule>(EventSchedule::InXDays(gameTime, 30)));
                }
                break;
            case PregnancySaga::Announce:
                {
                    void(entity.add<PausesGame>());
                    auto name = "Evento - Gravidez##" + std::to_string(entity.id());
                    CenterNextImGuiWindow();
                    if (ImGui::Begin(name.data()))
                    {
                        ImGui::Text("%s e %s vão ter uma criança.", father->mName.data(), mother->mName.data());
                        if (ImGui::Button("Close"))
                        {
                            saga.stage = PregnancySaga::Birth;
                            // TODO: make random deterministic
                            int variation = Random::GetIntRange(30, 60);
                            void(entity.remove<PausesGame>()
                                .remove<FiredEvent>()
                                .set<EventSchedule>(EventSchedule::InXDays(gameTime, 30 * 7 + variation)));
                        }
                    }
                    ImGui::End();
                    break;
                }
            case PregnancySaga::Birth:
                {
                    saga.child = BirthChildCharacter(it.world(), *father, *mother, saga.dynasty);
                    saga.stage = PregnancySaga::BirthAnnounce;
                }
            case PregnancySaga::BirthAnnounce:
                {
                    void(entity.add<PausesGame>());
                    auto name = "Evento - Nascimento##" + std::to_string(entity.id());
                    const auto *child = saga.child.try_get<Character>();
                    CenterNextImGuiWindow();
                    if (ImGui::Begin(name.data()))
                    {
                       ImGui::Text("O filho de %s e %s nasceu: %s", father->mName.data(), mother->mName.data(), child ? child->mName.data() : nullptr);
                       if (ImGui::Button("Close"))
                       {
                           void(entity.destruct());
                       }
                    }
                    ImGui::End();
                    break;
                }
            }
        });
}

void DoAdultMarriageSystems(const flecs::world& ecs, flecs::timer)
{
    // Adds marriage planning to unmarried adults
    ecs.observer<const Character, const GameTime>()
        .with<AgeClass>(AgeClass::Adult)
        .without(ecs.component<MarriedTo>(), flecs::Wildcard)
        .event(flecs::Monitor)
        .each([](flecs::iter &it, size_t i, const Character &c,  const GameTime &gameTime)
        {
            flecs::entity characterEntity = it.entity(i);
            if (it.event() == flecs::OnRemove)
            {
                characterEntity.children([](flecs::entity child)
                {
                    if (child.has<PlanMarriageSaga>()) return child.destruct();
                });
            } else if (it.event() == flecs::OnAdd)
            {
                void(characterEntity.child()
                    .set<EventSchedule>(EventSchedule::InXDays(gameTime, 30))
                    .set<PlanMarriageSaga>({ characterEntity }));
            }
        });

    // TODO: implement the behavior
}

void DoEventSchedulingSystems(const flecs::world& ecs, flecs::timer tickTimer)
{
    void(ecs.component<FiredEvent>()
        .add(flecs::Trait)
        .add(flecs::CanToggle));

    // Adds a fired tag to events which are scheduled to the past
    ecs.system<const EventSchedule, const GameTime>()
        .kind(flecs::PreUpdate)
        .tick_source(tickTimer)
        .order_by<EventSchedule>([](
            flecs::entity_t, const EventSchedule *a,
            flecs::entity_t, const EventSchedule *b)
        {
            return (*b < *a) - (*a < *b);
        })
        .run([](flecs::iter &it)
        {
            while (it.next())
            {
                auto &gameTime = it.field_at<const GameTime>(1, 0);
                for (const size_t i : it)
                {
                    const auto &schedule = it.field_at<const EventSchedule>(0, i);
                    if (schedule.mTimeSecs > gameTime.mTimeSecs)
                        return it.fini();
                    void(it.entity(i).add<FiredEvent>());
                }
            }
        });
}

void DoSamplePopupSystem(const flecs::world& ecs, flecs::timer tickTimer)
{
    // Sample Popup for testing event schedules
    ecs.system<const SamplePopup, const FiredEvent>()
        .kind(flecs::OnUpdate)
        .tick_source(tickTimer)
        .each([](const flecs::entity &e, const SamplePopup& p, const FiredEvent&)
        {
            void(e.add<PausesGame>());

            auto name = "Popup##" + std::to_string(e.id());

            CenterNextImGuiWindow();
            if (ImGui::Begin(name.c_str(), nullptr, ImGuiWindowFlags_NoDocking))
            {
                ImGui::Text("ID %zu", e.id());
                ImGui::Text("%s", p.mMessage.c_str());
                if (ImGui::Button("Close me"))
                {
                    e.destruct();
                }
            }
            ImGui::End();
        });
}

void DoCharacterAgingSystem(const flecs::world& ecs, const GameTickSources& timers)
{
    ecs.system<Character>()
        .tick_source(timers.mYearTimer)
        .each([](flecs::entity e, Character &character)
        {
            if (character.mAgeDays < 360 * 60) return;
            // TODO: add a timeout to death
            if (Random::GetFloat() < 0.2f) void(e.add<Deceased>());
        });

    ecs.system<Character, const GameTime>()
        .tick_source(timers.mDayTimer)
        .each([](flecs::entity e, Character &character, const GameTime &gameTime)
        {
            character.mAgeDays += gameTime.CountDayChanges();
            if (!e.has(AgeClass::Adult) && character.mAgeDays > 16 * 360)
                void(e.add(AgeClass::Adult));
        });
}

void DoGameOverEvents(const flecs::world& ecs, const GameTickSources& timers)
{
    ecs.system<const Character>()
        .entity(ecs.entity<Player>())
        .with<Deceased>()
        .tick_source(timers.mTickTimer)
        .each([](flecs::entity entity, const Character &character)
        {
            void(entity.add<PausesGame>());
            if (ImGui::Begin("Você Morreu"))
            {
                ImGui::Text("Após uma vida, você retornou ao pó, e deixa um reino a seus filhos.");
                ImGui::Text("Vivestes por %zd anos", character.mAgeDays / 360);
                ImGui::Text("Volte ao menu para reiniciar.");
            }
            ImGui::End();
        });

    ecs.system<const EstatePowers>()
        .tick_source(timers.mTickTimer)
        .each([](flecs::iter &it, size_t, const EstatePowers &powers)
        {
            if (powers.mCommonersPower > -100 && powers.mCommonersPower < 100
                && powers.mNobilityPower > -100 && powers.mNobilityPower < 100
                && powers.mClergyPower > -100 && powers.mClergyPower < 100)
                return;

            void(it.world().entity("GameOver").add<PausesGame>());
            if (ImGui::Begin("Você Morreu"))
            {
                ImGui::Text("O reino colapsou devido à má gestão.");
                if (powers.mCommonersPower <= -100)
                    ImGui::Text("A plebe, irada pela vida precária, invade o palácio.");
                if (powers.mCommonersPower >= 100)
                    ImGui::Text("A burguesia resolve livrar-se da família real e instaurar uma república.");
                if (powers.mClergyPower <= -100)
                    ImGui::Text("Devido à perseguição extrema ao clero, sofreu um julgamento divino.");
                if (powers.mClergyPower >= 100)
                    ImGui::Text("Espiritualistas tomam conta do clero e da nação, proclamando que a matéria é má e que ninguém deve obedecer a rei algum.");
                if (powers.mNobilityPower <= -100)
                    ImGui::Text("Os nobres vassalos não mais respeitam os seus contratos, já que você não respeitou sua parte, e o reino já não existe.");
                if (powers.mNobilityPower >= 100)
                    ImGui::Text("O poder que rege no reino já não é mais o seu, o povo respeita e admira mais a nobreza local, e eles decidem dividir o poder e eleger um novo monarca entre si.");
            }
            ImGui::Text("Volte ao menu para reiniciar.");
            ImGui::End();
        });
}

EventsSampleScene::EventsSampleScene(const flecs::world& ecs)
{
    const auto &timers = ecs.get<GameTickSources>();

    DoGameTimeSystems(ecs, timers.mTickTimer);

    DoEventSchedulingSystems(ecs, timers.mTickTimer);

    // DoAdultMarriageSystems(ecs, timers.mTickTimer);

    // DoCharacterBirthSystems(ecs, timers.mTickTimer);

    DoSamplePopupSystem(ecs, timers.mTickTimer);


    DoCharacterAgingSystem(ecs, timers);

    DoEstatePowerSystems(ecs, timers);

    DoGameOverEvents(ecs, timers);

    InitializeEntities(ecs);
}

void EventsSampleScene::InitializeEntities(const flecs::world& ecs)
{}

#include <cmath>
#include <imgui.h>

#include "Events.hpp"
#include "Characters.hpp"
#include "Game.hpp"

#include "EstatePower.hpp"
#include "Components/Dynasty.hpp"
#include "UI/GameOver.hpp"
#include "UI/UIScreens/GameUIModule.hpp"

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
        .each([ecs](flecs::iter &it, size_t i, PregnancySaga &saga, const GameTime &gameTime)
        {
            const auto &entity = it.entity(i);
            auto *father = saga.father.try_get<Character>();
            auto *mother = saga.mother.try_get<Character>();
            if (father == nullptr || mother == nullptr) return entity.destruct();
            auto dynasty = saga.father.target<DynastyMember>();
            auto playerDynasty = ecs.entity<Player>().target<DynastyMember>();

            switch (saga.stage)
            {
            case PregnancySaga::Attempt:
                {
                    saga.NextStage(entity, gameTime);
                }
                break;
            case PregnancySaga::Announce:
                {
                    if (dynasty == playerDynasty)
                    {
                        void(entity.add<PausesGame>());
                        auto name = "Evento - Gravidez##" + std::to_string(entity.id());
                        CenterNextImGuiWindow();
                        if (ImGui::Begin(name.data()))
                        {
                            ImGui::Text("%s e %s, da sua dinastia, vão ter uma criança.", father->mName.data(), mother->mName.data());
                            if (ImGui::Button("Close"))
                            {
                                saga.NextStage(entity, gameTime);
                            }
                        }
                        ImGui::End();
                    } else
                    {
                        saga.NextStage(entity, gameTime);
                    }
                    break;
                }
            case PregnancySaga::Birth:
                {
                    saga.NextStage(entity, gameTime);
                    break;
                }
            case PregnancySaga::BirthAnnounce:
                {
                    if (dynasty == playerDynasty)
                    {
                        void(entity.add<PausesGame>());
                        auto name = "Evento - Nascimento##" + std::to_string(entity.id());
                        auto &child = saga.child.get_mut<Character>();
                        CenterNextImGuiWindow();
                        if (ImGui::Begin(name.data()))
                        {
                            ImGui::Text("O filho de %s e %s nasceu: %s", father->mName.data(), mother->mName.data(), child.mName.data());
                            if (saga.nameBuffer[0] == '\0') strncpy(saga.nameBuffer, child.mName.c_str(), 200);
                            if (ImGui::InputText("Nome", saga.nameBuffer, 256))
                            {
                                child.mName = saga.nameBuffer;
                            }
                            if (ImGui::Button("Close"))
                            {
                                saga.NextStage(entity, gameTime);
                            }
                        }
                        ImGui::End();
                    } else
                    {
                        saga.NextStage(entity, gameTime);
                    }
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
            if (Random::GetFloat() < 0.2f) void(e.set(AgeClass::Deceased));
        });

    ecs.system<Character, const GameTime>()
        .tick_source(timers.mDayTimer)
        .each([](flecs::entity e, Character &character, const GameTime &gameTime)
        {
            character.mAgeDays += gameTime.CountDayChanges();
            if (character.mAgeDays <= 16 * 360)
                void(e.set(AgeClass::Child));
            else if (!e.has(AgeClass::Deceased))
                void(e.set(AgeClass::Adult));
        });
}

void DoGameOverEvents(const flecs::world& ecs, const GameTickSources& timers)
{
    // Sistema para morte do jogador
    ecs.system<const Character>()
        .entity(ecs.entity<Player>())
        .with(AgeClass::Deceased)
        .tick_source(timers.mTickTimer)
        .each([&ecs](flecs::entity entity, const Character &character)
        {
            // Coletar informações do jogador
            flecs::entity playerEntity = ecs.entity<Player>();
            const Character* playerChar = playerEntity.try_get<Character>();

            // Obter título do jogador
            std::string titleName = "Sem título";
            flecs::entity titleEntity = playerEntity.target<RulerOf>();
            if (titleEntity.is_valid()) {
                const Title* title = titleEntity.try_get<Title>();
                if (title) titleName = title->name;
            }

            // Obter dinastia
            std::string dynastyName = "Sem dinastia";
            flecs::entity dynastyEntity = playerEntity.target<DynastyMember>();
            if (dynastyEntity.is_valid()) {
                const Dynasty* dynasty = dynastyEntity.try_get<Dynasty>();
                if (dynasty) dynastyName = dynasty->name;
            }

            // Preparar mensagem de game over
            std::string cause = "Após uma vida, você retornou ao pó, e deixa um reino a seus filhos.\n";
            cause += "Vivestes por " + std::to_string(character.mAgeDays / 360) + " anos.\n";
            cause += "Seu reinado chegou ao fim natural.";

            // Configurar informações para a tela de game over
            GameOverModule::SetGameOverInfo(
                cause,
                playerChar ? playerChar->mName : "",
                playerChar ? playerChar->mAgeDays / 360 : 0,
                titleName,
                dynastyName,
                playerChar ? playerChar->MoneyFloat() : 0.0f
            );

            // Desativar UI do jogo e ativar tela de game over
            auto testUIEntity = ecs.entity<GameUIModule>();
            auto gameOverEntity = ecs.entity<GameOverModule>();

            if (testUIEntity.is_valid()) testUIEntity.disable();
            if (gameOverEntity.is_valid()) gameOverEntity.enable();

            // Pausar o jogo
            void(entity.add<PausesGame>());
        });

    // Sistema para colapso do reino (poder dos estados)
    ecs.system<const EstatePowers>()
        .tick_source(timers.mTickTimer)
        .each([&ecs](flecs::iter &it, size_t, const EstatePowers &powers)
        {
            if (powers.mCommonersPower > -100 && powers.mCommonersPower < 100
                && powers.mNobilityPower > -100 && powers.mNobilityPower < 100
                && powers.mClergyPower > -100 && powers.mClergyPower < 100)
                return;

            // Coletar informações do jogador
            flecs::entity playerEntity = it.world().entity<Player>();
            const Character* playerChar = playerEntity.try_get<Character>();

            // Obter título do jogador
            std::string titleName = "Sem título";
            flecs::entity titleEntity = playerEntity.target<RulerOf>();
            if (titleEntity.is_valid()) {
                const Title* title = titleEntity.try_get<Title>();
                if (title) titleName = title->name;
            }

            // Obter dinastia
            std::string dynastyName = "Sem dinastia";
            flecs::entity dynastyEntity = playerEntity.target<DynastyMember>();
            if (dynastyEntity.is_valid()) {
                const Dynasty* dynasty = dynastyEntity.try_get<Dynasty>();
                if (dynasty) dynastyName = dynasty->name;
            }

            // Preparar mensagem de game over
            std::string cause = GameOverModule::FormatGameOverCause(&powers);

            // Configurar informações para a tela de game over
            GameOverModule::SetGameOverInfo(
                cause,
                playerChar ? playerChar->mName : "",
                playerChar ? playerChar->mAgeDays / 360 : 0,
                titleName,
                dynastyName,
                playerChar ? playerChar->MoneyFloat() : 0.0f
            );

            // Desativar UI do jogo e ativar tela de game over
            auto testUIEntity = it.world().entity<GameUIModule>();
            auto gameOverEntity = it.world().entity<GameOverModule>();

            if (testUIEntity.is_valid()) testUIEntity.disable();
            if (gameOverEntity.is_valid()) gameOverEntity.enable();

            // Pausar o jogo
            void(it.world().entity("GameOver").add<PausesGame>());
        });
}

EventsModule::EventsModule(const flecs::world& ecs)
{
    const auto &timers = ecs.get<GameTickSources>();

    DoGameTimeSystems(ecs, timers.mTickTimer);

    DoEventSchedulingSystems(ecs, timers.mTickTimer);

    // DoAdultMarriageSystems(ecs, timers.mTickTimer);

    DoCharacterBirthSystems(ecs, timers.mTickTimer);

    DoSamplePopupSystem(ecs, timers.mTickTimer);

    DoCharacterAgingSystem(ecs, timers);

    DoEstatePowerSystems(ecs, timers);

    DoGameOverEvents(ecs, timers);
}

void PregnancySaga::NextStage(flecs::entity entity, const GameTime &gameTime)
{
    switch (stage)
    {
    case Attempt:
        // TODO: make random deterministic
        if (Random::GetFloat() < 0.2f)
        {
            stage = PregnancySaga::Announce;
        }
        void(entity.remove<PausesGame>()
            .remove<FiredEvent>()
            .set<EventSchedule>(EventSchedule::InXDays(gameTime, 30)));
        break;
    case Announce:
        {
            stage = PregnancySaga::Birth;
            // TODO: make random deterministic
            int variation = Random::GetIntRange(30, 60);
            void(entity.remove<PausesGame>()
                .remove<FiredEvent>()
                .set<EventSchedule>(EventSchedule::InXDays(gameTime, 30 * 7 + variation)));
        }
        break;
    case Birth:
        child = BirthChildCharacter(entity.world(), father.get<Character>(), mother.get<Character>(), dynasty);
        stage = PregnancySaga::BirthAnnounce;
        break;
    case BirthAnnounce:
        void(entity.destruct());
        break;
    default: ;
    }
}

#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <vector>

using namespace geode::prelude;

enum class BotState {
    Idle,
    Recording,
    Playing
};

struct ReplayInput {
    int frame;
    int button;
    bool player2;
    bool down;
};

static std::vector<ReplayInput> g_recordedInputs;
static BotState g_state = BotState::Idle;
static int g_currentFrame = 0;
static size_t g_playbackIndex = 0;

class $modify(BotPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        g_currentFrame = 0;
        g_playbackIndex = 0;

        return true;
    }

    void handleButton(bool down, int button, bool isPlayer1) {
        PlayLayer::handleButton(down, button, isPlayer1);
        if (g_state == BotState::Recording) {
            g_recordedInputs.push_back({g_currentFrame, button, !isPlayer1, down});
        }
    }

    void update(float dt) {
        PlayLayer::update(dt);

        if (g_state == BotState::Playing) {
            while (g_playbackIndex < g_recordedInputs.size() &&
                   g_recordedInputs[g_playbackIndex].frame == g_currentFrame) {
                auto& input = g_recordedInputs[g_playbackIndex];
                this->handleButton(input.down, input.button, !input.player2);
                g_playbackIndex++;
            }

            if (g_playbackIndex >= g_recordedInputs.size()) {
                g_state = BotState::Idle;
            }
        }

        g_currentFrame++;
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        g_currentFrame = 0;
        g_playbackIndex = 0;
    }
};

class $modify(BotPauseLayer, PauseLayer) {
    bool init(bool inEditor) {
        if (!PauseLayer::init(inEditor)) return false;

        auto menu = typeinfo_cast<CCMenu*>(this->getChildByID("pause-menu"));
        if (!menu) return true;

        auto recordBtn = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_button_01.png"),
            this,
            menu_selector(BotPauseLayer::onToggleRecord)
        );
        recordBtn->setID("bot-record-button");
        recordBtn->setPosition({-50, 50});
        menu->addChild(recordBtn);

        auto playBtn = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_button_02.png"),
            this,
            menu_selector(BotPauseLayer::onTogglePlay)
        );
        playBtn->setID("bot-play-button");
        playBtn->setPosition({50, 50});
        menu->addChild(playBtn);

        return true;
    }

    void onToggleRecord(CCObject*) {
        if (g_state == BotState::Recording) {
            g_state = BotState::Idle;
        } else {
            g_recordedInputs.clear();
            g_state = BotState::Recording;
        }
        Notification::create(
            g_state == BotState::Recording ? "Recording started" : "Recording stopped",
            NotificationIcon::Info
        )->show();
    }

    void onTogglePlay(CCObject*) {
        if (g_state == BotState::Playing) {
            g_state = BotState::Idle;
        } else if (!g_recordedInputs.empty()) {
            g_playbackIndex = 0;
            g_state = BotState::Playing;
        }
        Notification::create(
            g_state == BotState::Playing ? "Playback started" : "Playback stopped",
            NotificationIcon::Info
        )->show();
    }
};

#include <Geode/modify/PlayLayer.hpp>
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
static PauseLayer* g_lastPauseLayer = nullptr;

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

class BotButtonHandler : public CCObject {
public:
    void onToggleRecord(CCObject* sender) { ::onToggleRecord(sender); }
    void onTogglePlay(CCObject* sender) { ::onTogglePlay(sender); }
};

static BotButtonHandler* g_handler = nullptr;

void tryAddPauseButtons() {
    auto scene = CCDirector::sharedDirector()->getRunningScene();
    if (!scene) return;

    PauseLayer* pause = nullptr;
    CCArray* children = scene->getChildren();
    if (children) {
        for (int i = 0; i < children->count(); i++) {
            auto node = children->objectAtIndex(i);
            if (auto pl = typeinfo_cast<PauseLayer*>(node)) {
                pause = pl;
                break;
            }
        }
    }

    if (!pause) {
        g_lastPauseLayer = nullptr;
        return;
    }

    if (pause == g_lastPauseLayer) return;
    g_lastPauseLayer = pause;

    auto menu = typeinfo_cast<CCMenu*>(pause->getChildByID("pause-menu"));
    if (!menu) return;

    if (!g_handler) g_handler = new BotButtonHandler();

    auto recordBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_button_01.png"),
        g_handler,
        menu_selector(BotButtonHandler::onToggleRecord)
    );
    recordBtn->setID("bot-record-button");
    recordBtn->setPosition({-50, 50});
    menu->addChild(recordBtn);

    auto playBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_button_02.png"),
        g_handler,
        menu_selector(BotButtonHandler::onTogglePlay)
    );
    playBtn->setID("bot-play-button");
    playBtn->setPosition({50, 50});
    menu->addChild(playBtn);
}

class $modify(BotPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        g_currentFrame = 0;
        g_playbackIndex = 0;
        g_lastPauseLayer = nullptr;

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

        tryAddPauseButtons();

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

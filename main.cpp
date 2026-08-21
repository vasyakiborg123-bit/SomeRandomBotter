#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <vector>
using namespace geode::prelude;

struct ReplayInput {
    int frame;
    bool hold;
    bool isPlayer2;
};

static std::vector<ReplayInput> g_recordedInputs;
static bool g_isRecording = false;
static bool g_isPlaying = false;
static int g_currentFrame = 0;
static size_t g_playbackIndex = 0;

class $modify(BotPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        g_currentFrame = 0;
        g_playbackIndex = 0;
        return true;
    }

    void pushButton(PlayerButton button, bool isPlayer1) {
        PlayLayer::pushButton(button, isPlayer1);
        if (g_isRecording) {
            g_recordedInputs.push_back({g_currentFrame, true, !isPlayer1});
        }
    }

    void releaseButton(PlayerButton button, bool isPlayer1) {
        PlayLayer::releaseButton(button, isPlayer1);
        if (g_isRecording) {
            g_recordedInputs.push_back({g_currentFrame, false, !isPlayer1});
        }
    }

    void update(float dt) {
        PlayLayer::update(dt);

        if (g_isPlaying) {
            while (g_playbackIndex < g_recordedInputs.size() &&
                   g_recordedInputs[g_playbackIndex].frame == g_currentFrame) {
                auto& input = g_recordedInputs[g_playbackIndex];
                auto player = input.isPlayer2 ? m_player2 : m_player1;
                if (input.hold) player->pushButton(PlayerButton::Jump);
                else player->releaseButton(PlayerButton::Jump);
                g_playbackIndex++;
            }
        }

        g_currentFrame++;
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        g_currentFrame = 0;
        g_playbackIndex = 0;
        if (g_isRecording) g_recordedInputs.clear();
    }
};

class $modify(BotPauseLayer, PauseLayer) {
    bool init(bool level) {
        if (!PauseLayer::init(level)) return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        this->addChild(menu, 100);

        auto recordLabel = CCLabelBMFont::create("Record", "bigFont.fnt");
        recordLabel->setScale(0.5f);
        auto recordBtn = CCMenuItemSpriteExtra::create(
            recordLabel, this, menu_selector(BotPauseLayer::onRecord)
        );
        recordBtn->setPosition({winSize.width / 2 - 100, 50});
        menu->addChild(recordBtn);

        auto playLabel = CCLabelBMFont::create("Play", "bigFont.fnt");
        playLabel->setScale(0.5f);
        auto playBtn = CCMenuItemSpriteExtra::create(
            playLabel, this, menu_selector(BotPauseLayer::onPlay)
        );
        playBtn->setPosition({winSize.width / 2 + 100, 50});
        menu->addChild(playBtn);

        this->updateLabels();

        return true;
    }

    void updateLabels() {
        // визуальная обратная связь через уведомления
    }

    void onRecord(CCObject* sender) {
        g_isRecording = !g_isRecording;
        g_isPlaying = false;
        if (g_isRecording) {
            g_recordedInputs.clear();
            Notification::create("Recording started", NotificationIcon::Success)->show();
        } else {
            Notification::create(
                fmt::format("Recording stopped ({} inputs)", g_recordedInputs.size()),
                NotificationIcon::Success
            )->show();
        }
    }

    void onPlay(CCObject* sender) {
        if (g_recordedInputs.empty()) {
            Notification::create("No replay recorded!", NotificationIcon::Error)->show();
            return;
        }
        g_isPlaying = !g_isPlaying;
        g_isRecording = false;
        g_playbackIndex = 0;
        Notification::create(
            g_isPlaying ? "Playback started" : "Playback stopped",
            NotificationIcon::Success
        )->show();
    }
};

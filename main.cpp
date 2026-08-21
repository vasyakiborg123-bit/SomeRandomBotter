#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <vector>
using namespace geode::prelude;

struct ReplayInput {
    int frame;
    bool hold;
    bool isPlayer2;
    bool player1Button; // true = jump/click, можно расширить под другие кнопки
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

        // ВРЕМЕННО: включаем запись сразу при старте уровня для теста
        g_isRecording = true;
        g_recordedInputs.clear();

        return true;
    }

    void pushButton(PlayerButton button, bool isPlayer1) {
        PlayLayer::pushButton(button, isPlayer1);
        if (g_isRecording) {
            g_recordedInputs.push_back({g_currentFrame, true, !isPlayer1, true});
        }
    }

    void releaseButton(PlayerButton button, bool isPlayer1) {
        PlayLayer::releaseButton(button, isPlayer1);
        if (g_isRecording) {
            g_recordedInputs.push_back({g_currentFrame, false, !isPlayer1, true});
        }
    }

    void update(float dt) {
        PlayLayer::update(dt);

        if (g_isPlaying) {
            while (g_playbackIndex < g_recordedInputs.size() &&
                   g_recordedInputs[g_playbackIndex].frame == g_currentFrame) {
                auto& input = g_recordedInputs[g_playbackIndex];
                if (input.hold) {
                    if (input.isPlayer2) m_player2->pushButton(PlayerButton::Jump);
                    else m_player1->pushButton(PlayerButton::Jump);
                } else {
                    if (input.isPlayer2) m_player2->releaseButton(PlayerButton::Jump);
                    else m_player1->releaseButton(PlayerButton::Jump);
                }
                g_playbackIndex++;
            }
        }

        g_currentFrame++;
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        g_currentFrame = 0;
        g_playbackIndex = 0;
        if (g_isRecording) {
            // при рестарте во время записи — начинаем запись заново
            g_recordedInputs.clear();
        }
    }
};

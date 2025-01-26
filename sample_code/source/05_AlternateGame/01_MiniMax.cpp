// Copyright [2022] <Copyright Eita Aoki (Thunder) >
#include <string>
#include <array>
#include <vector>
#include <sstream>
#include <utility>
#include <random>
#include <assert.h>
#include <math.h>
#include <chrono>
#include <algorithm>
#include <iostream>
#include <functional>
#include <queue>
#include <set>
#pragma GCC diagnostic ignored "-Wsign-compare"
std::random_device rnd;
std::mt19937 mt_for_action(0);

constexpr const int H = 3;  // 미로의 높이
constexpr const int W = 3;  // 미로의 너비
constexpr int END_TURN = 4; // 게임 종료 턴

using ScoreType = int64_t;
constexpr const ScoreType INF = 1000000000LL;

enum WinningStatus
{
    WIN,
    LOSE,
    DRAW,
    NONE,
};

class AlternateMazeState
{
private:
    static constexpr const int dx[4] = {1, -1, 0, 0};
    static constexpr const int dy[4] = {0, 0, 1, -1};
    struct Character
    {
        int y_;
        int x_;
        int game_score_;
        Character(const int y = 0, const int x = 0) : y_(y), x_(x), game_score_(0) {}
    };
    std::vector<std::vector<int>> points_; // 바닥의 점수는 1~9 중 하나
    int turn_;                             // 현재 턴
    std::vector<Character> characters_;

public:
    AlternateMazeState(const int seed) : points_(H, std::vector<int>(W)),
                                         turn_(0),
                                         characters_({Character(H / 2, (W / 2) - 1), Character(H / 2, (W / 2) + 1)})
    {
        auto mt_for_construct = std::mt19937(seed);

        for (int y = 0; y < H; y++)
            for (int x = 0; x < W; x++)
            {
                int point = mt_for_construct() % 10;
                if (characters_[0].y_ == y && characters_[0].x_ == x)
                {
                    continue;
                }
                if (characters_[1].y_ == y && characters_[1].x_ == x)
                {
                    continue;
                }

                this->points_[y][x] = point;
            }
    }

    // [모든 게임에서 구현] : 게임 종료 판정
    bool isDone() const
    {
        return this->turn_ == END_TURN;
    }

    // [모든 게임에서 구현] : 지정한 action으로 게임을 1턴 진행하고 다음 플레이어 시점의 게임판으로 만든다.
    void advance(const int action)
    {
        auto &character = this->characters_[0];
        character.x_ += dx[action];
        character.y_ += dy[action];
        auto &point = this->points_[character.y_][character.x_];
        if (point > 0)
        {
            character.game_score_ += point;
            point = 0;
        }
        this->turn_++;
        std::swap(this->characters_[0], this->characters_[1]);
    }

    // [모든 게임에서 구현] : 현재 플레이어가 가능한 행동을 모두 획득한다.
    std::vector<int> legalActions() const
    {
        std::vector<int> actions;
        const auto &character = this->characters_[0];
        for (int action = 0; action < 4; action++)
        {
            int ty = character.y_ + dy[action];
            int tx = character.x_ + dx[action];
            if (ty >= 0 && ty < H && tx >= 0 && tx < W)
            {
                actions.emplace_back(action);
            }
        }
        return actions;
    }

    // [모든 게임에서 구현] : 승패 정보를 획득한다.
    WinningStatus getWinningStatus() const
    {
        if (isDone())
        {
            if (characters_[0].game_score_ > characters_[1].game_score_)
                return WinningStatus::WIN;
            else if (characters_[0].game_score_ < characters_[1].game_score_)
                return WinningStatus::LOSE;
            else
                return WinningStatus::DRAW;
        }
        else
        {
            return WinningStatus::NONE;
        }
    }

    // [모든 게임에서 구현] : 현재 플레이어 시점에서 게임판을 평가한다.
    ScoreType getScore() const
    {
        return characters_[0].game_score_ - characters_[1].game_score_;
    }

    // [필수는 아니지만 구현하면 편리] : 현재 게임 상황을 문자열로 만든다.
    std::string toString() const
    {
        std::stringstream ss("");
        ss << "turn:\t" << this->turn_ << "\n";
        for (int player_id = 0; player_id < this->characters_.size(); player_id++)
        {
            int actual_player_id = player_id;
            if (this->turn_ % 2 == 1)
            {
                actual_player_id = (player_id + 1) % 2; // 짝수 턴은 초기 배치 시점에서 보면 player_id가 반대
            }
            const auto &chara = this->characters_[actual_player_id];
            ss << "score(" << player_id << "):\t" << chara.game_score_ << "\ty: " << chara.y_ << " x: " << chara.x_ << "\n";
        }
        for (int h = 0; h < H; h++)
        {
            for (int w = 0; w < W; w++)
            {
                bool is_written = false; // 해당 좌표에 기록할 문자가 결정되었는지 여부
                for (int player_id = 0; player_id < this->characters_.size(); player_id++)
                {
                    int actual_player_id = player_id;
                    if (this->turn_ % 2 == 1)
                    {
                        actual_player_id = (player_id + 1) % 2; // 짝수 턴은 초기 배치 시점에서 보면 player_id가 반대
                    }

                    const auto &character = this->characters_[player_id];
                    if (character.y_ == h && character.x_ == w)
                    {
                        if (actual_player_id == 0)
                        {
                            ss << 'A';
                        }
                        else
                        {
                            ss << 'B';
                        }
                        is_written = true;
                    }
                }
                if (!is_written)
                {
                    if (this->points_[h][w] > 0)
                    {
                        ss << points_[h][w];
                    }
                    else
                    {
                        ss << '.';
                    }
                }
            }
            ss << '\n';
        }

        return ss.str();
    }
};

using State = AlternateMazeState;

// 무작위로 행동을 결정한다.
int randomAction(const State &state)
{
    auto legal_actions = state.legalActions();
    return legal_actions[mt_for_action() % (legal_actions.size())];
}

namespace minimax
{
    // 미니맥스 알고리즘용 점수 계산
    ScoreType miniMaxScore(const State &state, const int depth)
    {
        if (state.isDone() || depth == 0)
        {
            return state.getScore();
        }
        auto legal_actions = state.legalActions();
        if (legal_actions.empty())
        {
            return state.getScore();
        }
        ScoreType bestScore = -INF;
        for (const auto action : legal_actions)
        {
            State next_state = state;
            next_state.advance(action);
            ScoreType score = -miniMaxScore(next_state, depth - 1);
            if (score > bestScore)
            {
                bestScore = score;
            }
        }
        return bestScore;
    }
    // 깊이를 지정해서 미니맥스 알고리즘으로 행동을 결정한다.
    int miniMaxAction(const State &state, const int depth)
    {
        ScoreType best_action = -1;
        ScoreType best_score = -INF;
        for (const auto action : state.legalActions())
        {
            State next_state = state;
            next_state.advance(action);
            ScoreType score = -miniMaxScore(next_state, depth);
            if (score > best_score)
            {
                best_action = action;
                best_score = score;
            }
        }
        return best_action;
    }
}
using minimax::miniMaxAction;

// 게임을 1회 플레이해서 게임 상황을 표시한다.
void playGame(const int seed)
{
    using std::cout;
    using std::endl;
    auto state = State(seed);
    cout << state.toString() << endl;
    while (!state.isDone())
    {
        // 1p
        {
            cout << "1p ------------------------------------" << endl;
            int action = miniMaxAction(state, /*depth*/ END_TURN);
            cout << "action " << action << endl;
            state.advance(action);
            cout << state.toString() << endl;
            if (state.isDone())
            {

                switch (state.getWinningStatus())
                {
                case (WinningStatus::WIN):
                    cout << "winner: "
                         << "2p" << endl;
                    break;
                case (WinningStatus::LOSE):
                    cout << "winner: "
                         << "1p" << endl;
                    break;
                default:
                    cout << "DRAW" << endl;
                    break;
                }
                break;
            }
        }
        // 2p
        {
            cout << "2p ------------------------------------" << endl;
            int action = randomAction(state);
            cout << "action " << action << endl;
            state.advance(action);
            cout << state.toString() << endl;
            if (state.isDone())
            {

                switch (state.getWinningStatus())
                {
                case (WinningStatus::WIN):
                    cout << "winner: "
                         << "1p" << endl;
                    break;
                case (WinningStatus::LOSE):
                    cout << "winner: "
                         << "2p" << endl;
                    break;
                default:
                    cout << "DRAW" << endl;
                    break;
                }
                break;
            }
        }
    }
}

int main()
{
    using std::cout;
    using std::endl;
    playGame(4121859904);
    return 0;
}
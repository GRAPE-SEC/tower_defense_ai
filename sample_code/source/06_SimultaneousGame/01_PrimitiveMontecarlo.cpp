// Copyright [2022] <Copyright Eita Aoki (Thunder) >
#include <string>
#include <vector>
#include <array>
#include <sstream>
#include <utility>
#include <random>
#include <assert.h>
#include <math.h>
#include <chrono>
#include <queue>
#include <algorithm>
std::random_device rnd;
std::mt19937 mt_for_action(0);

constexpr const int H = 5;   // 미로의 높이
constexpr const int W = 5;   // 미로의 너비
constexpr int END_TURN = 20; // 게임 종료 턴

using ScoreType = int64_t;
constexpr const ScoreType INF = 1000000000LL;

static const std::string dstr[4] = {"RIGHT", "LEFT", "DOWN", "UP"};

enum WinningStatus
{
    FIRST,  // 플레이어 0이 승리
    SECOND, // 플레이어 1이 승리
    DRAW,
    NONE,
};

// 동시 2인 게임 예
// 1턴에 상하좌우 네 방향 중 하나로 벽이 없는 장소로 한 칸 이동한다.
// 바닥에 있는 점수를 차지하면 자신의 점수가 되고, 바닥의 점수는 사라진다.
// END_TURN 시점에 기록 점수가 상대방보다 많으면 승리한다.
class SimultaneousMazeState
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
    SimultaneousMazeState(const int seed) : points_(H, std::vector<int>(W)),
                                            turn_(0),
                                            characters_({Character(H / 2, (W / 2) - 1), Character(H / 2, (W / 2) + 1)})
    {
        auto mt_for_construct = std::mt19937(seed);

        for (int y = 0; y < H; y++)
            for (int x = 0; x < W / 2 + 1; x++)
            {
                int ty = y;
                int tx = x;
                int point = mt_for_construct() % 10;
                if (characters_[0].y_ == y && characters_[0].x_ == x)
                {
                    continue;
                }
                if (characters_[1].y_ == y && characters_[1].x_ == x)
                {
                    continue;
                }
                this->points_[ty][tx] = point;
                tx = W - 1 - x;
                this->points_[ty][tx] = point;
            }
    }

    // [모든 게임에서 구현] : 승패 정보를 획득한다.
    WinningStatus getWinningStatus() const
    {
        if (isDone())
        {
            if (characters_[0].game_score_ > characters_[1].game_score_)
                return WinningStatus::FIRST;
            else if (characters_[0].game_score_ < characters_[1].game_score_)
                return WinningStatus::SECOND;
            else
                return WinningStatus::DRAW;
        }
        else
        {
            return WinningStatus::NONE;
        }
    }

    // [모든 게임에서 구현] : 게임 종료 판정
    bool isDone() const
    {
        return this->turn_ == END_TURN;
    }
    // [모든 게임에서 구현] : 지정한 action으로 게임을 1턴 진행한다.
    void advance(const int action0, const int action1)
    {
        {
            auto &character = this->characters_[0];
            const auto &action = action0;
            character.x_ += dx[action];
            character.y_ += dy[action];
            const auto point = this->points_[character.y_][character.x_];
            if (point > 0)
            {
                character.game_score_ += point;
            }
        }
        {
            auto &character = this->characters_[1];
            const auto &action = action1;
            character.x_ += dx[action];
            character.y_ += dy[action];
            const auto point = this->points_[character.y_][character.x_];
            if (point > 0)
            {
                character.game_score_ += point;
            }
        }

        for (const auto &character : this->characters_)
        {
            this->points_[character.y_][character.x_] = 0;
        }
        this->turn_++;
    }

    // [모든 게임에서 구현] : 지정한 플레이어가 가능한 행동을 모두 획득한다.
    std::vector<int> legalActions(const int player_id) const
    {
        std::vector<int> actions;
        const auto &character = this->characters_[player_id];
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

    // [필수는 아니지만 구현하면 편리] : 플레이어 0의 승리 확률을 계산하기 위해서 기록 점수를 계산한다.
    double getFirstPlayerScoreForWinRate() const
    {
        switch (this->getWinningStatus())
        {
        case (WinningStatus::FIRST):
            return 1.;
        case (WinningStatus::SECOND):
            return 0.;
        default:
            return 0.5;
        }
    }

    // [필수는 아니지만 구현하면 편리] : 현재 게임 상황을 문자열로 만든다.
    std::string toString() const
    {
        std::stringstream ss("");
        ss << "turn:\t" << this->turn_ << "\n";
        for (int player_id = 0; player_id < this->characters_.size(); player_id++)
        {
            ss << "score(" << player_id << "):\t" << this->characters_[player_id].game_score_ << "\n";
        }
        for (int h = 0; h < H; h++)
        {
            for (int w = 0; w < W; w++)
            {
                bool is_written = false; // 해당 좌표에 기록할 문자가 결정되었는지 여부
                for (int player_id = 0; player_id < this->characters_.size(); player_id++)
                {
                    const auto &character = this->characters_[player_id];
                    if (character.y_ == h && character.x_ == w)
                    {
                        if (player_id == 0)
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
using State = SimultaneousMazeState;

// 지정한 플레이어 행동을 무작위로 결정한다.
int randomAction(const State &state, const int player_id)
{
    auto legal_actions = state.legalActions(player_id);
    return legal_actions[mt_for_action() % (legal_actions.size())];
}

namespace montecarlo
{
    // 플레이어 0 시점에서 평가
    double playout(State *state)
    { // const&를 사용하면 재귀중에 깊은 복사가 필요하므로, 속도를 위해 포인터를 사용한다.(const가 아닌 참조도 가능)
        switch (state->getWinningStatus())
        {
        case (WinningStatus::FIRST):
            return 1.;
        case (WinningStatus::SECOND):
            return 0.;
        case (WinningStatus::DRAW):
            return 0.5;
        default:
            state->advance(randomAction(*state, 0), randomAction(*state, 1));
            return playout(state);
        }
    }
    // 플레이아웃 횟수를 지정해서 순수 몬테카를로 탐색으로 지정한 플레이어의 행동을 결정한다.
    int primitiveMontecarloAction(const State &state, const int player_id, const int playout_number)
    {
        auto my_legal_actions = state.legalActions(player_id);
        auto opp_legal_actions = state.legalActions((player_id + 1) % 2);
        double best_value = -INF;
        int best_action_index = -1;
        for (int i = 0; i < my_legal_actions.size(); i++)
        {
            double value = 0;
            for (int j = 0; j < playout_number; j++)
            {
                State next_state = state;
                if (player_id == 0)
                {
                    next_state.advance(my_legal_actions[i], opp_legal_actions[mt_for_action() % opp_legal_actions.size()]);
                }
                else
                {
                    next_state.advance(opp_legal_actions[mt_for_action() % opp_legal_actions.size()], my_legal_actions[i]);
                }
                double player0_win_rate = playout(&next_state);
                double win_rate = (player_id == 0 ? player0_win_rate : 1. - player0_win_rate);
                value += win_rate;
            }
            if (value > best_value)
            {
                best_action_index = i;
                best_value = value;
            }
        }
        return my_legal_actions[best_action_index];
    }
};
using ::montecarlo::primitiveMontecarloAction;

#include <iostream>
#include <functional>

using AIFunction = std::function<int(const State &, const int)>;
using StringAIPair = std::pair<std::string, AIFunction>;

// 게임을 game_number번 플레이해서 ais의 0번째 AI 승률을 표시한다.
void testFirstPlayerWinRate(std::array<StringAIPair, 2> &ais, const int game_number)
{
    using std::cout;
    using std::endl;
    std::mt19937 mt_for_construct(0);

    double first_player_win_rate = 0;
    for (int i = 0; i < game_number; i++)
    {
        auto state = State(mt_for_construct());
        auto &first_ai = ais[0];
        auto &second_ai = ais[1];
        while (true)
        {
            state.advance(first_ai.second(state, 0), second_ai.second(state, 1));
            if (state.isDone())
                break;
        }
        double win_rate_point = state.getFirstPlayerScoreForWinRate();
        if (win_rate_point >= 0)
        {
            state.toString();
        }
        first_player_win_rate += win_rate_point;

        cout << "i " << i << " w " << first_player_win_rate / (i + 1) << endl;
    }
    first_player_win_rate /= (double)game_number;
    cout << "Winning rate of " << ais[0].first << " to " << ais[1].first << ":\t" << first_player_win_rate << endl;
}

int main()
{
    auto ais = std::array<StringAIPair, 2>{

        StringAIPair("primitiveMontecarloAction", [](const State &state, const int player_id)
                     { return primitiveMontecarloAction(state, player_id, 1000); }),
        StringAIPair("randomAction", [](const State &state, const int player_id)
                     { return randomAction(state, player_id); }),
    };

    testFirstPlayerWinRate(ais, 500);

    return 0;
}
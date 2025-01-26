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

constexpr const int H = 3;   // 미로의 높이
constexpr const int W = 3;   // 미로의 너비
constexpr int END_TURN = 10; // 게임 종료 턴

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

    // 현재 플레이어가 선공인지 판정한다.
    bool isFirstPlayer() const
    {
        return this->turn_ % 2 == 0;
    }

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

    // [필수는 아니지만 구현하면 편리] : 선공 플레이어의 승률 계산하기 위해서 승점을 계산한다.
    double getFirstPlayerScoreForWinRate() const
    {
        switch (this->getWinningStatus())
        {
        case (WinningStatus::WIN):
            if (this->isFirstPlayer())
            {
                return 1.;
            }
            else
            {
                return 0.;
            }
        case (WinningStatus::LOSE):
            if (this->isFirstPlayer())
            {
                return 0.;
            }
            else
            {
                return 1.;
            }
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
    // 미니맥스 알고리즘용 기록 점수 계산
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

namespace alphabeta
{
    // 알파-베타 가지치기용 점수 계산
    ScoreType alphaBetaScore(const State &state, ScoreType alpha, const ScoreType beta, const int depth)
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
        for (const auto action : legal_actions)
        {
            State next_state = state;
            next_state.advance(action);
            ScoreType score = -alphaBetaScore(next_state, -beta, -alpha, depth - 1);
            if (score > alpha)
            {
                alpha = score;
            }
            if (alpha >= beta)
            {
                return alpha;
            }
        }
        return alpha;
    }
    // 깊이를 지정해서 알파-베타 가지치기로 행동을 결정한다.
    int alphaBetaAction(const State &state, const int depth)
    {
        ScoreType best_action = -1;
        ScoreType alpha = -INF;
        ScoreType beta = INF;
        for (const auto action : state.legalActions())
        {
            State next_state = state;
            next_state.advance(action);
            ScoreType score = -alphaBetaScore(next_state, -beta, -alpha, depth);
            if (score > alpha)
            {
                best_action = action;
                alpha = score;
            }
        }
        return best_action;
    }
}
using alphabeta::alphaBetaAction;

using AIFunction = std::function<int(const State &)>;
using StringAIPair = std::pair<std::string, AIFunction>;

// 게임을 game_number×2(선공과 후공을 교대)횟수만큼 플레이해서 ais의 0번째에 있는 AI 승률을 표시한다.
void testFirstPlayerWinRate(const std::array<StringAIPair, 2> &ais, const int game_number)
{
    using std::cout;
    using std::endl;

    double first_player_win_rate = 0;
    for (int i = 0; i < game_number; i++)
    {
        auto base_state = State(i);
        for (int j = 0; j < 2; j++)
        { // 공평하게 선공과 후공을 교대함
            auto state = base_state;
            auto &first_ai = ais[j];
            auto &second_ai = ais[(j + 1) % 2];
            while (true)
            {
                state.advance(first_ai.second(state));
                if (state.isDone())
                    break;
                state.advance(second_ai.second(state));
                if (state.isDone())
                    break;
            }
            double win_rate_point = state.getFirstPlayerScoreForWinRate();
            if (j == 1)
                win_rate_point = 1 - win_rate_point;
            if (win_rate_point >= 0)
            {
                state.toString();
            }
            first_player_win_rate += win_rate_point;
        }
        cout << "i " << i << " w " << first_player_win_rate / ((i + 1) * 2) << endl;
    }
    first_player_win_rate /= (double)(game_number * 2);
    cout << "Winning rate of " << ais[0].first << " to " << ais[1].first << ":\t" << first_player_win_rate << endl;
}

std::vector<State> getSampleStates(const int game_number)
{
    std::mt19937 mt_for_construct(0);
    std::vector<State> states;
    for (int i = 0; i < game_number; i++)
    {
        auto state = State(mt_for_construct());
        int turn = mt_for_construct() % END_TURN;
        for (int t = 0; t < turn; t++)
        {
            state.advance(randomAction(state));
        }
        states.emplace_back(state);
    }
    return states;
}

void calculateExecutionSpeed(const StringAIPair &ai, const std::vector<State> &states)
{
    using std::cout;
    using std::endl;
    auto start_time = std::chrono::high_resolution_clock::now();
    for (const auto &state : states)
    {
        ai.second(state);
    }
    auto diff = std::chrono::high_resolution_clock::now() - start_time;
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
    cout << ai.first << " take " << time << " ms to process " << states.size() << " nodes" << endl;
}

int main()
{
    using std::cout;
    using std::endl;
    auto states = getSampleStates(100);
    calculateExecutionSpeed(
        StringAIPair("alphaBetaAction", [](const State &state)
                     { return alphaBetaAction(state, END_TURN); }),
        states);
    calculateExecutionSpeed(
        StringAIPair("miniMaxAction", [](const State &state)
                     { return miniMaxAction(state, END_TURN); }),
        states);
    return 0;
}
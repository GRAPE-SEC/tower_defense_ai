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
public:
    // AlternateMazeState 생성자에서 참조하기 위해서 모든 멤버를 public으로 만듬
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

class AlternateMazeState
{
private:
    static constexpr const int END_TURN_ = END_TURN * 2; // 동시에 두는 게임의 1턴은 교대로 두는 게임의 2턴 분량
    static constexpr const int dx[4] = {1, -1, 0, 0};
    static constexpr const int dy[4] = {0, 0, 1, -1};
    std::vector<std::vector<int>> points_; // 바닥의 점수는 1~9 중 하나
    int turn_;                             // 현재 턴
    using Character = SimultaneousMazeState::Character;
    std::vector<Character> characters_;

public:
    AlternateMazeState(const SimultaneousMazeState &base_state, const int player_id) : points_(base_state.points_),
                                                                                       turn_(base_state.turn_ * 2), // 동시에 두는 게임의 1턴은 교대로 두는 게임의 2턴 분량
                                                                                       characters_(player_id == 0 ? base_state.characters_ : std::vector<Character>{base_state.characters_[1], base_state.characters_[0]})
    {
    }

    // [모든 게임에서 구현] : 승패 정보를 획득한다.
    WinningStatus getWinningStatus() const
    {
        if (isDone())
        {
            if (characters_[0].game_score_ > characters_[1].game_score_)
                return WinningStatus::FIRST; // WIN
            else if (characters_[0].game_score_ < characters_[1].game_score_)
                return WinningStatus::SECOND; // LOSE
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
        return this->turn_ == END_TURN_;
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
        constexpr const int player_id = 0;
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
};
using AlternateState = AlternateMazeState;

namespace altanate_motecalo
{
    // 무작위로 행동을 결정한다.
    int randomAction(const AlternateState &state)
    {
        auto legal_actions = state.legalActions();
        return legal_actions[mt_for_action() % (legal_actions.size())];
    }
    // 무작위로 플레이아웃해서 승패 점수를 계산한다.
    double playout(AlternateState *state)
    { // const&를 사용하면 재귀중에 깊은 복사가 필요하므로, 속도를 위해 포인터를 사용한다.(const가 아닌 참조도 가능)
        switch (state->getWinningStatus())
        {
        case (WinningStatus::FIRST): // WIN
            return 1.;
        case (WinningStatus::SECOND): // LOSE
            return 0.;
        case (WinningStatus::DRAW):
            return 0.5;
        default:
            state->advance(randomAction(*state));
            return 1. - playout(state);
        }
    }
    constexpr const double C = 1.;             // UCB1 계산에 사용하는 상수
    constexpr const int EXPAND_THRESHOLD = 10; // 노드를 확장하는 임계치

    // MCTS 계산에 사용하는 노드
    class Node
    {
    private:
        AlternateState state_;
        double w_;

    public:
        std::vector<Node> child_nodes;
        double n_;

        // 노드를 평가한다.
        double evaluate()
        {
            if (this->state_.isDone())
            {
                double value = 0.5;
                switch (this->state_.getWinningStatus())
                {
                case (WinningStatus::FIRST):
                    value = 1.;
                    break;
                case (WinningStatus::SECOND):
                    value = 0.;
                    break;
                default:
                    break;
                }
                this->w_ += value;
                ++this->n_;
                return value;
            }
            if (this->child_nodes.empty())
            {
                AlternateState state_copy = this->state_;
                double value = playout(&state_copy);
                this->w_ += value;
                ++this->n_;

                if (this->n_ == EXPAND_THRESHOLD)
                    this->expand();

                return value;
            }
            else
            {
                double value = 1. - this->nextChildNode().evaluate();
                this->w_ += value;
                ++this->n_;
                return value;
            }
        }

        // 노드를 확장한다.
        void expand()
        {
            auto legal_actions = this->state_.legalActions();
            this->child_nodes.clear();
            for (const auto action : legal_actions)
            {
                this->child_nodes.emplace_back(this->state_);
                this->child_nodes.back().state_.advance(action);
            }
        }

        // 어떤 노드를 평가할지 선택한다.
        Node &nextChildNode()
        {
            for (auto &child_node : this->child_nodes)
            {
                if (child_node.n_ == 0)
                    return child_node;
            }
            double t = 0;
            for (const auto &child_node : this->child_nodes)
            {
                t += child_node.n_;
            }
            double best_value = -INF;
            int best_action_index = -1;
            for (int i = 0; i < this->child_nodes.size(); i++)
            {
                const auto &child_node = this->child_nodes[i];
                double ucb1_value = 1. - child_node.w_ / child_node.n_ + (double)C * std::sqrt(2. * std::log(t) / child_node.n_);
                if (ucb1_value > best_value)
                {
                    best_action_index = i;
                    best_value = ucb1_value;
                }
            }
            return this->child_nodes[best_action_index];
        }

        Node(const AlternateState &state) : state_(state), w_(0), n_(0) {}
    };

    // 플레이아웃 횟수를 지정해서 MCTS로 행동을 결정한다.
    int mctsAction(const State &base_state, const int player_id, const int playout_number)
    {
        auto state = AlternateState(base_state, player_id);
        Node root_node = Node(state);
        root_node.expand();
        for (int i = 0; i < playout_number; i++)
        {
            root_node.evaluate();
        }
        auto legal_actions = state.legalActions();

        int best_action_searched_number = -1;
        int best_action_index = -1;
        assert(legal_actions.size() == root_node.child_nodes.size());
        for (int i = 0; i < legal_actions.size(); i++)
        {
            int n = root_node.child_nodes[i].n_;
            if (n > best_action_searched_number)
            {
                best_action_index = i;
                best_action_searched_number = n;
            }
        }
        return legal_actions[best_action_index];
    }

}
using altanate_motecalo::mctsAction;

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
        StringAIPair("mctsAction", [](const State &state, const int player_id)
                     { return mctsAction(state, player_id, 1000); }),
        StringAIPair("primitiveMontecarloAction", [](const State &state, const int player_id)
                     { return primitiveMontecarloAction(state, player_id, 1000); }),
    };

    testFirstPlayerWinRate(ais, 500);

    return 0;
}
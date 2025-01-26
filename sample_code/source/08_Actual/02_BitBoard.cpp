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

// 시간을 관리하는 클래스
class TimeKeeper
{
private:
    std::chrono::high_resolution_clock::time_point start_time_;
    int64_t time_threshold_;

public:
    // 시간 제한을 밀리초 단위로 지정해서 인스턴스를 생성한다.
    TimeKeeper(const int64_t &time_threshold)
        : start_time_(std::chrono::high_resolution_clock::now()),
          time_threshold_(time_threshold)
    {
    }

    // 인스턴스를 생성한 시점부터 지정한 시간 제한을 초과하지 않았는지 판정한다.
    bool isTimeOver() const
    {
        auto diff = std::chrono::high_resolution_clock::now() - this->start_time_;
        return std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() >= time_threshold_;
    }
};

constexpr const int H = 6; // 미로의 높이
constexpr const int W = 7; // 미로의 너비

using ScoreType = int64_t;
constexpr const ScoreType INF = 1000000000LL;

enum WinningStatus
{
    WIN,
    LOSE,
    DRAW,
    NONE,
};

class ConnectFourState
{
private:
    static constexpr const int dx[2] = {1, -1};          // 이동 방향의 x성분
    static constexpr const int dy_right_up[2] = {1, -1}; // /"／"대각선 방향의 x성분
    static constexpr const int dy_left_up[2] = {-1, 1};  // "\"대각선 방향의 x성분
    static constexpr const int dy[4] = {0, 0, 1, -1};    // 오른쪽, 왼쪽, 위쪽, 아래쪽 이동 방향의 y성분

    WinningStatus winning_status_ = WinningStatus::NONE;

public:
    bool is_first_ = true; // 선공 여부
    int my_board_[H][W] = {};
    int enemy_board_[H][W] = {};

    ConnectFourState()
    {
    }

    // [모든 게임에서 구현] : 게임 종료 판정
    bool isDone() const
    {
        return winning_status_ != WinningStatus::NONE;
    }

    // [모든 게임에서 구현] : 지정한 action으로 게임을 1턴 진행하고 다음 플레이어 시점의 게임판으로 만든다.
    void advance(const int action)
    {
        std::pair<int, int> coordinate;
        for (int y = 0; y < H; y++)
        {
            if (this->my_board_[y][action] == 0 && this->enemy_board_[y][action] == 0)
            {
                this->my_board_[y][action] = 1;
                coordinate = std::pair<int, int>(y, action);
                break;
            }
        }

        { // 가로 방향으로 연속인가 판정한다.

            auto que = std::deque<std::pair<int, int>>();
            que.emplace_back(coordinate);
            std::vector<std::vector<bool>> check(H, std::vector<bool>(W, false));
            int count = 0;
            while (!que.empty())
            {
                const auto &tmp_cod = que.front();
                que.pop_front();
                ++count;
                if (count >= 4)
                {
                    this->winning_status_ = WinningStatus::LOSE; // 자신의 돌이 연속이면 상대방의 패배
                    break;
                }
                check[tmp_cod.first][tmp_cod.second] = true;

                for (int action = 0; action < 2; action++)
                {
                    int ty = tmp_cod.first;
                    int tx = tmp_cod.second + dx[action];

                    if (ty >= 0 && ty < H && tx >= 0 && tx < W && my_board_[ty][tx] == 1 && !check[ty][tx])
                    {
                        que.emplace_back(ty, tx);
                    }
                }
            }
        }
        if (!isDone())
        { // "／"방향으로 연속인가 판정한다.
            auto que = std::deque<std::pair<int, int>>();
            que.emplace_back(coordinate);
            std::vector<std::vector<bool>> check(H, std::vector<bool>(W, false));
            int count = 0;
            while (!que.empty())
            {
                const auto &tmp_cod = que.front();
                que.pop_front();
                ++count;
                if (count >= 4)
                {
                    this->winning_status_ = WinningStatus::LOSE; // 자신의 돌이 연속이면 상대방의 패배
                    break;
                }
                check[tmp_cod.first][tmp_cod.second] = true;

                for (int action = 0; action < 2; action++)
                {
                    int ty = tmp_cod.first + dy_right_up[action];
                    int tx = tmp_cod.second + dx[action];

                    if (ty >= 0 && ty < H && tx >= 0 && tx < W && my_board_[ty][tx] == 1 && !check[ty][tx])
                    {
                        que.emplace_back(ty, tx);
                    }
                }
            }
        }

        if (!isDone())
        { // "\"방향으로 연속인가 판정한다.

            auto que = std::deque<std::pair<int, int>>();
            que.emplace_back(coordinate);
            std::vector<std::vector<bool>> check(H, std::vector<bool>(W, false));
            int count = 0;
            while (!que.empty())
            {
                const auto &tmp_cod = que.front();
                que.pop_front();
                ++count;
                if (count >= 4)
                {
                    this->winning_status_ = WinningStatus::LOSE; // 자신의 돌이 연속이면 상대방의 패배
                    break;
                }
                check[tmp_cod.first][tmp_cod.second] = true;

                for (int action = 0; action < 2; action++)
                {
                    int ty = tmp_cod.first + dy_left_up[action];
                    int tx = tmp_cod.second + dx[action];

                    if (ty >= 0 && ty < H && tx >= 0 && tx < W && my_board_[ty][tx] == 1 && !check[ty][tx])
                    {
                        que.emplace_back(ty, tx);
                    }
                }
            }
        }
        if (!isDone())
        { // 세로 방향으로 연속인가 판정한다.

            int ty = coordinate.first;
            int tx = coordinate.second;
            bool is_win = true;
            for (int i = 0; i < 4; i++)
            {
                bool is_mine = (ty >= 0 && ty < H && tx >= 0 && tx < W && my_board_[ty][tx] == 1);

                if (!is_mine)
                {
                    is_win = false;
                    break;
                }
                --ty;
            }
            if (is_win)
            {
                this->winning_status_ = WinningStatus::LOSE; // 자신의 돌이 연속이면 상대방의 패배
            }
        }

        std::swap(my_board_, enemy_board_);
        is_first_ = !is_first_;
        if (this->winning_status_ == WinningStatus::NONE && legalActions().size() == 0)
        {
            this->winning_status_ = WinningStatus::DRAW;
        }
    }

    // [모든 게임에서 구현] : 현재 플레이어가 가능한 행동을 모두 획득한다.
    std::vector<int> legalActions() const
    {
        std::vector<int> actions;
        for (int x = 0; x < W; x++)
            for (int y = H - 1; y >= 0; y--)
            {
                if (my_board_[y][x] == 0 && enemy_board_[y][x] == 0)
                {
                    actions.emplace_back(x);
                    break;
                }
            }
        return actions;
    }

    // [모든 게임에서 구현] : 승패 정보를 획득한다.
    WinningStatus getWinningStatus() const
    {
        return this->winning_status_;
    }

    // [필수는 아니지만 구현하면 편리] : 선공 플레이어의 승률 계산하기 위해서 승점을 계산한다.
    double getFirstPlayerScoreForWinRate() const
    {
        switch (this->getWinningStatus())
        {
        case (WinningStatus::WIN):
            if (this->is_first_)
            {
                return 1.;
            }
            else
            {
                return 0.;
            }
        case (WinningStatus::LOSE):
            if (this->is_first_)
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

        ss << "is_first:\t" << this->is_first_ << "\n";
        for (int y = H - 1; y >= 0; y--)
        {
            for (int x = 0; x < W; x++)
            {
                char c = '.';
                if (my_board_[y][x] == 1)
                {
                    c = (is_first_ ? 'x' : 'o');
                }
                else if (enemy_board_[y][x] == 1)
                {
                    c = (is_first_ ? 'o' : 'x');
                }
                ss << c;
            }
            ss << "\n";
        }

        return ss.str();
    }
};

class ConnectFourStateByBitSet
{
private:
    uint64_t my_board_ = 0ULL;
    uint64_t all_board_ = 0uLL;
    bool is_first_ = true; // 선공 여부
    WinningStatus winning_status_ = WinningStatus::NONE;

    bool isWinner(const uint64_t board)
    {
        // 가로 방향으로 연속인가 판정한다.
        uint64_t tmp_board = board & (board >> 7);
        if ((tmp_board & (tmp_board >> 14)) != 0)
        {
            return true;
        }
        // "\"방향으로 연속인가 판정한다.
        tmp_board = board & (board >> 6);
        if ((tmp_board & (tmp_board >> 12)) != 0)
        {
            return true;
        }
        // "／"방향으로 연속인가 판정한다.
        tmp_board = board & (board >> 8);
        if ((tmp_board & (tmp_board >> 16)) != 0)
        {
            return true;
        }
        // 세로 방향으로 연속인가 판정한다.
        tmp_board = board & (board >> 1);
        if ((tmp_board & (tmp_board >> 2)) != 0)
        {
            return true;
        }

        return false;
    }

public:
    ConnectFourStateByBitSet() {}
    ConnectFourStateByBitSet(const ConnectFourState &state) : is_first_(state.is_first_)
    {

        my_board_ = 0ULL;
        all_board_ = 0uLL;
        for (int y = 0; y < H; y++)
        {
            for (int x = 0; x < W; x++)
            {
                int index = x * (H + 1) + y;
                if (state.my_board_[y][x] == 1)
                {
                    this->my_board_ |= 1ULL << index;
                }
                if (state.my_board_[y][x] == 1 || state.enemy_board_[y][x] == 1)
                {
                    this->all_board_ |= 1ULL << index;
                }
            }
        }
    }
    bool isDone() const
    {
        return winning_status_ != WinningStatus::NONE;
    }

    void advance(const int action)
    {
        this->my_board_ ^= this->all_board_; // 상대방의 시점으로 바뀐다
        is_first_ = !is_first_;
        uint64_t new_all_board = this->all_board_ | (this->all_board_ + (1ULL << (action * 7)));
        this->all_board_ = new_all_board;
        uint64_t filled = 0b0111111011111101111110111111011111101111110111111ULL;

        if (isWinner(this->my_board_ ^ this->all_board_))
        {
            this->winning_status_ = WinningStatus::LOSE;
        }
        else if (this->all_board_ == filled)
        {
            this->winning_status_ = WinningStatus::DRAW;
        }
    }
    std::vector<int> legalActions() const
    {
        std::vector<int> actions;
        uint64_t possible = this->all_board_ + 0b0000001000000100000010000001000000100000010000001ULL;
        uint64_t filter = 0b0111111;
        for (int x = 0; x < W; x++)
        {
            if ((filter & possible) != 0)
            {
                actions.emplace_back(x);
            }
            filter <<= 7;
        }
        return actions;
    }

    WinningStatus getWinningStatus() const
    {
        return this->winning_status_;
    }

    std::string toString() const
    {
        std::stringstream ss("");
        ss << "is_first:\t" << this->is_first_ << "\n";
        for (int y = H - 1; y >= 0; y--)
        {
            for (int x = 0; x < W; x++)
            {
                int index = x * (H + 1) + y;
                char c = '.';
                if (((my_board_ >> index) & 1ULL) != 0)
                {
                    c = (is_first_ ? 'x' : 'o');
                }
                else if ((((all_board_ ^ my_board_) >> index) & 1ULL) != 0)
                {
                    c = (is_first_ ? 'o' : 'x');
                }
                ss << c;
            }
            ss << "\n";
        }

        return ss.str();
    }
};

using State = ConnectFourState;

using AIFunction = std::function<int(const State &)>;
using StringAIPair = std::pair<std::string, AIFunction>;

// 무작위로 행동을 결정한다.
int randomAction(const State &state)
{
    auto legal_actions = state.legalActions();
    return legal_actions[mt_for_action() % (legal_actions.size())];
}

namespace montecarlo
{
    // 무작위로 플레이아웃해서 승패 점수를 계산한다.
    double playout(State *state)
    { // const&를 사용하면 재귀중에 깊은 복사가 필요하므로, 속도를 위해 포인터를 사용한다.(const가 아닌 참조도 가능)
        switch (state->getWinningStatus())
        {
        case (WinningStatus::WIN):
            return 1.;
        case (WinningStatus::LOSE):
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
        State state_;
        double w_;

    public:
        std::vector<Node> child_nodes_;
        double n_;

        Node(const State &state) : state_(state), w_(0), n_(0) {}

        // 노드를 평가한다.
        double evaluate()
        {
            if (this->state_.isDone())
            {
                double value = 0.5;
                switch (this->state_.getWinningStatus())
                {
                case (WinningStatus::WIN):
                    value = 1.;
                    break;
                case (WinningStatus::LOSE):
                    value = 0.;
                    break;
                default:
                    break;
                }
                this->w_ += value;
                ++this->n_;
                return value;
            }
            if (this->child_nodes_.empty())
            {
                State state_copy = this->state_;
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
            this->child_nodes_.clear();
            for (const auto action : legal_actions)
            {
                this->child_nodes_.emplace_back(this->state_);
                this->child_nodes_.back().state_.advance(action);
            }
        }

        // 어떤 노드를 평가할지 선택한다.
        Node &nextChildNode()
        {
            for (auto &child_node : this->child_nodes_)
            {
                if (child_node.n_ == 0)
                    return child_node;
            }
            double t = 0;
            for (const auto &child_node : this->child_nodes_)
            {
                t += child_node.n_;
            }
            double best_value = -INF;
            int best_action_index = -1;
            for (int i = 0; i < this->child_nodes_.size(); i++)
            {
                const auto &child_node = this->child_nodes_[i];
                double ucb1_value = 1. - child_node.w_ / child_node.n_ + (double)C * std::sqrt(2. * std::log(t) / child_node.n_);
                if (ucb1_value > best_value)
                {
                    best_action_index = i;
                    best_value = ucb1_value;
                }
            }
            return this->child_nodes_[best_action_index];
        }
    };

    // 제한 시간(밀리초)을 지정해서 MCTS로 행동을 결정한다.
    int mctsActionWithTimeThreshold(const State &state, const int64_t time_threshold)
    {
        Node root_node = Node(state);
        root_node.expand();
        auto time_keeper = TimeKeeper(time_threshold);
        for (int cnt = 0;; cnt++)
        {
            if (time_keeper.isTimeOver())
            {
                break;
            }
            root_node.evaluate();
        }
        auto legal_actions = state.legalActions();

        int best_action_searched_number = -1;
        int best_action_index = -1;
        assert(legal_actions.size() == root_node.child_nodes_.size());
        for (int i = 0; i < legal_actions.size(); i++)
        {
            int n = root_node.child_nodes_[i].n_;
            if (n > best_action_searched_number)
            {
                best_action_index = i;
                best_action_searched_number = n;
            }
        }
        return legal_actions[best_action_index];
    }
}

using montecarlo::mctsActionWithTimeThreshold;

namespace montecarlo_bit
{
    int randomActionBit(const ConnectFourStateByBitSet &state)
    {
        auto legal_actions = state.legalActions();
        return legal_actions[mt_for_action() % (legal_actions.size())];
    }
    // 무작위로 플레이아웃해서 승패 점수를 계산한다.
    double playout(ConnectFourStateByBitSet *state)
    { // const&를 사용하면 재귀중에 깊은 복사가 필요하므로, 속도를 위해 포인터를 사용한다.(const가 아닌 참조도 가능)
        switch (state->getWinningStatus())
        {
        case (WinningStatus::WIN):
            return 1.;
        case (WinningStatus::LOSE):
            return 0.;
        case (WinningStatus::DRAW):
            return 0.5;
        default:
            state->advance(randomActionBit(*state));
            return 1. - playout(state);
        }
    }

    constexpr const double C = 1.;             // UCB1 계산에 사용하는 상수
    constexpr const int EXPAND_THRESHOLD = 10; // 노드를 확장하는 임계치

    // MCTS 계산에 사용하는 노드
    class Node
    {
    private:
        ConnectFourStateByBitSet state_;
        double w_;

    public:
        std::vector<Node> child_nodes_;
        double n_;

        Node(const ConnectFourStateByBitSet &state) : state_(state), w_(0), n_(0) {}

        // 노드를 평가한다.
        double evaluate()
        {
            if (this->state_.isDone())
            {
                double value = 0.5;
                switch (this->state_.getWinningStatus())
                {
                case (WinningStatus::WIN):
                    value = 1.;
                    break;
                case (WinningStatus::LOSE):
                    value = 0.;
                    break;
                default:
                    break;
                }
                this->w_ += value;
                ++this->n_;
                return value;
            }
            if (this->child_nodes_.empty())
            {
                ConnectFourStateByBitSet state_copy = this->state_;
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
            this->child_nodes_.clear();
            for (const auto action : legal_actions)
            {
                this->child_nodes_.emplace_back(this->state_);
                this->child_nodes_.back().state_.advance(action);
            }
        }

        // 어떤 노드를 평가할지 선택한다.
        Node &nextChildNode()
        {
            for (auto &child_node : this->child_nodes_)
            {
                if (child_node.n_ == 0)
                    return child_node;
            }
            double t = 0;
            for (const auto &child_node : this->child_nodes_)
            {
                t += child_node.n_;
            }
            double best_value = -INF;
            int best_action_index = -1;
            for (int i = 0; i < this->child_nodes_.size(); i++)
            {
                const auto &child_node = this->child_nodes_[i];
                double ucb1_value = 1. - child_node.w_ / child_node.n_ + (double)C * std::sqrt(2. * std::log(t) / child_node.n_);
                if (ucb1_value > best_value)
                {
                    best_action_index = i;
                    best_value = ucb1_value;
                }
            }
            return this->child_nodes_[best_action_index];
        }
    };

    // 제한 시간(밀리초)을 지정해서 MCTS로 행동을 결정한다.
    int mctsActionBitWithTimeThreshold(const State &state, const int64_t time_threshold)
    {
        Node root_node = Node(ConnectFourStateByBitSet(state));
        root_node.expand();
        auto time_keeper = TimeKeeper(time_threshold);
        for (int cnt = 0;; cnt++)
        {
            if (time_keeper.isTimeOver())
            {
                break;
            }
            root_node.evaluate();
        }
        auto legal_actions = state.legalActions();

        int best_action_searched_number = -1;
        int best_action_index = -1;
        assert(legal_actions.size() == root_node.child_nodes_.size());
        for (int i = 0; i < legal_actions.size(); i++)
        {
            int n = root_node.child_nodes_[i].n_;
            if (n > best_action_searched_number)
            {
                best_action_index = i;
                best_action_searched_number = n;
            }
        }
        return legal_actions[best_action_index];
    }
}
using montecarlo_bit::mctsActionBitWithTimeThreshold;

// 게임을 game_number×2(선공과 후공을 교대)횟수만큼 플레이해서 ais의 0번째에 있는 AI 승률을 표시한다.
void testFirstPlayerWinRate(const std::array<StringAIPair, 2> &ais, const int game_number)
{
    using std::cout;
    using std::endl;

    double first_player_win_rate = 0;
    for (int i = 0; i < game_number; i++)
    {
        auto base_state = State();
        for (int j = 0; j < 2; j++)
        { // 공평하게 선공과 후공을 교대함
            auto state = base_state;
            auto &first_ai = ais[j];
            auto &second_ai = ais[(j + 1) % 2];
            for (int k = 0;; k++)
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

int main()
{
    using std::cout;
    using std::endl;
    auto ais = std::array<StringAIPair, 2>{
        StringAIPair("mctsActionBitWithTimeThreshold 1ms", [](const State &state)
                     { return mctsActionBitWithTimeThreshold(state, 1); }),
        StringAIPair("mctsActionWithTimeThreshold 1ms", [](const State &state)
                     { return mctsActionWithTimeThreshold(state, 1); }),
    };
    testFirstPlayerWinRate(ais, 100);

    return 0;
}
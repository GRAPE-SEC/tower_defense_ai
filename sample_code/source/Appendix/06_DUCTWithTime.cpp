// Copyright [2022] <Copyright Eita Aoki (Thunder) >
#include <string>
#include <vector>
#include <array>
#include <sstream>
#include <utility>
#include <random>
#include <math.h>
#include <chrono>
#include <queue>
#include <algorithm>

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

using ScoreType = int64_t;
constexpr const ScoreType INF = 1000000000LL;

enum WinningStatus
{
    FIRST,  // 플레이어 0이 승리
    SECOND, // 플레이어 1이 승리
    DRAW,
    NONE,
};

class State
{
private:
public:
    // 승패 정보를 획득한다.
    WinningStatus getWinningStatus() const
    {
        return /*(WinningStatus)승패*/;
    }

    // 게임 종료 판정
    bool isDone() const
    {
        /*처리*/
        return /*(bool)종료 판정*/;
    }

    // 지정한 action으로 게임을 1턴 진행한다.
    void advance(const int action0, const int action1)
    {
        /*처리*/
    }

    // 지정한 플레이어가 가능한 행동을 모두 획득한다.
    std::vector<int> legalActions(const int player_id) const
    {
        std::vector<int> actions;
        /*처리*/
        return actions;
    }

    // // 필요에 따라 주석 해제
    // // 현재 게임 상황을 문자열로 만든다.
    // std::string toString() const
    // {
    //     return /*(std::string)문자열로 만든 게임 상황*/;
    // }
};

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

    constexpr const double C = 1.;            // UCB1 계산에 사용하는 상수
    constexpr const int EXPAND_THRESHOLD = 5; // 노드를 확장하는 임계치

    // DUCT 계산에 사용하는 노드
    class Node
    {
    private:
        State state_;
        double w_;

    public:
        std::vector<std::vector<Node>> child_nodeses_;
        double n_;

        // 플레이어 0 시점으로 노드를 평가한다.
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
            if (this->child_nodeses_.empty())
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
                double value = this->nextChildNode().evaluate();
                this->w_ += value;
                ++this->n_;
                return value;
            }
        }
        // 노드를 확장한다.
        void expand()
        {
            auto legal_actions0 = this->state_.legalActions(0);
            auto legal_actions1 = this->state_.legalActions(1);
            this->child_nodeses_.clear();
            for (const auto &action0 : legal_actions0)
            {
                this->child_nodeses_.emplace_back();
                auto &target_nodes = this->child_nodeses_.back();
                for (const auto &action1 : legal_actions1)
                {
                    target_nodes.emplace_back(this->state_);
                    auto &target_node = target_nodes.back();
                    target_node.state_.advance(action0, action1);
                }
            }
        }
        // 어떤 노드를 평가할지 선택한다.
        Node &nextChildNode()
        {
            for (auto &child_nodes : this->child_nodeses_)
            {
                for (auto &child_node : child_nodes)
                {
                    if (child_node.n_ == 0)
                        return child_node;
                }
            }
            double t = 0;
            for (auto &child_nodes : this->child_nodeses_)
            {
                for (auto &child_node : child_nodes)
                {
                    t += child_node.n_;
                }
            }
            int best_is[] = {-1, -1};

            // 플레이어 0의 행동 선택(어느 쪽 플레이어인지 관계없이 여기서는 플레이어 0의 행동)
            double best_value = -INF;
            for (int i = 0; i < this->child_nodeses_.size(); i++)
            {
                const auto &childe_nodes = this->child_nodeses_[i];
                double w = 0;
                double n = 0;
                for (int j = 0; j < childe_nodes.size(); j++)
                {
                    const auto &child_node = childe_nodes[j];
                    w += child_node.w_;
                    n += child_node.n_;
                }

                double ucb1_value = w / n + (double)C * std::sqrt(2. * std::log(t) / n);
                if (ucb1_value > best_value)
                {
                    best_is[0] = i;
                    best_value = ucb1_value;
                }
            }
            // 플레이어 1의 행동 선택(어느 쪽 플레이어인지 관계없이 여기서는 플레이어 1의 행동)
            best_value = -INF;
            for (int j = 0; j < this->child_nodeses_[0].size(); j++)
            {
                double w = 0;
                double n = 0;
                for (int i = 0; i < this->child_nodeses_.size(); i++)
                {
                    const auto &child_node = child_nodeses_[i][j];
                    w += child_node.w_;
                    n += child_node.n_;
                }
                w = 1. - w; // 상대방 쪽의 행동 선택 차례이므로 평가를 반전시켜야 함
                double ucb1_value = w / n + (double)C * std::sqrt(2. * std::log(t) / n);
                if (ucb1_value > best_value)
                {
                    best_is[1] = j;
                    best_value = ucb1_value;
                }
            }

            return this->child_nodeses_[best_is[0]][best_is[1]];
        }

        Node(const State &state) : state_(state), w_(0), n_(0) {}
    };

    // 제한 시간(밀리초)을 지정해서 DUCT로 지정한 플레이어의 행동을 결정한다.
    int ductActionWithTimeThreshold(const State &state, const int player_id, const int64_t time_threshold)
    {
        auto time_keeper = TimeKeeper(time_threshold);
        Node root_node = Node(state);
        root_node.expand();
        for (int i = 0;; i++)
        {
            if (time_keeper.isTimeOver())
            {
                break;
            }
            root_node.evaluate();
        }
        auto legal_actions = state.legalActions(player_id);
        int i_size = root_node.child_nodeses_.size();
        int j_size = root_node.child_nodeses_[0].size();

        if (player_id == 0)
        {
            int best_action_searched_number = -1;
            int best_action_index = -1;
            for (int i = 0; i < i_size; i++)
            {
                int n = 0;
                for (int j = 0; j < j_size; j++)
                {
                    n += root_node.child_nodeses_[i][j].n_;
                }
                if (n > best_action_searched_number)
                {
                    best_action_index = i;
                    best_action_searched_number = n;
                }
            }
            return legal_actions[best_action_index];
        }
        else
        {
            int best_action_searched_number = -1;
            int best_j = -1;
            for (int j = 0; j < j_size; j++)
            {
                int n = 0;
                for (int i = 0; i < i_size; i++)
                {
                    n += root_node.child_nodeses_[i][j].n_;
                }
                if (n > best_action_searched_number)
                {
                    best_j = j;
                    best_action_searched_number = n;
                }
            }
            return legal_actions[best_j];
        }
    }
};
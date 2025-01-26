// Copyright [2022] <Copyright Eita Aoki (Thunder) >
#include <string>
#include <array>
#include <vector>
#include <sstream>
#include <random>
#include <math.h>
#include <chrono>
#include <algorithm>
#include <iostream>
#include <queue>
#include <set>

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
    WIN,
    LOSE,
    DRAW,
    NONE,
};

class State
{
private:
public:
    // 게임 종료 판정
    bool isDone() const
    {
        /*처리*/
        return /*(bool)종료 판정*/;
    }

    // 지정한 action으로 게임을 1턴 진행하고 다음 플레이어 시점의 게임판으로 만든다.
    void advance(const int action)
    {
        /*처리*/
    }

    // 현재 플레이어가 가능한 행동을 모두 획득한다.
    std::vector<int> legalActions() const
    {
        std::vector<int> actions;
        /*처리*/
        return actions;
    }

    // 승패 정보를 획득한다.
    WinningStatus getWinningStatus() const
    {
        return /*(WinningStatus)승패*/;
    }

    // // 필요에 따라 주석 해제
    // // 현재 게임 상황을 문자열로 만든다.
    // std::string toString() const
    // {
    //     return /*(std::string)문자열로 만든 게임 상황*/;
    // }
};

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

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

    // 현재 플레이어 시점에서 게임판을 평가해서 0~1 값을 돌려준다.
    double getScoreRate() const
    {
        return /*(WinningStatus)승률*/;
    }

    // // 필요에 따라 주석 해제
    // // 현재 게임 상황을 문자열로 만든다.
    // std::string toString() const
    // {
    //     return /*(std::string)문자열로 만든 게임 상황*/;
    // }
};

namespace thunder
{

    // Thunder 탐색 계산에서 사용하는 노드
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
                double value = this->state_.getScoreRate();
                this->w_ += value;
                ++this->n_;

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

                double thunder_value = 1. - child_node.w_ / child_node.n_;
                if (thunder_value > best_value)
                {
                    best_action_index = i;
                    best_value = thunder_value;
                }
            }
            return this->child_nodes_[best_action_index];
        }
    };

    // 탐색 횟수를 지정해서 Thunder 탐색으로 행동을 결정한다.
    int thunderSearchAction(const State &state, const int playout_number)
    {
        Node root_node = Node(state);
        root_node.expand();
        for (int i = 0; i < playout_number; i++)
        {
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

    // 제한 시간(밀리초)을 지정해서 MCTS로 행동을 결정한다.
    int thunderSearchActionWithTimeThreshold(const State &state, const int64_t time_threshold)
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
